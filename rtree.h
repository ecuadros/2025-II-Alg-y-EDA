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
    // Eliminación Thread-Safe
    bool Remove(const RectType& rect, const ObjIDType& id);

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

    // Elimina nodos vacíos/pobres y devuelve una lista de entradas "huerfanas" para reinsertar.
    void CondenseTree(PageType* leafNode, std::vector<Entry>& orphanedEntries);

    // Función auxiliar para encontrar la hoja que contiene el dato
    PageType* FindLeaf(PageType* node, const RectType& rect, const ObjIDType& id);
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

template <typename Trait>
bool RTree<Trait>::Remove(const RectType& rect, const ObjIDType& id) {
    std::unique_lock<std::shared_mutex> lk(m_mtx);

    // 1. Encontrar la hoja que contiene el dato
    PageType* leaf = FindLeaf(m_Root, rect, id);
    if (!leaf) return false; 

    // 2. Eliminar la entrada de la hoja
    bool removed = leaf->RemoveEntry(id);
    if (!removed) return false;

    // 3. CondenseTree: Ajustar MBRs hacia arriba y eliminar nodos con underflow
    std::vector<Entry> orphanedEntries;
    CondenseTree(leaf, orphanedEntries);

    // 4. Ajustar la raíz si quedó con un solo hijo (reducción de altura)
    if (!m_Root->IsLeaf() && m_Root->Count() == 1) {
        PageType* newRoot = m_Root->m_Entries[0].childPtr;
        newRoot->m_Parent = nullptr;
        
        m_Root->m_Entries.clear(); // Evitar borrado recursivo accidental
        delete m_Root;
        m_Root = newRoot;
        m_Height--;
    } 
    else if (m_Root->IsLeaf() && m_Root->Count() == 0) {
        // Árbol totalmente vacío, reseteamos altura
        m_Height = 1;
    }

    // 5. Re-insertar entradas huérfanas (datos de nodos eliminados por underflow)
    for(const auto& e : orphanedEntries) {
        if(e.childPtr == nullptr) { // Es un dato hoja
             Entry newE = e;
             rt_ErrorCode r = m_Root->Insert(newE);
             
             if(r == rt_overflow) {
                 Entry dummy;
                 PageType* sib = m_Root->SplitNode(dummy);
                 SplitRoot(m_Root, sib);
                 m_Height++;
             }
        }
    }

    m_NumObjs--;
    return true;
}

template <typename Trait>
typename RTree<Trait>::PageType* RTree<Trait>::FindLeaf(PageType* node, const RectType& rect, const ObjIDType& id) {
    if (node->IsLeaf()) {
        // Búsqueda secuencial en la hoja
        for (const auto& entry : node->m_Entries) {
            if (entry.objID == id) return node;
        }
        return nullptr;
    }

    // Si es interno, buscamos en los hijos cuyo MBR contenga al objeto
    for (const auto& entry : node->m_Entries) {
        if (entry.mbr.Contains(rect)) {
             PageType* res = FindLeaf(entry.childPtr, rect, id);
             if (res) return res;
        }
    }
    return nullptr;
}

template <typename Trait>
void RTree<Trait>::CondenseTree(PageType* node, std::vector<Entry>& orphanedEntries) {
    PageType* parent = nullptr;
    Entry* entryInParent = nullptr;

    // Subimos desde la hoja hasta la raíz
    while (node != m_Root) {
        parent = node->m_Parent;
        
        // Localizar la entrada en el padre que apunta al nodo actual
        int idxInParent = -1;
        for(size_t i=0; i < parent->m_Entries.size(); ++i) {
            if(parent->m_Entries[i].childPtr == node) {
                idxInParent = i;
                entryInParent = &parent->m_Entries[i];
                break;
            }
        }

        // Caso 1: Underflow (el nodo tiene menos entradas de las permitidas)
        if (node->Count() < m_MinEntries) {
            if (idxInParent != -1) {
                parent->m_Entries.erase(parent->m_Entries.begin() + idxInParent);
            }
            
            for (const auto& e : node->m_Entries) {
                orphanedEntries.push_back(e);
            }
            
            // Eliminamos el nodo físico
            node->m_Entries.clear(); 
            delete node;
            
        } else {
            // Caso 2: El nodo está sano, solo actualizamos su MBR en el padre
            if (entryInParent) {
                entryInParent->mbr = node->GetNodeMBR();
            }
        }
        node = parent;
    }
    
}

#endif