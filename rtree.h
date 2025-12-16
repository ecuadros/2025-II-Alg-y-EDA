#ifndef __RTREE_H__
#define __RTREE_H__

#include <iostream>
#include <shared_mutex>
#include <fstream>
#include <sstream>
#include <string>
#include "rtreepage.h"

#define DEFAULT_RTREE_ORDER 4

/**
 * @brief R-Tree (Rectangle Tree) - Estructura de indexación espacial
 * @tparam Trait RTreeTrait con configuración
 */
template <typename Trait>
class RTree {
public:
    using RectType = typename Trait::RectType;
    using PointType = typename Trait::PointType;
    using ObjIDType = typename Trait::ObjIDType;
    using CoordType = typename Trait::CoordType;
    using ObjectInfo = tagRTreeObjectInfo<Trait>;
    using RTPage = CRTreePage<Trait>;

public:
    /**
     * @brief Constructor
     * @param maxEntries Capacidad máxima de cada página (M)
     */
    RTree(size_t maxEntries = DEFAULT_RTREE_ORDER)
        : m_MaxEntries(maxEntries),
          m_MinEntries(maxEntries / 2),
          m_Height(1),
          m_NumObjects(0) {
        m_Root = new RTPage(m_MaxEntries, true);
    }

    /**
     * @brief Destructor
     */
    ~RTree() {
        delete m_Root;
    }

    /**
     * @brief Move constructor
     */
    RTree(RTree&& other) noexcept {
        std::lock_guard<std::shared_mutex> lock(other.m_Mutex);
        m_MaxEntries = other.m_MaxEntries;
        m_MinEntries = other.m_MinEntries;
        m_Root = other.m_Root;
        m_Height = other.m_Height;
        m_NumObjects = other.m_NumObjects;
        other.m_Root = nullptr;
        other.m_NumObjects = 0;
        other.m_Height = 0;
    }

    // Getters
    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(m_Mutex);
        return m_NumObjects;
    }

    size_t height() const {
        std::shared_lock<std::shared_mutex> lock(m_Mutex);
        return m_Height;
    }

    size_t GetMaxEntries() const {
        std::shared_lock<std::shared_mutex> lock(m_Mutex);
        return m_MaxEntries;
    }

    /**
     * @brief Inserta un objeto espacial
     * @param mbr Minimum Bounding Rectangle del objeto
     * @param objID Identificador del objeto
     * @return true si se insertó correctamente
     */
    bool Insert(const RectType& mbr, ObjIDType objID);

    /**
     * @brief Elimina un objeto espacial
     * @param mbr MBR del objeto a eliminar
     * @param objID ID del objeto a eliminar
     * @return true si se eliminó
     */
    bool Remove(const RectType& mbr, ObjIDType objID);

    /**
     * @brief Busca todos los objetos que intersectan un rectángulo
     * @param query Rectángulo de consulta
     * @param results Vector donde se almacenan los resultados
     */
    void Search(const RectType& query, std::vector<ObjectInfo>& results) const;

    /**
     * @brief Busca todos los objetos que contienen un punto
     * @param point Punto de consulta
     * @param results Vector donde se almacenan los resultados
     */
    void SearchPoint(const PointType& point, std::vector<ObjectInfo>& results) const;

    /**
     * @brief Imprime el árbol completo
     */
    void Print(std::ostream& os) const {
        std::shared_lock<std::shared_mutex> lock(m_Mutex);
        os << "RTree: Height=" << m_Height
           << " Objects=" << m_NumObjects
           << " MaxEntries=" << m_MaxEntries << "\n";
        if (m_Root)
            m_Root->Print(os, 0);
    }

    // Iteradores
    using iterator = RTreeForwardIterator<Trait>;
    using reverse_iterator = RTreeBackwardIterator<Trait>;

    iterator begin() {
        std::shared_lock<std::shared_mutex> lock(m_Mutex);
        if (m_NumObjects == 0)
            return iterator();
        return iterator(m_Root);
    }

    iterator end() {
        return iterator();
    }

    reverse_iterator rbegin() {
        std::shared_lock<std::shared_mutex> lock(m_Mutex);
        if (m_NumObjects == 0)
            return reverse_iterator();
        return reverse_iterator(m_Root);
    }

    reverse_iterator rend() {
        return reverse_iterator();
    }

    /**
     * @brief Aplica una función a cada objeto del árbol
     * @tparam Func Tipo de función
     * @tparam Args Tipos de argumentos adicionales
     */
    template <typename Func, typename... Args>
    void ForEach(Func&& func, Args&&... args) {
        std::shared_lock<std::shared_mutex> lock(m_Mutex);
        if (m_Root)
            ForEachRecursive(m_Root, std::forward<Func>(func), std::forward<Args>(args)...);
    }

    /**
     * @brief Busca el primer objeto que cumple un predicado
     * @tparam Pred Tipo de predicado
     * @tparam Args Tipos de argumentos adicionales
     */
    template <typename Pred, typename... Args>
    ObjectInfo* FirstThat(Pred&& pred, Args&&... args) {
        std::shared_lock<std::shared_mutex> lock(m_Mutex);
        if (m_Root)
            return FirstThatRecursive(m_Root, std::forward<Pred>(pred), std::forward<Args>(args)...);
        return nullptr;
    }

    // Persistencia
    void Write(std::ostream& os);
    void Write(const std::string& filename);
    void Read(std::istream& is);
    void Read(const std::string& filename);

    template <typename T>
    friend std::ostream& operator<<(std::ostream& os, RTree<T>& obj);

private:
    RTPage*     m_Root;
    size_t      m_MaxEntries;
    size_t      m_MinEntries;
    size_t      m_Height;
    size_t      m_NumObjects;
    mutable std::shared_mutex m_Mutex;

    // Métodos auxiliares de inserción
    RTPage* ChooseLeaf(const RectType& mbr);
    void AdjustTree(RTPage* page, RTPage* newPage);
    RTPage* SplitPage(RTPage* page);

    // Métodos auxiliares de eliminación
    RTPage* FindLeaf(RTPage* page, const RectType& mbr, ObjIDType objID);
    void CondenseTree(RTPage* page, std::vector<ObjectInfo>& orphans);
    void CollectLeafEntries(RTPage* page, std::vector<ObjectInfo>& orphans);

    // Métodos auxiliares de búsqueda
    void SearchRecursive(RTPage* page, const RectType& query, std::vector<ObjectInfo>& results) const;

    // Métodos auxiliares para ForEach/FirstThat
    template <typename Func, typename... Args>
    void ForEachRecursive(RTPage* page, Func&& func, Args&&... args);

    template <typename Pred, typename... Args>
    ObjectInfo* FirstThatRecursive(RTPage* page, Pred&& pred, Args&&... args);

    // Helpers de I/O
    void WriteNode(std::ostream& os, RTPage* page, size_t level);
    RTPage* ReadNode(std::istream& is, RTPage* parent);
};

/**
 * @brief Inserta un objeto en el R-Tree
 */
template <typename Trait>
bool RTree<Trait>::Insert(const RectType& mbr, ObjIDType objID) {
    std::lock_guard<std::shared_mutex> lock(m_Mutex);

    // 1. Encontrar hoja donde insertar
    RTPage* leaf = ChooseLeaf(mbr);

    // 2. Agregar entrada a la hoja
    ObjectInfo entry(mbr, objID);
    leaf->AddEntry(entry);

    // 3. Manejar overflow con split si es necesario
    RTPage* newLeaf = nullptr;
    if (leaf->IsFull()) {
        newLeaf = SplitPage(leaf);
    }

    // 4. Ajustar árbol propagando cambios
    AdjustTree(leaf, newLeaf);

    // 5. Si la raíz hizo split, crear nueva raíz
    if (newLeaf != nullptr && leaf == m_Root) {
        RTPage* newRoot = new RTPage(m_MaxEntries, false);

        ObjectInfo entry1(leaf->GetMBR(), -1);
        newRoot->AddEntry(entry1, leaf);

        ObjectInfo entry2(newLeaf->GetMBR(), -1);
        newRoot->AddEntry(entry2, newLeaf);

        m_Root = newRoot;
        m_Height++;
    }

    m_NumObjects++;
    return true;
}

/**
 * @brief Encuentra la hoja óptima donde insertar un MBR
 */
template <typename Trait>
typename RTree<Trait>::RTPage*
RTree<Trait>::ChooseLeaf(const RectType& mbr) {
    RTPage* page = m_Root;

    while (!page->IsLeaf()) {
        size_t bestIndex = page->ChooseBestEntry(mbr);
        page = page->m_Children[bestIndex];
    }

    return page;
}

/**
 * @brief Propaga cambios de MBR hacia arriba en el árbol
 */
template <typename Trait>
void RTree<Trait>::AdjustTree(RTPage* page, RTPage* newPage) {
    while (page != m_Root) {
        RTPage* parent = page->GetParent();

        // Actualizar MBR del nodo en la entrada del padre
        for (size_t i = 0; i < parent->m_Children.size(); ++i) {
            if (parent->m_Children[i] == page) {
                parent->m_Entries[i].mbr = page->GetMBR();
                break;
            }
        }

        // Si hubo split, agregar el nuevo nodo al padre
        if (newPage != nullptr) {
            ObjectInfo newEntry(newPage->GetMBR(), -1);
            parent->AddEntry(newEntry, newPage);

            if (parent->IsFull()) {
                newPage = SplitPage(parent);
            } else {
                newPage = nullptr;
            }
        }

        parent->UpdateMBR();
        page = parent;
    }

    m_Root->UpdateMBR();
}

/**
 * @brief Divide una página llena en dos páginas
 */
template <typename Trait>
typename RTree<Trait>::RTPage*
RTree<Trait>::SplitPage(RTPage* page) {
    // 1. Seleccionar semillas
    size_t seed1, seed2;
    LinearPickSeeds<Trait>(page->m_Entries, seed1, seed2);

    // 2. Crear nueva página
    RTPage* newPage = new RTPage(m_MaxEntries, page->IsLeaf());

    // 3. Copiar todas las entradas
    std::vector<ObjectInfo> allEntries = page->m_Entries;
    std::vector<RTPage*> allChildren;
    if (!page->IsLeaf())
        allChildren = page->m_Children;

    // 4. Limpiar página original
    page->m_Entries.clear();
    page->m_Children.clear();

    // 5. Agregar semillas
    page->AddEntry(allEntries[seed1],
                   page->IsLeaf() ? nullptr : allChildren[seed1]);
    newPage->AddEntry(allEntries[seed2],
                      page->IsLeaf() ? nullptr : allChildren[seed2]);

    // 6. Distribuir entradas restantes
    for (size_t i = 0; i < allEntries.size(); ++i) {
        if (i == seed1 || i == seed2) continue;

        auto enlargement1 = page->GetMBR().enlargement(allEntries[i].mbr);
        auto enlargement2 = newPage->GetMBR().enlargement(allEntries[i].mbr);

        RTPage* targetPage;
        if (enlargement1 < enlargement2)
            targetPage = page;
        else if (enlargement2 < enlargement1)
            targetPage = newPage;
        else
            targetPage = (page->GetMBR().area() < newPage->GetMBR().area())
                        ? page : newPage;

        targetPage->AddEntry(allEntries[i],
                            page->IsLeaf() ? nullptr : allChildren[i]);
    }

    // 7. Actualizar MBRs
    page->UpdateMBR();
    newPage->UpdateMBR();

    return newPage;
}

/**
 * @brief Busca todos los objetos que intersectan un rectángulo
 */
template <typename Trait>
void RTree<Trait>::Search(const RectType& query, std::vector<ObjectInfo>& results) const {
    std::shared_lock<std::shared_mutex> lock(m_Mutex);
    results.clear();
    if (m_Root)
        SearchRecursive(m_Root, query, results);
}

/**
 * @brief Búsqueda recursiva interna
 */
template <typename Trait>
void RTree<Trait>::SearchRecursive(RTPage* page, const RectType& query,
                                   std::vector<ObjectInfo>& results) const {
    if (page->IsLeaf()) {
        for (const auto& entry : page->m_Entries) {
            if (query.intersects(entry.mbr)) {
                results.push_back(entry);
            }
        }
    } else {
        for (size_t i = 0; i < page->m_Entries.size(); ++i) {
            if (query.intersects(page->m_Entries[i].mbr)) {
                SearchRecursive(page->m_Children[i], query, results);
            }
        }
    }
}

/**
 * @brief Busca objetos que contienen un punto
 */
template <typename Trait>
void RTree<Trait>::SearchPoint(const PointType& point, std::vector<ObjectInfo>& results) const {
    RectType pointRect;
    for (size_t i = 0; i < Trait::Dimensions; ++i) {
        pointRect.min[i] = point[i];
        pointRect.max[i] = point[i];
    }
    Search(pointRect, results);
}

/**
 * @brief Elimina un objeto del R-Tree
 */
template <typename Trait>
bool RTree<Trait>::Remove(const RectType& mbr, ObjIDType objID) {
    std::lock_guard<std::shared_mutex> lock(m_Mutex);

    // 1. Encontrar hoja que contiene el objeto
    RTPage* leaf = FindLeaf(m_Root, mbr, objID);
    if (leaf == nullptr)
        return false;

    // 2. Eliminar entrada de la hoja
    for (size_t i = 0; i < leaf->m_Entries.size(); ++i) {
        if (leaf->m_Entries[i].ObjID == objID &&
            leaf->m_Entries[i].mbr == mbr) {
            leaf->RemoveEntry(i);
            break;
        }
    }

    // 3. Condensar árbol y recolectar entradas huérfanas
    std::vector<ObjectInfo> orphans;
    CondenseTree(leaf, orphans);

    // 4. Reinsertar entradas huérfanas
    for (const auto& orphan : orphans) {
        RTPage* newLeaf = ChooseLeaf(orphan.mbr);
        newLeaf->AddEntry(orphan);

        RTPage* splitPage = nullptr;
        if (newLeaf->IsFull()) {
            splitPage = SplitPage(newLeaf);
        }

        AdjustTree(newLeaf, splitPage);

        if (splitPage != nullptr && newLeaf == m_Root) {
            RTPage* newRoot = new RTPage(m_MaxEntries, false);
            ObjectInfo entry1(newLeaf->GetMBR(), -1);
            newRoot->AddEntry(entry1, newLeaf);
            ObjectInfo entry2(splitPage->GetMBR(), -1);
            newRoot->AddEntry(entry2, splitPage);
            m_Root = newRoot;
            m_Height++;
        }
    }

    // 5. Si la raíz quedó con un solo hijo, bajar la altura
    if (!m_Root->IsLeaf() && m_Root->GetNumEntries() == 1) {
        RTPage* oldRoot = m_Root;
        m_Root = m_Root->m_Children[0];
        m_Root->m_pParent = nullptr;

        oldRoot->m_Children.clear();
        delete oldRoot;
        m_Height--;
    }

    m_NumObjects--;
    return true;
}

/**
 * @brief Encuentra la hoja que contiene un objeto
 */
template <typename Trait>
typename RTree<Trait>::RTPage*
RTree<Trait>::FindLeaf(RTPage* page, const RectType& mbr, ObjIDType objID) {
    if (page->IsLeaf()) {
        for (const auto& entry : page->m_Entries) {
            if (entry.ObjID == objID && entry.mbr == mbr)
                return page;
        }
        return nullptr;
    } else {
        for (size_t i = 0; i < page->m_Entries.size(); ++i) {
            if (page->m_Entries[i].mbr.contains(mbr)) {
                RTPage* result = FindLeaf(page->m_Children[i], mbr, objID);
                if (result != nullptr)
                    return result;
            }
        }
        return nullptr;
    }
}

/**
 * @brief Condensa el árbol después de una eliminación
 */
template <typename Trait>
void RTree<Trait>::CondenseTree(RTPage* page, std::vector<ObjectInfo>& orphans) {
    std::vector<RTPage*> eliminatedPages;

    while (page != m_Root) {
        RTPage* parent = page->GetParent();

        if (page->IsUnderflow()) {
            // Encontrar índice del nodo en el padre
            size_t index = 0;
            for (size_t i = 0; i < parent->m_Children.size(); ++i) {
                if (parent->m_Children[i] == page) {
                    index = i;
                    break;
                }
            }

            parent->RemoveEntry(index);

            if (page->IsLeaf()) {
                for (const auto& entry : page->m_Entries)
                    orphans.push_back(entry);
            } else {
                eliminatedPages.push_back(page);
            }

            page->m_Children.clear();
            delete page;

        } else {
            for (size_t i = 0; i < parent->m_Children.size(); ++i) {
                if (parent->m_Children[i] == page) {
                    parent->m_Entries[i].mbr = page->GetMBR();
                    break;
                }
            }
            parent->UpdateMBR();
        }

        page = parent;
    }

    for (RTPage* eliminated : eliminatedPages) {
        CollectLeafEntries(eliminated, orphans);
    }
}

/**
 * @brief Recolecta todas las entradas de hojas bajo una página
 */
template <typename Trait>
void RTree<Trait>::CollectLeafEntries(RTPage* page, std::vector<ObjectInfo>& orphans) {
    if (page->IsLeaf()) {
        for (const auto& entry : page->m_Entries)
            orphans.push_back(entry);
    } else {
        for (RTPage* child : page->m_Children)
            CollectLeafEntries(child, orphans);
    }
}

/**
 * @brief ForEach recursivo
 */
template <typename Trait>
template <typename Func, typename... Args>
void RTree<Trait>::ForEachRecursive(RTPage* page, Func&& func, Args&&... args) {
    if (page->IsLeaf()) {
        for (auto& entry : page->m_Entries) {
            std::invoke(std::forward<Func>(func), entry, std::forward<Args>(args)...);
        }
    } else {
        for (RTPage* child : page->m_Children) {
            ForEachRecursive(child, std::forward<Func>(func), std::forward<Args>(args)...);
        }
    }
}

/**
 * @brief FirstThat recursivo
 */
template <typename Trait>
template <typename Pred, typename... Args>
typename RTree<Trait>::ObjectInfo*
RTree<Trait>::FirstThatRecursive(RTPage* page, Pred&& pred, Args&&... args) {
    if (page->IsLeaf()) {
        for (auto& entry : page->m_Entries) {
            if (std::invoke(std::forward<Pred>(pred), entry, std::forward<Args>(args)...))
                return &entry;
        }
    } else {
        for (RTPage* child : page->m_Children) {
            auto* result = FirstThatRecursive(child, std::forward<Pred>(pred), std::forward<Args>(args)...);
            if (result != nullptr)
                return result;
        }
    }
    return nullptr;
}

/**
 * @brief Escribe el árbol a un stream
 */
template <typename Trait>
void RTree<Trait>::Write(std::ostream& os) {
    std::shared_lock<std::shared_mutex> lock(m_Mutex);
    os << "RTREE " << m_Height << " " << m_NumObjects << " " << m_MaxEntries << "\n";
    if (m_Root)
        WriteNode(os, m_Root, 0);
}

/**
 * @brief Escribe una página recursivamente
 */
template <typename Trait>
void RTree<Trait>::WriteNode(std::ostream& os, RTPage* page, size_t level) {
    std::string indent(level, '\t');

    os << indent << (page->IsLeaf() ? "LEAF" : "INTERNAL")
       << " " << page->GetNumEntries() << "\n";

    for (size_t i = 0; i < page->m_Entries.size(); ++i) {
        os << indent << "\t";

        const auto& mbr = page->m_Entries[i].mbr;
        for (size_t d = 0; d < Trait::Dimensions; ++d)
            os << mbr.min[d] << " ";
        for (size_t d = 0; d < Trait::Dimensions; ++d)
            os << mbr.max[d] << " ";

        os << page->m_Entries[i].ObjID << "\n";

        if (!page->IsLeaf())
            WriteNode(os, page->m_Children[i], level + 1);
    }
}

/**
 * @brief Guarda el árbol en un archivo
 */
template <typename Trait>
void RTree<Trait>::Write(const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        Write(file);
        file.close();
    }
}

/**
 * @brief Lee el árbol desde un stream
 */
template <typename Trait>
void RTree<Trait>::Read(std::istream& is) {
    std::lock_guard<std::shared_mutex> lock(m_Mutex);

    delete m_Root;

    std::string header;
    is >> header;
    if (header != "RTREE") return;

    is >> m_Height >> m_NumObjects >> m_MaxEntries;
    m_MinEntries = m_MaxEntries / 2;

    m_Root = ReadNode(is, nullptr);
}

/**
 * @brief Lee una página recursivamente
 */
template <typename Trait>
typename RTree<Trait>::RTPage*
RTree<Trait>::ReadNode(std::istream& is, RTPage* parent) {
    std::string nodeType;
    size_t numEntries;
    is >> nodeType >> numEntries;

    bool isLeaf = (nodeType == "LEAF");
    RTPage* page = new RTPage(m_MaxEntries, isLeaf);
    page->m_pParent = parent;

    for (size_t i = 0; i < numEntries; ++i) {
        RectType mbr;
        for (size_t d = 0; d < Trait::Dimensions; ++d)
            is >> mbr.min[d];
        for (size_t d = 0; d < Trait::Dimensions; ++d)
            is >> mbr.max[d];

        ObjIDType objID;
        is >> objID;

        ObjectInfo entry(mbr, objID);

        RTPage* child = nullptr;
        if (!isLeaf)
            child = ReadNode(is, page);

        page->AddEntry(entry, child);
    }

    page->UpdateMBR();
    return page;
}

/**
 * @brief Lee el árbol desde un archivo
 */
template <typename Trait>
void RTree<Trait>::Read(const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        Read(file);
        file.close();
    }
}

/**
 * @brief Operador de salida para RTree
 */
template <typename Trait>
std::ostream& operator<<(std::ostream& os, RTree<Trait>& tree) {
    tree.Write(os);
    return os;
}

#endif // __RTREE_H__
