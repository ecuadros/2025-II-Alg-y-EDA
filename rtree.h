#ifndef __RTREE_H__
#define __RTREE_H__

#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <cmath>
#include <algorithm>

// Trait para configurar el RTree
template <typename _CoordType, typename _DataType>
struct RTreeTrait
{
       using CoordType = _CoordType;  // Tipo de coordenadas (float, double, int)
       using DataType = _DataType;    // Tipo de datos almacenados
};

// Forward declarations
template <typename Trait, int MaxNodes, int MinNodes> class RTree;
template <typename Trait> class RTreeNode;

// Rectangulo MBR (Minimum Bounding Rectangle)
template <typename Trait>
struct Rect
{
       typedef typename Trait::CoordType CoordType;
       
       CoordType xMin, yMin, xMax, yMax;
       
       // Verifica si dos rectangulos se solapan
       bool Overlaps(const Rect& other) const
       {
              return xMin <= other.xMax && xMax >= other.xMin &&
                     yMin <= other.yMax && yMax >= other.yMin;
       }
       
       // Verifica si este rectangulo contiene completamente a otro
       bool Contains(const Rect& other) const
       {
              return xMin <= other.xMin && xMax >= other.xMax &&
                     yMin <= other.yMin && yMax >= other.yMax;
       }
       
       // Verifica si dos rectangulos son iguales
       bool operator==(const Rect& other) const
       {
              return xMin == other.xMin && yMin == other.yMin &&
                     xMax == other.xMax && yMax == other.yMax;
       }
       
       bool operator!=(const Rect& other) const { return !(*this == other); }
       
       // Calcula el area del rectangulo
       CoordType Area() const
       {
              return (xMax - xMin) * (yMax - yMin);
       }
       
       // Combina dos rectangulos en uno que los contenga a ambos
       Rect Combine(const Rect& other) const
       {
              Rect result;
              result.xMin = std::min(xMin, other.xMin);
              result.yMin = std::min(yMin, other.yMin);
              result.xMax = std::max(xMax, other.xMax);
              result.yMax = std::max(yMax, other.yMax);
              return result;
       }
       
       // Calcula cuanto creceria el area al agregar otro rectangulo
       CoordType Enlargement(const Rect& other) const
       {
              return Combine(other).Area() - Area();
       }
};

// Entrada en un nodo del RTree
template <typename Trait>
struct RTreeEntry
{
       typedef typename Trait::DataType DataType;
       typedef Rect<Trait> RectType;
       typedef RTreeNode<Trait>* NodePtr;
       
       RectType rect;           // MBR de la entrada
       DataType data;           // Dato (solo en hojas)
       NodePtr child = nullptr; // Hijo (solo en nodos internos)
};

// Nodo del RTree
template <typename Trait>
class RTreeNode
{
public:
       typedef typename Trait::CoordType CoordType;
       typedef typename Trait::DataType DataType;
       typedef Rect<Trait> RectType;
       typedef RTreeEntry<Trait> Entry;
       
       int level = 0;                 // Nivel del nodo (0 = hoja)
       std::vector<Entry> entries;   // Entradas del nodo
       
       bool IsLeaf() const { return level == 0; }
       
       // Calcula el MBR que contiene todas las entradas
       RectType ComputeMBR() const
       {
              RectType mbr = entries[0].rect;
              for (size_t i = 1; i < entries.size(); ++i)
                     mbr = mbr.Combine(entries[i].rect);
              return mbr;
       }
       
       // Encuentra el hijo cuyo MBR requiere menor expansion (ChooseSubtree)
       size_t ChooseBestChild(const RectType& rect) const
       {
              size_t best = 0;
              CoordType minEnlargement = entries[0].rect.Enlargement(rect);
              CoordType minArea = entries[0].rect.Area();  // Desempate por area menor
              
              for (size_t i = 1; i < entries.size(); ++i)
              {
                     CoordType enl = entries[i].rect.Enlargement(rect);
                     CoordType area = entries[i].rect.Area();
                     
                     // Preferir menor enlargement, desempatar por menor area
                     if (enl < minEnlargement || (enl == minEnlargement && area < minArea))
                     {
                            minEnlargement = enl;
                            minArea = area;
                            best = i;
                     }
              }
              return best;
       }
};

// Arbol R (R-Tree)
template <typename Trait, int MaxNodes = 4, int MinNodes = 2>
class RTree
{
       // Validacion en tiempo de compilacion
       static_assert(MinNodes >= 1, "MinNodes debe ser mayor o igual que 1");
       static_assert(MaxNodes >= 2 * MinNodes, "MaxNodes debe ser mayor o igual que 2*MinNodes para splits");
       
public:
       // Tipos extraidos del Trait
       typedef typename Trait::CoordType CoordType;
       typedef typename Trait::DataType DataType;
       typedef Rect<Trait> RectType;
       typedef RTreeEntry<Trait> Entry;
       typedef RTreeNode<Trait> Node;
       
       // Tipo para resultados de RangeQuery (MBR + Data)
       typedef std::pair<RectType, DataType> QueryResult;

private:
       Node* m_Root;
       size_t m_Count;

public:
       RTree() : m_Root(new Node()), m_Count(0) {}
       
       ~RTree() { Destroy(m_Root); }
       
       // ==================== INSERCION ====================
       // Inserta un elemento con su MBR y dato asociado
       void Insert(CoordType xMin, CoordType yMin, CoordType xMax, CoordType yMax, const DataType& data)
       {
              Entry entry;
              entry.rect = {xMin, yMin, xMax, yMax};
              entry.data = data;
              entry.child = nullptr;
              
              Node* newNode = nullptr;
              if (InsertRec(entry, m_Root, newNode, 0))
              {
                     // La raiz se dividio, crear nueva raiz
                     Node* newRoot = new Node();
                     newRoot->level = m_Root->level + 1;
                     
                     Entry e1, e2;
                     e1.rect = m_Root->ComputeMBR();
                     e1.child = m_Root;
                     e2.rect = newNode->ComputeMBR();
                     e2.child = newNode;
                     
                     newRoot->entries.push_back(e1);
                     newRoot->entries.push_back(e2);
                     m_Root = newRoot;
              }
              ++m_Count;
       }
       
       // ==================== ELIMINACION ====================
       // Elimina un elemento buscando por MBR Y dato}
       bool Remove(CoordType xMin, CoordType yMin, CoordType xMax, CoordType yMax, const DataType& data)
       {
              RectType targetRect = {xMin, yMin, xMax, yMax};
              std::vector<Node*> reinsertList;
              
              if (RemoveRec(targetRect, data, m_Root, reinsertList))
                     return false;  // No encontrado
              
              // Reinsertar entradas de nodos con underflow (CondenseTree)
              for (Node* node : reinsertList)
              {
                     ReinsertEntries(node);
                     delete node;
              }
              
              // Reducir altura si la raiz quedo con un solo hijo
              while (!m_Root->IsLeaf() && m_Root->entries.size() == 1)
              {
                     Node* oldRoot = m_Root;
                     m_Root = m_Root->entries[0].child;
                     delete oldRoot;
              }
              --m_Count;
              return true;
       }
       
       // Sobrecarga: elimina solo por dato (busca en todo el arbol)
       bool Remove(const DataType& data)
       {
              std::vector<Node*> reinsertList;
              
              if (RemoveByDataRec(data, m_Root, reinsertList))
                     return false;
              
              for (Node* node : reinsertList)
              {
                     ReinsertEntries(node);
                     delete node;
              }
              
              while (!m_Root->IsLeaf() && m_Root->entries.size() == 1)
              {
                     Node* oldRoot = m_Root;
                     m_Root = m_Root->entries[0].child;
                     delete oldRoot;
              }
              --m_Count;
              return true;
       }
       
       // ==================== RANGE QUERY ====================
       // Retorna todos los elementos cuyo MBR intersecta con el rango dado
       // Incluye tanto el MBR como el dato
       std::vector<QueryResult> RangeQuery(CoordType xMin, CoordType yMin, CoordType xMax, CoordType yMax) const
       {
              std::vector<QueryResult> results;
              RectType queryRect = {xMin, yMin, xMax, yMax};
              SearchRec(m_Root, queryRect, results);
              return results;
       }
       
       // ==================== WRITE TO DISK ====================
       bool Write(const std::string& filename) const
       {
              std::ofstream file(filename);
              if (!file) return false;
              
              // Escribir cantidad de elementos
              file << m_Count << "\n";
              WriteNode(file, m_Root);
              return true;
       }
       
       // ==================== READ FROM DISK ====================
       bool Read(const std::string& filename)
       {
              std::ifstream file(filename);
              if (!file) return false;
              
              // Leer cantidad de elementos
              file >> m_Count;
              Destroy(m_Root);
              m_Root = ReadNode(file);
              return true;
       }
       
       // ==================== UTILIDADES ====================
       size_t size() const { return m_Count; }
       bool empty() const { return m_Count == 0; }
       
       void Print() const { Print(std::cout); }
       
       void Print(std::ostream& os) const
       {
              os << "RTree [" << m_Count << " elementos]\n";
              PrintNode(os, m_Root, 0);
       }

private:
       // Destruye un nodo y sus hijos recursivamente
       void Destroy(Node* node)
       {
              if (!node) return;
              for (auto& entry : node->entries)
                     if (entry.child) Destroy(entry.child);
              delete node;
       }
       
       // Verifica si el nodo esta lleno
       bool IsFull(Node* node) const
       {
              return node->entries.size() >= MaxNodes;
       }
       
       // Verifica si el nodo tiene underflow
       bool HasUnderflow(Node* node) const
       {
              return node->entries.size() < MinNodes;
       }
       
       // ==================== INSERCION RECURSIVA ====================
       // Retorna true si hubo split
       bool InsertRec(const Entry& entry, Node* node, Node*& newNode, int targetLevel)
       {
              if (node->level > targetLevel)
              {
                     // Nodo interno: bajar al mejor hijo (ChooseLeaf)
                     size_t best = node->ChooseBestChild(entry.rect);
                     Node* childNew = nullptr;
                     bool split = InsertRec(entry, node->entries[best].child, childNew, targetLevel);
                     
                     // Actualizar MBR del hijo (AdjustTree)
                     node->entries[best].rect = node->entries[best].child->ComputeMBR();
                     
                     if (!split) return false;
                     
                     // El hijo se dividio, agregar el nuevo nodo
                     Entry newEntry;
                     newEntry.rect = childNew->ComputeMBR();
                     newEntry.child = childNew;
                     
                     if (!IsFull(node))
                     {
                            node->entries.push_back(newEntry);
                            return false;
                     }
                     Split(node, newEntry, newNode);
                     return true;
              }
              else
              {
                     // Nodo hoja: insertar aqui
                     if (!IsFull(node))
                     {
                            node->entries.push_back(entry);
                            return false;
                     }
                     Split(node, entry, newNode);
                     return true;
              }
       }
       
       // ==================== SPLIT ====================
       void Split(Node* node, const Entry& entry, Node*& newNode)
       {
              // Juntar todas las entradas + la nueva
              std::vector<Entry> all = node->entries;
              all.push_back(entry);
              
              // PickSeeds: elegir las 2 entradas mas separadas
              size_t seed1 = 0, seed2 = 1;
              CoordType worstWaste = all[0].rect.Combine(all[1].rect).Area()
                                   - all[0].rect.Area() - all[1].rect.Area();
              
              for (size_t i = 0; i < all.size() - 1; ++i)
              {
                     for (size_t j = i + 1; j < all.size(); ++j)
                     {
                            // Espacio desperdiciado si van juntas
                            CoordType waste = all[i].rect.Combine(all[j].rect).Area()
                                            - all[i].rect.Area() - all[j].rect.Area();
                            if (waste > worstWaste)
                            {
                                   worstWaste = waste;
                                   seed1 = i;
                                   seed2 = j;
                            }
                     }
              }
              
              // Crear nuevo nodo y distribuir semillas
              newNode = new Node();
              newNode->level = node->level;
              node->entries.clear();
              
              node->entries.push_back(all[seed1]);
              newNode->entries.push_back(all[seed2]);
              
              // Marcar semillas como usadas
              std::vector<bool> used(all.size(), false);
              used[seed1] = used[seed2] = true;
              
              // PickNext: distribuir el resto de entradas
              for (size_t remaining = all.size() - 2; remaining > 0; --remaining)
              {
                     // Si un grupo necesita todas las restantes para MinNodes
                     if (node->entries.size() + remaining <= MinNodes)
                     {
                            for (size_t i = 0; i < all.size(); ++i)
                                   if (!used[i]) { node->entries.push_back(all[i]); used[i] = true; }
                            break;
                     }
                     if (newNode->entries.size() + remaining <= MinNodes)
                     {
                            for (size_t i = 0; i < all.size(); ++i)
                                   if (!used[i]) { newNode->entries.push_back(all[i]); used[i] = true; }
                            break;
                     }
                     
                     // Elegir entrada que maximice diferencia de enlargement
                     RectType mbr1 = node->ComputeMBR();
                     RectType mbr2 = newNode->ComputeMBR();
                     size_t bestIdx = 0;
                     CoordType bestDiff = -1;
                     int bestGroup = 0;
                     
                     for (size_t i = 0; i < all.size(); ++i)
                     {
                            if (used[i]) continue;
                            CoordType enl1 = mbr1.Enlargement(all[i].rect);
                            CoordType enl2 = mbr2.Enlargement(all[i].rect);
                            CoordType diff = std::abs(enl1 - enl2);
                            
                            if (bestDiff < 0 || diff > bestDiff)
                            {
                                   bestDiff = diff;
                                   bestIdx = i;
                                   bestGroup = (enl1 < enl2) ? 0 : 1;
                            }
                     }
                     used[bestIdx] = true;
                     if (bestGroup == 0)
                            node->entries.push_back(all[bestIdx]);
                     else
                            newNode->entries.push_back(all[bestIdx]);
              }
       }
       
       // ==================== ELIMINACION RECURSIVA (por MBR + dato) ====================
       // Usa el MBR para podar la busqueda
       bool RemoveRec(const RectType& targetRect, const DataType& data, Node* node, std::vector<Node*>& reinsertList)
       {
              if (!node->IsLeaf())
              {
                     // Nodo interno: solo buscar en hijos cuyo MBR contiene al objetivo
                     for (size_t i = 0; i < node->entries.size(); ++i)
                     {
                            // Optimizacion: solo bajar si el MBR del hijo contiene al objetivo
                            if (!node->entries[i].rect.Contains(targetRect))
                                   continue;
                            
                            if (!RemoveRec(targetRect, data, node->entries[i].child, reinsertList))
                            {
                                   // Actualizar MBR del hijo
                                   if (node->entries[i].child->entries.empty())
                                   {
                                          delete node->entries[i].child;
                                          node->entries.erase(node->entries.begin() + i);
                                   }
                                   else
                                   {
                                          node->entries[i].rect = node->entries[i].child->ComputeMBR();
                                          
                                          // Verificar underflow en hijo
                                          if (HasUnderflow(node->entries[i].child))
                                          {
                                                 reinsertList.push_back(node->entries[i].child);
                                                 node->entries[i].child = nullptr;
                                                 node->entries.erase(node->entries.begin() + i);
                                          }
                                   }
                                   return false;  // Encontrado
                            }
                     }
                     return true;  // No encontrado
              }
              else
              {
                     // Nodo hoja: buscar por MBR Y dato
                     for (size_t i = 0; i < node->entries.size(); ++i)
                     {
                            if (node->entries[i].rect == targetRect && node->entries[i].data == data)
                            {
                                   node->entries.erase(node->entries.begin() + i);
                                   return false;  // Encontrado y eliminado
                            }
                     }
                     return true;  // No encontrado
              }
       }
       
       // Eliminacion solo por dato (busca en todo el arbol, sin podar)
       bool RemoveByDataRec(const DataType& data, Node* node, std::vector<Node*>& reinsertList)
       {
              if (!node->IsLeaf())
              {
                     for (size_t i = 0; i < node->entries.size(); ++i)
                     {
                            if (!RemoveByDataRec(data, node->entries[i].child, reinsertList))
                            {
                                   if (node->entries[i].child->entries.empty())
                                   {
                                          delete node->entries[i].child;
                                          node->entries.erase(node->entries.begin() + i);
                                   }
                                   else
                                   {
                                          node->entries[i].rect = node->entries[i].child->ComputeMBR();
                                          if (HasUnderflow(node->entries[i].child))
                                          {
                                                 reinsertList.push_back(node->entries[i].child);
                                                 node->entries[i].child = nullptr;
                                                 node->entries.erase(node->entries.begin() + i);
                                          }
                                   }
                                   return false;
                            }
                     }
                     return true;
              }
              else
              {
                     for (size_t i = 0; i < node->entries.size(); ++i)
                     {
                            if (node->entries[i].data == data)
                            {
                                   node->entries.erase(node->entries.begin() + i);
                                   return false;
                            }
                     }
                     return true;
              }
       }
       
       // Reinserta todas las entradas de un nodo
       void ReinsertEntries(Node* node)
       {
              if (node->IsLeaf())
              {
                     for (auto& entry : node->entries)
                     {
                            Insert(entry.rect.xMin, entry.rect.yMin,
                                   entry.rect.xMax, entry.rect.yMax, entry.data);
                            --m_Count;  // Insert incrementa, compensar
                     }
              }
              else
              {
                     for (auto& entry : node->entries)
                     {
                            ReinsertEntries(entry.child);
                            delete entry.child;
                     }
              }
       }
       
       // ==================== BUSQUEDA RECURSIVA ====================
       void SearchRec(Node* node, const RectType& query, std::vector<QueryResult>& results) const
       {
              for (auto& entry : node->entries)
              {
                     if (query.Overlaps(entry.rect))
                     {
                            if (node->IsLeaf())
                                   results.push_back({entry.rect, entry.data});  // Retorna MBR + dato
                            else
                                   SearchRec(entry.child, query, results);
                     }
              }
       }
       
       // ==================== PERSISTENCIA ====================
       void WriteNode(std::ostream& os, Node* node) const
       {
              os << node->level << " " << node->entries.size() << "\n";
              for (auto& entry : node->entries)
              {
                     os << entry.rect.xMin << " " << entry.rect.yMin << " "
                        << entry.rect.xMax << " " << entry.rect.yMax << "\n";
                     if (node->IsLeaf())
                            os << entry.data << "\n";
                     else
                            WriteNode(os, entry.child);
              }
       }
       
       Node* ReadNode(std::istream& is)
       {
              Node* node = new Node();
              size_t n;
              is >> node->level >> n;
              
              for (size_t i = 0; i < n; ++i)
              {
                     Entry entry;
                     is >> entry.rect.xMin >> entry.rect.yMin
                        >> entry.rect.xMax >> entry.rect.yMax;
                     if (node->IsLeaf())
                            is >> entry.data;
                     else
                            entry.child = ReadNode(is);
                     node->entries.push_back(entry);
              }
              return node;
       }
       
       // ==================== IMPRESION ====================
       void PrintNode(std::ostream& os, Node* node, int depth) const
       {
              std::string indent(depth * 2, ' ');
              os << indent << (node->IsLeaf() ? "LEAF" : "INTERNAL")
                 << " [level=" << node->level << ", n=" << node->entries.size() << "]\n";
              
              for (size_t i = 0; i < node->entries.size(); ++i)
              {
                     auto& entry = node->entries[i];
                     os << indent << "  (" << entry.rect.xMin << "," << entry.rect.yMin
                        << ")-(" << entry.rect.xMax << "," << entry.rect.yMax << ")";
                     if (node->IsLeaf())
                            os << " data=" << entry.data;
                     os << "\n";
                     if (!node->IsLeaf())
                            PrintNode(os, entry.child, depth + 1);
              }
       }
};

#endif