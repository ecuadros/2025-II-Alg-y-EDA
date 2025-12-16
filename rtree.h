#ifndef __RTREE_H__
#define __RTREE_H__

#include <shared_mutex>
#include <mutex>
#include <iostream>
#include <vector>
#include <functional> 
#include "rtreepage.h"

#define DEFAULT_RTREE_ORDER 4 

template <typename Trait>
class RTree {
    using PageType  = RTreePage<Trait>;
    using RectType  = typename Trait::RectType;
    using ObjIDType = typename Trait::ObjIDType;
    using Entry     = typename PageType::Entry;

public:
    RTree(size_t maxEntries = DEFAULT_RTREE_ORDER)
        : m_MaxEntries(maxEntries), m_MinEntries(maxEntries / 2), m_Height(1), m_NumObjs(0) 
    {
        // La raíz empieza como hoja
        m_Root = new PageType(m_MinEntries, m_MaxEntries, true);
    }

    ~RTree() {
        if (m_Root) delete m_Root;
    }

    // Inserción Thread-Safe
    bool Insert(const RectType& rect, const ObjIDType& id);

    // Búsqueda (Range Query)
    std::vector<ObjIDType> Search(const RectType& searchRect);

    // Getters de estado
    size_t size() const {
        std::shared_lock<std::shared_mutex> lk(m_mtx);
        return m_NumObjs;
    }
    
    size_t height() const {
        std::shared_lock<std::shared_mutex> lk(m_mtx);
        return m_Height;
    }

protected:
    PageType* m_Root;
    size_t              m_MaxEntries;
    size_t              m_MinEntries;
    size_t              m_Height;
    size_t              m_NumObjs;
    
    mutable std::shared_mutex m_mtx; // Misma concurrencia que en BTree

    // Helpers internos
    void SplitRoot(PageType* oldRoot, PageType* newSibling);
    
    // Helper para verificar intersección si el RectType no tiene el método
    bool CheckIntersection(const RectType& r1, const RectType& r2) const;
};


template <typename Trait>
bool RTree<Trait>::Insert(const RectType& rect, const ObjIDType& id) {
    // Usamos unique_lock porque la inserción modifica la estructura
    std::unique_lock<std::shared_mutex> lk(m_mtx);

    Entry newEntry(rect, id);
    
    // Insertar en la raíz (la recursión hacia abajo ocurre dentro de PageType::Insert)
    rt_ErrorCode result = m_Root->Insert(newEntry);

    if (result == rt_overflow) {
        // Si la raíz se desborda, necesitamos dividirla.
        // En RTreePage, SplitNode se encarga de redistribuir las entradas del nodo lleno
        // y devuelve un puntero al nuevo nodo hermano creado.
        
        Entry dummy; // Placeholder si SplitNode requiere argumento (depende de implementación exacta en rtreepage)
        PageType* newSibling = m_Root->SplitNode(dummy); 

        // Creamos una nueva raíz que apunte al viejo root y al nuevo hermano
        SplitRoot(m_Root, newSibling);
        m_Height++;
    }
    
    m_NumObjs++;
    return true;
}

template <typename Trait>
void RTree<Trait>::SplitRoot(PageType* oldRoot, PageType* newSibling) {
    // Crear nueva raíz (siempre es nodo interno, isLeaf = false)
    PageType* newRoot = new PageType(m_MinEntries, m_MaxEntries, false);
    
    // Crear las entradas que apuntan a los dos hijos (vieja raíz y nueva hoja/rama)
    Entry e1(oldRoot->GetNodeMBR(), oldRoot);
    Entry e2(newSibling->GetNodeMBR(), newSibling);
    
    // Insertar las entradas en la nueva raíz (no debería causar overflow porque tiene capacidad nueva)
    newRoot->Insert(e1); 
    newRoot->Insert(e2); 
    
    // Actualizar punteros de padre (si la estructura PageType lo soporta)
    oldRoot->m_Parent = newRoot;
    newSibling->m_Parent = newRoot;
    
    // Actualizar el puntero maestro de la raíz
    m_Root = newRoot;
}

// Helper simple para intersección N-Dimensional
template <typename Trait>
bool RTree<Trait>::CheckIntersection(const RectType& r1, const RectType& r2) const {
    for (size_t i = 0; i < Trait::Dimension; ++i) {
        if (r1.maxP[i] < r2.minP[i] || r1.minP[i] > r2.maxP[i]) {
            return false;
        }
    }
    return true;
}

template <typename Trait>
std::vector<typename Trait::ObjIDType> RTree<Trait>::Search(const RectType& searchRect) {
    std::shared_lock<std::shared_mutex> lk(m_mtx);
    std::vector<ObjIDType> results;
    
    // Lambda recursiva para recorrer el árbol
    std::function<void(PageType*)> searchRecursive = 
        [&](PageType* node) {
            for (const auto& entry : node->m_Entries) {
              
                if (CheckIntersection(searchRect, entry.mbr)) { 
                    if (node->IsLeaf()) {
                        // Es una hoja y el objeto está dentro (o intersecta) el rango
                        results.push_back(entry.objID);
                    } else {
                        // Es un nodo interno, bajamos a explorar esa rama
                        searchRecursive(entry.childPtr);
                    }
                }
            }
        };

    if (m_Root) searchRecursive(m_Root);
    return results;
}

#endif