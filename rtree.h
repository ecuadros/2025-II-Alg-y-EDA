#ifndef RTREE_H
#define RTREE_H

#include <array>
#include <vector>
#include <memory>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>
#include <string>
#include <stdexcept>
#include <functional>

/**
 * @brief Caja delimitadora N-dimensional (AABB - Axis-Aligned Bounding Box)
 * @tparam ScalarT Tipo escalar para las coordenadas (float, double, etc.)
 * @tparam DimV Número de dimensiones
 */
template<class ScalarT, size_t DimV>
struct BoxND {
  using Scalar = ScalarT;
  static constexpr size_t Dim = DimV;

  std::array<Scalar, Dim> m_min{}; /
  std::array<Scalar, Dim> m_max{}; 

  /**
   * @brief Crea una caja vacía (volumen cero con límites infinitos invertidos)
   * @return Caja vacía
   */
  static BoxND Empty() {
    BoxND b{};
    for (size_t d = 0; d < Dim; ++d) {
      b.m_min[d] = std::numeric_limits<Scalar>::infinity();
      b.m_max[d] = -std::numeric_limits<Scalar>::infinity();
    }
    return b;
  }

  /**
   * @brief Verifica si la caja está vacía
   * @return true si min > max en alguna dimensión
   */
  bool IsEmpty() const {
    for (size_t d = 0; d < Dim; ++d) 
      if (m_min[d] > m_max[d]) return true;
    return false;
  }

  /**
   * @brief Verifica si la caja es válida (min <= max en todas las dimensiones)
   * @return true si la caja es válida
   */
  bool IsValid() const {
    for (size_t d = 0; d < Dim; ++d) {
      if (m_min[d] > m_max[d]) return false;
    }
    return true;
  }

  /**
   * @brief Calcula el área/volumen/hipervolumen de la caja
   * @return Producto de las longitudes en cada dimensión
   */
  Scalar Area() const {
    if (IsEmpty()) return Scalar{0};
    Scalar v = Scalar{1};
    for (size_t d = 0; d < Dim; ++d) {
      Scalar len = m_max[d] - m_min[d];
      if (len < Scalar{0}) len = Scalar{0};
      v *= len;
    }
    return v;
  }

  /**
   * @brief Verifica si esta caja intersecta con otra
   * @param other Otra caja a verificar
   * @return true si las cajas se superponen
   */
  bool Intersects(const BoxND& other) const {
    for (size_t d = 0; d < Dim; ++d) {
      if (m_max[d] < other.m_min[d] || other.m_max[d] < m_min[d]) 
        return false;
    }
    return true;
  }

  /**
   * @brief Combina dos cajas en su envolvente mínima (MBR)
   * @param a Primera caja
   * @param b Segunda caja
   * @return Caja que contiene a ambas
   */
  static BoxND Combine(const BoxND& a, const BoxND& b) {
    if (a.IsEmpty()) return b;
    if (b.IsEmpty()) return a;
    BoxND out{};
    for (size_t d = 0; d < Dim; ++d) {
      out.m_min[d] = std::min(a.m_min[d], b.m_min[d]);
      out.m_max[d] = std::max(a.m_max[d], b.m_max[d]);
    }
    return out;
  }

  /**
   * @brief Calcula el incremento de área al combinar dos cajas
   * @param cur Caja actual
   * @param add Caja a añadir
   * @return Diferencia de área (nueva - actual)
   */
  static Scalar Enlargement(const BoxND& cur, const BoxND& add) {
    return Combine(cur, add).Area() - cur.Area();
  }

  /**
   * @brief Expande esta caja para incluir otra (modifica in-place)
   * @param add Caja a incluir
   */
  void ExpandInPlace(const BoxND& add) {
    *this = Combine(*this, add);
  }
};

/**
 * @brief Traits para configurar el comportamiento del R-Tree
 * @tparam ScalarT Tipo escalar para coordenadas
 * @tparam DimV Número de dimensiones
 * @tparam Mv Número máximo de entradas por nodo (M)
 */
template<class ScalarT, size_t DimV, size_t Mv = 16>
struct RTreeTraits {
  using Scalar = ScalarT;
  static constexpr size_t Dim = DimV;
  using Box = BoxND<Scalar, Dim>;
  using Value = std::uint64_t; 

  static constexpr size_t M = Mv;           ///< Máximo de entradas por nodo
  static constexpr size_t m = (Mv + 1) / 2; ///< Mínimo de entradas por nodo

  /**
   * @brief Serializa un valor al stream
   * @param os Stream de salida
   * @param v Valor a serializar
   */
  static void SerializeValue(std::ostream& os, Value v) {
    os.write(reinterpret_cast<const char*>(&v), sizeof(Value));
    if (!os) throw std::runtime_error("serialize_value failed");
  }
  
  /**
   * @brief Deserializa un valor desde el stream
   * @param is Stream de entrada
   * @return Valor deserializado
   */
  static Value DeserializeValue(std::istream& is) {
    Value v{};
    is.read(reinterpret_cast<char*>(&v), sizeof(Value));
    if (!is) throw std::runtime_error("deserialize_value failed");
    return v;
  }
};

/**
 * @class CRTree
 * @brief R-Tree N-dimensional con split cuadrático
 * @tparam Traits Configuración del árbol (ver RTreeTraits)
 */
template<class Traits>
class CRTree {
public:
  using Scalar = typename Traits::Scalar;
  static constexpr size_t Dim = Traits::Dim;
  using Box   = typename Traits::Box;
  using Value = typename Traits::Value;

  static constexpr size_t M = Traits::M;
  static constexpr size_t m = Traits::m;

  static_assert(M >= 4, "Traits::M must be >= 4");
  static_assert(m >= 2, "Traits::m must be >= 2");
  static_assert(m <= M, "Traits::m must be <= Traits::M");

private:
  struct Node;

  struct Entry {
    Box m_box{};
    bool m_isLeafEntry = false;
    Value m_value{};
    std::unique_ptr<Node> m_child{};

    static Entry CreateLeaf(const Box& b, Value v) {
      Entry e;
      e.m_box = b;
      e.m_isLeafEntry = true;
      e.m_value = v;
      return e;
    }

    static Entry CreateInternal(const Box& b, std::unique_ptr<Node> c) {
      Entry e;
      e.m_box = b;
      e.m_isLeafEntry = false;
      e.m_child = std::move(c);
      return e;
    }
  };

  struct Node {
    bool m_isLeaf = true;
    std::vector<Entry> m_entries{};
    Box m_mbr = Box::Empty();

    explicit Node(bool leaf) : m_isLeaf(leaf) {}

    size_t Size() const noexcept { return m_entries.size(); }
    
    void RecalcMBR() {
      Box b = Box::Empty();
      for (const auto& e : m_entries) b.ExpandInPlace(e.m_box);
      m_mbr = b;
    }

    void RefreshInternalBoxesFromChildren() {
      if (m_isLeaf) return;
      for (auto& e : m_entries) 
        e.m_box = e.m_child ? e.m_child->m_mbr : Box::Empty();
      RecalcMBR();
    }
  };

public:
  /**
   * @brief Constructor por defecto
   */
  CRTree() : m_pRoot(std::make_unique<Node>(true)) {
    if (M < 4) throw std::invalid_argument("M must be >= 4");
    if (m < 2) throw std::invalid_argument("m must be >= 2");
    if (m > M) throw std::invalid_argument("m must be <= M");
  }

  /**
   * @brief Elimina todos los elementos del árbol
   */
  void Clear() {
    m_pRoot = std::make_unique<Node>(true);
    m_size = 0;
  }

  /**
   * @brief Retorna el número de elementos en el árbol
   */
  size_t Size() const noexcept { return m_size; }
  
  /**
   * @brief Verifica si el árbol está vacío
   * @return true si no hay elementos
   */
  bool Empty() const noexcept { return m_size == 0; }

  /**
   * @brief Inserta un elemento en el árbol
   * @param id Identificador único
   * @param box Caja delimitadora
   */
  void Insert(Value id, const Box& box) {
    if (!box.IsValid()) {
      throw std::invalid_argument("Invalid box: min must be <= max in all dimensions");
    }

    auto [leaf, path] = ChooseLeaf(m_pRoot.get(), box);
    leaf->m_entries.push_back(Entry::CreateLeaf(box, id));
    leaf->RecalcMBR();

    std::unique_ptr<Node> sibling = nullptr;
    if (leaf->m_entries.size() > M) {
      sibling = SplitQuadratic(leaf);
    }

    AdjustAfterInsert(path, leaf, std::move(sibling));
    ++m_size;
  }

  /**
   * @brief Elimina un elemento del árbol
   * @param id Identificador a eliminar
   * @return true si se eliminó, false si no existía
   */
  bool Delete(Value id) {
    std::vector<Node*> path;
    Node* leaf = FindLeaf(m_pRoot.get(), id, path);
    if (!leaf) return false;

    auto& ents = leaf->m_entries;
    auto it = std::find_if(ents.begin(), ents.end(),
      [&](const Entry& e){ return e.m_isLeafEntry && e.m_value == id; });

    if (it == ents.end()) return false;

    ents.erase(it);
    leaf->RecalcMBR();

    std::vector<Entry> orphan_leaf_entries;
    CondenseTree(path, orphan_leaf_entries);
    ShrinkRoot();

    for (const auto& e : orphan_leaf_entries) {
      Insert(e.m_value, e.m_box);
      --m_size;
    }

    --m_size;
    return true;
  }

  /**
   * @brief Busca elementos que intersectan con la región
   * @param query Región de búsqueda
   * @return IDs de elementos encontrados
   */
  std::vector<Value> RangeQuery(const Box& query) const {
    std::vector<Value> out;
    RangeQueryRecursive(m_pRoot.get(), query, out);
    return out;
  }

  /**
   * @brief Guarda el árbol en disco
   * @param path Ruta del archivo
   */
  void WriteToFile(const std::string& path) const {
    std::ofstream os(path, std::ios::binary);
    if (!os) throw std::runtime_error("Cannot open for writing: " + path);

    const std::uint32_t magic = 0x52544E44;
    const std::uint32_t version = 1;
    os.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    os.write(reinterpret_cast<const char*>(&version), sizeof(version));

    const std::uint64_t dim = static_cast<std::uint64_t>(Dim);
    const std::uint64_t Mfile = static_cast<std::uint64_t>(M);
    const std::uint64_t mfile = static_cast<std::uint64_t>(m);
    const std::uint64_t sz = static_cast<std::uint64_t>(m_size);
    os.write(reinterpret_cast<const char*>(&dim), sizeof(dim));
    os.write(reinterpret_cast<const char*>(&Mfile), sizeof(Mfile));
    os.write(reinterpret_cast<const char*>(&mfile), sizeof(mfile));
    os.write(reinterpret_cast<const char*>(&sz), sizeof(sz));

    WriteNode(os, m_pRoot.get());
    if (!os) throw std::runtime_error("Failed while writing tree.");
  }

  /**
   * @brief Carga el árbol desde disco
   * @param path Ruta del archivo
   */
  void ReadFromFile(const std::string& path) {
    std::ifstream is(path, std::ios::binary);
    if (!is) throw std::runtime_error("Cannot open for reading: " + path);

    std::uint32_t magic = 0, version = 0;
    is.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    is.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (!is || magic != 0x52544E44) throw std::runtime_error("Bad file magic.");
    if (version != 1) throw std::runtime_error("Unsupported version.");

    std::uint64_t dim = 0, Mfile = 0, mfile = 0, sz = 0;
    is.read(reinterpret_cast<char*>(&dim), sizeof(dim));
    is.read(reinterpret_cast<char*>(&Mfile), sizeof(Mfile));
    is.read(reinterpret_cast<char*>(&mfile), sizeof(mfile));
    is.read(reinterpret_cast<char*>(&sz), sizeof(sz));
    if (!is) throw std::runtime_error("Corrupt header.");
    if (dim != Dim) throw std::runtime_error("Dim mismatch.");
    if (Mfile != M || mfile != m) throw std::runtime_error("M/m mismatch.");

    m_pRoot = ReadNode(is);
    m_size = static_cast<size_t>(sz);
    m_pRoot->RecalcMBR();
  }

private:
  std::unique_ptr<Node> m_pRoot; 
  size_t m_size = 0;              

  /**
   * @brief Selecciona el subárbol óptimo para inserción
   * @param n Nodo actual (interno)
   * @param box Caja a insertar
   * @return Índice de la mejor entrada hijo
   */
  static size_t ChooseSubtree(const Node* n, const Box& box) {
    size_t best = 0;
    Scalar best_enl = std::numeric_limits<Scalar>::infinity();
    Scalar best_vol = std::numeric_limits<Scalar>::infinity();

    for (size_t i = 0; i < n->m_entries.size(); ++i) {
      const auto& e = n->m_entries[i];
      const Scalar enl = Box::Enlargement(e.m_box, box);
      const Scalar vol = e.m_box.Area();
      if (enl < best_enl || (enl == best_enl && vol < best_vol)) {
        best = i;
        best_enl = enl;
        best_vol = vol;
      }
    }
    return best;
  }

  /**
   * @brief Encuentra la hoja apropiada para insertar una caja
   * @param root Raíz del subárbol
   * @param box Caja a insertar
   * @return Par (nodo hoja, camino desde raíz)
   */
  static std::pair<Node*, std::vector<Node*>> ChooseLeaf(Node* root, const Box& box) {
    std::vector<Node*> path;
    Node* cur = root;
    while (!cur->m_isLeaf) {
      path.push_back(cur);
      const size_t idx = ChooseSubtree(cur, box);
      cur = cur->m_entries[idx].m_child.get();
    }
    return {cur, path};
  }

  /**
   * @brief Busca recursivamente la hoja que contiene un valor
   * @param cur Nodo actual
   * @param id Identificador a buscar
   * @param path Camino desde la raíz (out parameter)
   * @return Puntero a la hoja si se encuentra, nullptr si no
   */
  static Node* FindLeaf(Node* cur, Value id, std::vector<Node*>& path) {
    path.push_back(cur);

    if (cur->m_isLeaf) {
      for (const auto& e : cur->m_entries) {
        if (e.m_isLeafEntry && e.m_value == id) return cur;
      }
      path.pop_back();
      return nullptr;
    }

    for (auto& e : cur->m_entries) {
      if (Node* found = FindLeaf(e.m_child.get(), id, path)) {
        return found;
      }
    }

    path.pop_back();
    return nullptr;
  }

  /**
   * @brief Divide un nodo usando el algoritmo cuadrático de Guttman
   * @param n Nodo a dividir (con M+1 entradas)
   * @return Nodo hermano creado
   */
  std::unique_ptr<Node> SplitQuadratic(Node* n) {
    auto sibling = std::make_unique<Node>(n->m_isLeaf);
    const size_t total = n->m_entries.size();
    
    // Validación: debe haber al menos M+1 elementos para split
    if (total <= M) {
      throw std::runtime_error("SplitQuadratic called with insufficient entries");
    }
    
    // PickSeeds: encontrar las dos entradas más separadas
    size_t seed1 = 0, seed2 = 1;
    Scalar worst_waste = Scalar{-1};

    // OPTIMIZACIÓN: Para total pequeño (M=8), asegurar buena separación inicial
    bool found_seeds = false;
    for (size_t i = 0; i < total && i < total - 1; ++i) {
      for (size_t j = i + 1; j < total; ++j) {
        const Box u = Box::Combine(n->m_entries[i].m_box, n->m_entries[j].m_box);
        const Scalar waste = u.Area() - n->m_entries[i].m_box.Area() - n->m_entries[j].m_box.Area();
        if (waste > worst_waste) {
          worst_waste = waste;
          seed1 = i; 
          seed2 = j;
          found_seeds = true;
        }
      }
    }

    // Si no se encontraron seeds válidas (ej: todos los puntos coinciden), usar primeros dos
    if (!found_seeds || seed1 == seed2) {
      seed1 = 0;
      seed2 = (total > 1) ? 1 : 0;
    }

    // Mover entradas a buffer temporal
    std::vector<Entry> items;
    items.reserve(total);
    for (auto& e : n->m_entries) items.push_back(std::move(e));
    n->m_entries.clear();

    // Asignar semillas
    std::vector<bool> used(total, false);
    
    n->m_entries.push_back(std::move(items[seed1]));
    sibling->m_entries.push_back(std::move(items[seed2]));
    used[seed1] = true;
    used[seed2] = true;

    n->RecalcMBR();
    sibling->RecalcMBR();

    // Distribuir entradas restantes
    auto remaining_count = [&]() -> size_t {
      size_t count = 0;
      for (bool u : used) if (!u) ++count;
      return count;
    };

    while (remaining_count() > 0) {
      const size_t rem = remaining_count();

      // CRITICAL: Force-fill para garantizar mínimo m en cada nodo
      // Esto previene el bug con M=8
      if (n->m_entries.size() + rem == m) {
        for (size_t i = 0; i < total; ++i) {
          if (!used[i]) {
            n->m_entries.push_back(std::move(items[i]));
            used[i] = true;
          }
        }
        break;
      }
      if (sibling->m_entries.size() + rem == m) {
        for (size_t i = 0; i < total; ++i) {
          if (!used[i]) {
            sibling->m_entries.push_back(std::move(items[i]));
            used[i] = true;
          }
        }
        break;
      }

      // PickNext: encontrar entrada con mayor diferencia de enlargement
      size_t best_idx = 0;
      Scalar best_diff = Scalar{-1};
      bool found = false;

      for (size_t i = 0; i < total; ++i) {
        if (used[i]) continue;

        const Scalar enlA = Box::Enlargement(n->m_mbr, items[i].m_box);
        const Scalar enlB = Box::Enlargement(sibling->m_mbr, items[i].m_box);
        const Scalar diff = (enlA > enlB) ? (enlA - enlB) : (enlB - enlA);

        if (!found || diff > best_diff) {
          found = true;
          best_diff = diff;
          best_idx = i;
        }
      }

      if (!found) {
        // Fallback: tomar el primer no usado
        for (size_t i = 0; i < total; ++i) {
          if (!used[i]) {
            best_idx = i;
            break;
          }
        }
      }

      // Asignar al grupo con menor enlargement
      Entry next = std::move(items[best_idx]);
      used[best_idx] = true;

      const Scalar enlA = Box::Enlargement(n->m_mbr, next.m_box);
      const Scalar enlB = Box::Enlargement(sibling->m_mbr, next.m_box);

      if (enlA < enlB) {
        n->m_entries.push_back(std::move(next));
      } else if (enlB < enlA) {
        sibling->m_entries.push_back(std::move(next));
      } else {
        // Empate: criterios de desempate
        const Scalar volA = n->m_mbr.Area();
        const Scalar volB = sibling->m_mbr.Area();
        if (volA < volB) {
          n->m_entries.push_back(std::move(next));
        } else if (volB < volA) {
          sibling->m_entries.push_back(std::move(next));
        } else {
          // Último criterio: menor cantidad de entradas
          if (n->m_entries.size() <= sibling->m_entries.size()) {
            n->m_entries.push_back(std::move(next));
          } else {
            sibling->m_entries.push_back(std::move(next));
          }
        }
      }

      n->RecalcMBR();
      sibling->RecalcMBR();
    }

    // Actualizar boxes de nodos internos
    n->RefreshInternalBoxesFromChildren();
    sibling->RefreshInternalBoxesFromChildren();

    // VALIDACIÓN POST-SPLIT: Verificar invariantes
    if (n->m_entries.size() < m || sibling->m_entries.size() < m) {
      throw std::runtime_error("Split violated minimum occupancy constraint");
    }
    if (n->m_entries.size() > M || sibling->m_entries.size() > M) {
      throw std::runtime_error("Split violated maximum occupancy constraint");
    }

    return sibling;
  }

  /**
   * @brief Ajusta el árbol después de una inserción con posible split
   * @param path Camino desde la raíz hasta el nodo modificado
   * @param n Nodo modificado
   * @param nn Hermano creado por split (nullptr si no hubo split)
   */
  void AdjustAfterInsert(std::vector<Node*>& path, Node* n, std::unique_ptr<Node> nn) {
    while (!path.empty()) {
      Node* parent = path.back();
      path.pop_back();

      for (auto& e : parent->m_entries) {
        if (!e.m_isLeafEntry && e.m_child.get() == n) {
          e.m_box = n->m_mbr;
          break;
        }
      }

      if (nn) {
        parent->m_entries.push_back(Entry::CreateInternal(nn->m_mbr, std::move(nn)));
      }

      parent->RecalcMBR();

      if (parent->m_entries.size() > M) {
        nn = SplitQuadratic(parent);
        n = parent;
      } else {
        nn = nullptr;
        n = parent;
      }
    }

    if (nn) {
      auto new_root = std::make_unique<Node>(false);
      auto old_root = std::move(m_pRoot);

      new_root->m_entries.push_back(Entry::CreateInternal(old_root->m_mbr, std::move(old_root)));
      new_root->m_entries.push_back(Entry::CreateInternal(nn->m_mbr, std::move(nn)));
      new_root->RecalcMBR();
      m_pRoot = std::move(new_root);
    } else {
      m_pRoot->RefreshInternalBoxesFromChildren();
    }
  }

  /**
   * @brief Recolecta recursivamente todas las entradas hoja de un subárbol
   * @param sub Subárbol a recorrer
   * @param out Vector donde se almacenan las entradas (out parameter)
   */
  void CollectLeafEntries(Node& sub, std::vector<Entry>& out) {
    if (sub.m_isLeaf) {
      for (auto& e : sub.m_entries) {
        if (e.m_isLeafEntry) {
          out.push_back(Entry::CreateLeaf(e.m_box, e.m_value));
        }
      }
      return;
    }
    for (auto& e : sub.m_entries) {
      if (e.m_child) CollectLeafEntries(*e.m_child, out);
    }
  }

  /**
   * @brief Condensa el árbol eliminando nodos con underflow
   * @param path Camino desde la raíz hasta el nodo modificado
   * @param orphan_leaf_entries Entradas huérfanas a reinsertar (out parameter)
   */
  void CondenseTree(std::vector<Node*>& path, std::vector<Entry>& orphan_leaf_entries) {
    if (path.size() <= 1) {
      if (!path.empty()) path[0]->RecalcMBR();
      return;
    }
    
    for (size_t i = path.size() - 1; i > 0; --i) {
      Node* node = path[i];
      Node* parent = path[i - 1];

      if (node->m_entries.size() >= m) {
        node->RecalcMBR();
        for (auto& e : parent->m_entries) {
          if (!e.m_isLeafEntry && e.m_child.get() == node) {
            e.m_box = node->m_mbr;
            break;
          }
        }
        parent->RecalcMBR();
        continue;
      }

      auto it = std::find_if(parent->m_entries.begin(), parent->m_entries.end(),
        [&](const Entry& e){ return (!e.m_isLeafEntry && e.m_child.get() == node); });

      if (it != parent->m_entries.end()) {
        std::unique_ptr<Node> doomed = std::move(it->m_child);
        parent->m_entries.erase(it);
        CollectLeafEntries(*doomed, orphan_leaf_entries);
      }
      parent->RecalcMBR();
    }

    m_pRoot->RecalcMBR();
  }

  /**
   * @brief Reduce la altura del árbol si la raíz tiene un solo hijo
   */
  void ShrinkRoot() {
    if (!m_pRoot->m_isLeaf && m_pRoot->m_entries.size() == 1) {
      m_pRoot = std::move(m_pRoot->m_entries[0].m_child);
    }
    if (m_pRoot->m_entries.empty()) {
      m_pRoot = std::make_unique<Node>(true);
    }
    m_pRoot->RecalcMBR();
  }

  /**
   * @brief Búsqueda recursiva por rango
   * @param n Nodo actual
   * @param q Caja de consulta
   * @param out Vector de resultados (out parameter)
   * 
   * Poda ramas que no intersectan con la consulta
   */
  static void RangeQueryRecursive(const Node* n, const Box& q, std::vector<Value>& out) {
    if (!n->m_mbr.Intersects(q)) return;

    if (n->m_isLeaf) {
      for (const auto& e : n->m_entries) {
        if (e.m_box.Intersects(q)) out.push_back(e.m_value);
      }
      return;
    }

    for (const auto& e : n->m_entries) {
      if (!e.m_box.Intersects(q)) continue;
      RangeQueryRecursive(e.m_child.get(), q, out);
    }
  }

  static void WriteScalar(std::ostream& os, const Scalar& s) {
    os.write(reinterpret_cast<const char*>(&s), sizeof(Scalar));
    if (!os) throw std::runtime_error("WriteScalar failed");
  }
  
  static Scalar ReadScalar(std::istream& is) {
    Scalar s{};
    is.read(reinterpret_cast<char*>(&s), sizeof(Scalar));
    if (!is) throw std::runtime_error("ReadScalar failed");
    return s;
  }

  static void WriteBox(std::ostream& os, const Box& b) {
    for (size_t d = 0; d < Dim; ++d) WriteScalar(os, b.m_min[d]);
    for (size_t d = 0; d < Dim; ++d) WriteScalar(os, b.m_max[d]);
  }
  
  static Box ReadBox(std::istream& is) {
    Box b{};
    for (size_t d = 0; d < Dim; ++d) b.m_min[d] = ReadScalar(is);
    for (size_t d = 0; d < Dim; ++d) b.m_max[d] = ReadScalar(is);
    return b;
  }

  static void WriteNode(std::ostream& os, const Node* n) {
    if (!n) throw std::runtime_error("Attempt to write null node");
    
    const std::uint8_t leaf = n->m_isLeaf ? 1 : 0;
    os.write(reinterpret_cast<const char*>(&leaf), sizeof(leaf));
    if (!os) throw std::runtime_error("Failed to write leaf flag");

    const std::uint64_t count = static_cast<std::uint64_t>(n->m_entries.size());
    os.write(reinterpret_cast<const char*>(&count), sizeof(count));
    if (!os) throw std::runtime_error("Failed to write entry count");

    WriteBox(os, n->m_mbr);

    for (const auto& e : n->m_entries) {
      WriteBox(os, e.m_box);
      if (n->m_isLeaf) {
        Traits::SerializeValue(os, e.m_value);
      } else {
        if (!e.m_child) throw std::runtime_error("Null child pointer in internal node");
        WriteNode(os, e.m_child.get());
      }
    }
  }

  static std::unique_ptr<Node> ReadNode(std::istream& is) {
    std::uint8_t leaf = 0;
    is.read(reinterpret_cast<char*>(&leaf), sizeof(leaf));
    if (!is) throw std::runtime_error("Corrupt node (leaf flag).");

    std::uint64_t count = 0;
    is.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!is) throw std::runtime_error("Corrupt node (count).");

    auto n = std::make_unique<Node>(leaf != 0);
    n->m_mbr = ReadBox(is);
    n->m_entries.reserve(static_cast<size_t>(count));

    for (std::uint64_t i = 0; i < count; ++i) {
      Box eb = ReadBox(is);
      if (n->m_isLeaf) {
        Value v = Traits::DeserializeValue(is);
        n->m_entries.push_back(Entry::CreateLeaf(eb, v));
      } else {
        auto child = ReadNode(is);
        if (!child) throw std::runtime_error("Failed to read child node");
        n->m_entries.push_back(Entry::CreateInternal(eb, std::move(child)));
      }
    }

    n->RecalcMBR();
    return n;
  }
};

#endif 