#ifndef __RTREEPAGE_H__
#define __RTREEPAGE_H__

#include "rtree_traits.h"
#include <iostream>
#include <vector>

enum rt_ErrorCode { rt_ok, rt_overflow, rt_underflow, rt_duplicate, rt_nofound };

template <typename Trait>
class RTree; 

template <typename Trait>
class RTreePage {
    friend class RTree<Trait>;
    
    using CoordType = typename Trait::CoordType;
    using ObjIDType = typename Trait::ObjIDType;
    using RectType  = typename Trait::RectType;
    
    // Estructura interna para almacenar entradas
    struct Entry {
        RectType    mbr;
        ObjIDType   objID;     // Válido si es hoja
        RTreePage* childPtr;  // Válido si es nodo interno
        
        // Constructor para hojas
        Entry(RectType r, ObjIDType id) : mbr(r), objID(id), childPtr(nullptr) {}
        // Constructor para internos
        Entry(RectType r, RTreePage* ptr) : mbr(r), objID(0), childPtr(ptr) {}
        Entry() : objID(0), childPtr(nullptr) {}
    };

public:
    RTreePage(size_t minEntries, size_t maxEntries, bool isLeaf = true);
    ~RTreePage();

    // Inserción recursiva
    rt_ErrorCode Insert(const Entry& entry);

    // Getters básicos
    bool IsLeaf() const { return m_IsLeaf; }
    size_t Count() const { return m_Entries.size(); }
    
    // Calcular el MBR que cubre todas las entradas de esta página
    RectType GetNodeMBR() const;

protected:
    size_t m_MinEntries;
    size_t m_MaxEntries;
    bool   m_IsLeaf;
    RTreePage* m_Parent; // Útil para propagar cambios de MBR hacia arriba
    
    std::vector<Entry> m_Entries;

    //Inserción - logica

    // Elige el mejor sub-árbol para insertar (el que requiera menor expansión)
    RTreePage* ChooseSubtree(const RectType& mbr);

    // Ajusta el árbol hacia arriba (actualiza MBRs padres y maneja splits)
    void AdjustTree(RTreePage* node, RTreePage* splitSibling);

    // Retorna el nuevo nodo hermano creado
    RTreePage* SplitNode(Entry& extraEntry);

    // Ayudantes para el Split Cuadrático
    void PickSeeds(std::vector<Entry>& allEntries, size_t& seed1, size_t& seed2);
    size_t PickNext(std::vector<Entry>& entries, const RectType& group1MBR, const RectType& group2MBR);

    // Inserta una entrada localmente en el vector
    void AddEntry(const Entry& entry);
    
    // Destruye y resetea
    void Reset();
};


template <typename Trait>
RTreePage<Trait>::RTreePage(size_t minEntries, size_t maxEntries, bool isLeaf)
    : m_MinEntries(minEntries), m_MaxEntries(maxEntries), m_IsLeaf(isLeaf), m_Parent(nullptr) {
    m_Entries.reserve(m_MaxEntries + 1); // +1 para manejar el overflow temporal
}

template <typename Trait>
RTreePage<Trait>::~RTreePage() {
    Reset();
}

template <typename Trait>
void RTreePage<Trait>::Reset() {
    if (!m_IsLeaf) {
        for (auto& e : m_Entries) {
            delete e.childPtr;
        }
    }
    m_Entries.clear();
}

template <typename Trait>
rt_ErrorCode RTreePage<Trait>::Insert(const Entry& entry) {
    // 1. Si es hoja, intentamos insertar aquí
    if (m_IsLeaf) {
        AddEntry(entry);
        if (m_Entries.size() > m_MaxEntries) {
            return rt_overflow; // Señal para que el caller (RTree o Padre) maneje el split
        }
        return rt_ok;
    }

    // 2. Si es nodo interno, elegir mejor hijo
    RTreePage* bestChild = ChooseSubtree(entry.mbr);
    
    // 3. Insertar recursivamente
    rt_ErrorCode result = bestChild->Insert(entry);

  
    // Recalcular MBR de la entrada que apunta al hijo modificado
    for (auto& e : m_Entries) {
        if (e.childPtr == bestChild) {
            e.mbr = bestChild->GetNodeMBR();
            break;
        }
    }
    
    return result;
}

template <typename Trait>
RTreePage<Trait>* RTreePage<Trait>::ChooseSubtree(const typename Trait::RectType& rect) {
    // Buscar la entrada cuyo MBR requiera la MENOR expansión para incluir 'rect'
    double minEnlargement = std::numeric_limits<double>::max();
    double minArea = std::numeric_limits<double>::max();
    RTreePage* bestChild = nullptr;

    for (auto& entry : m_Entries) {
        double enlargement = entry.mbr.Enlargement(rect);
        double area = entry.mbr.Area();

        if (enlargement < minEnlargement) {
            minEnlargement = enlargement;
            minArea = area;
            bestChild = entry.childPtr;
        } else if (enlargement == minEnlargement) {
            // Desempate por área menor
            if (area < minArea) {
                minArea = area;
                bestChild = entry.childPtr;
            }
        }
    }
    return bestChild; // Retorna el puntero a la página hija
}

template <typename Trait>
void RTreePage<Trait>::AddEntry(const Entry& entry) {
    m_Entries.push_back(entry);
}

template <typename Trait>
typename Trait::RectType RTreePage<Trait>::GetNodeMBR() const {
    if (m_Entries.empty()) return typename Trait::RectType();
    
    typename Trait::RectType hull = m_Entries[0].mbr;
    for (size_t i = 1; i < m_Entries.size(); ++i) {
        hull.Merge(m_Entries[i].mbr);
    }
    return hull;
}

#endif