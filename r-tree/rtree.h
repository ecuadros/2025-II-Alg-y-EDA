#ifndef _RTREE_H_
#define _RTREE_H_

#include "node.h"
#include <memory>
#include <iostream>

template<typename Traits>
class RTree {
public:
    using Node = RNode<Traits>;
    using MBRType  = MBR<Traits>;
    using Ref      = typename Traits::Ref;
    using value_type = typename Traits::T;
    using EntryType = Entry<Traits>;
    using PointType  = Point<Traits>;

    static constexpr size_t DIM = Traits::DIM;
    static constexpr size_t M = Traits::M;
    static constexpr size_t m = Traits::m;

private:
    Node* m_Root;
    size_t m_Height;
    size_t m_Count;

public:
    RTree() {
        m_Root = new Node(0, true, true);
        m_Height = 1;
        m_Count = 0;
    }

    RTree(const RTree& other) {
        m_Root = new Node(*(other.m_Root));
        m_Height = other.m_Height;
        m_Count = other.m_Count;
    }

    RTree( RTree&& other ) noexcept {
        m_Root = other.m_Root;
        m_Height = other.m_Height;
        m_Count = other.m_Count;
        other.m_Root = nullptr;
        other.m_Height = 0;
        other.m_Count = 0;
    }

    RTree& operator=(const RTree& other) {
        if(this != &other) {
            Clear();
            m_Root = new Node(*(other.m_Root));
            m_Height = other.m_Height;
            m_Count = other.m_Count;
        }
        return *this;
    }

    ~RTree() {
        Clear();
    }

    /**
     * @brief Vacía completamente el árbol
     */
    void Clear() {
        if (m_root) {
            DeleteSubtree(m_root);
            m_root = nullptr;
            m_height = 0;
        }
    }

    /**
     * @brief Elimina recursivamente un subárbol
     */
    void DeleteSubtree(NodeType* node) {
        if (!node) return;
        
        if (!node->IsLeaf()) {
            const auto& entries = node->GetEntries();
            for (const auto& entry : entries) {
                if (entry.childNode) {
                    DeleteSubtree(entry.childNode);
                }
            }
        }
        
        delete node;
    }

    /**
     * @brief Inserta un nuevo punto con referencia en el R-Tree
     * @param point Punto a insertar
     * @param ref Referencia asociada al punto
     */
    void Insert(const PointType& point, RefType ref) {
        MBRType mbr(point);
        Insert(mbr, ref);
    }

    /**
     * @brief Inserta un MBR con referencia en el R-Tree
     * @param mbr MBR a insertar
     * @param ref Referencia asociada
     */
    void Insert(const MBRType& mbr, RefType ref) {
        EntryType newEntry(mbr, ref);
        
        NodeType* leaf = ChooseLeaf(mbr);
        
        //Añadir entry al nodo hoja
        if (!leaf->IsFull()) {
            leaf->AddEntry(newEntry);
            AdjustTree(leaf, nullptr);
        } else {
            //Dividir si está lleno
            HandleOverflow(leaf, newEntry);
        }
    }

    /**
     * @brief Elimina una entrada del R-Tree
     * @param mbr MBR de la entrada a eliminar
     * @param ref Referencia de la entrada
     * @return true si se encontró y eliminó, false en caso contrario
     */
    bool Delete(const MBRType& mbr, RefType ref) {
        NodeType* leaf = FindLeaf(m_root, mbr, ref);
        if (!leaf) return false;
        
        size_t index;
        EntryType target(mbr, ref);
        if (!leaf->FindEntry(target, index)) {
            return false;
        }
        leaf->RemoveEntry(index);
        
        CondenseTree(leaf);
        
        if (m_root->GetNumberOfEntries() == 1 && !m_root->IsLeaf()) {
            NodeType* newRoot = m_root->GetEntry(0).childNode;
            newRoot->SetRoot(true);
            delete m_root;
            m_root = newRoot;
            m_height--;
        }
        
        return true;
    }

    /**
     * @brief Busca todas las entradas que se solapan con el MBR de consulta
     * @param query MBR de consulta
     * @return Vector de referencias que se solapan
     */
    std::vector<RefType> Search(const MBRType& query) const {
        std::vector<RefType> results;
        if (m_root) {
            m_root->range_query(query, results);
        }
        return results;
    }

    /**
     * @brief Verifica si el árbol está vacío
     */
    bool Empty() const {
        return !m_root || m_root->GetNumberOfEntries() == 0;
    }

    /**
     * @brief Imprime la estructura del árbol (para debugging)
     */
    void Print(std::ostream& os) const {
        if (m_root) {
            m_root->Print(os, 0);
        } else {
            os << "Empty RTree\n";
        }
    }

private:
    /**
     * @brief Algoritmo ChooseLeaf de Guttman
     */
    NodeType* ChooseLeaf(const MBRType& mbr) {
        NodeType* currentNode = m_root;
        while (!currentNode->IsLeaf()) {
            size_t index = currentNode->ChooseSubtree(mbr);
            currentNode = currentNode->GetEntry(index).childNode;
        }
        return currentNode;
    }

    /**
     * @brief Ajusta el árbol después de una inserción
     */
    void AdjustTree(NodeType* node, NodeType* splitNode) {
        while (!node->IsRoot()) {
            NodeType* parent = FindParent(m_root, node);
            if (!parent) break;
            
            size_t index;
            if (FindChildIndex(parent, node, index)) {
                parent->GetEntry(index).mbr = node->GetMBR();
                
                if (splitNode) {
                    EntryType newEntry(splitNode->GetMBR(), splitNode);
                    if (!parent->IsFull()) {
                        parent->AddEntry(newEntry);
                        splitNode = nullptr;
                    } else {
                        HandleOverflow(parent, newEntry);
                        return;
                    }
                }
            }
            
            node = parent;
        }
        
        if (splitNode) {
            CreateNewRoot(node, splitNode);
        }
    }



}    

#endif // _RTREE_H_ //