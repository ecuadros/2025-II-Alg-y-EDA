#ifndef __AVL_H__
#define __AVL_H__

#include "binarytree.h"
#include <algorithm>
#include <map>

// Árbol AVL que hereda de CBinaryTree y mantiene el balance automáticamente
template <typename Traits>
class CAVLTree : public CBinaryTree<Traits> {
public:
    using Base = CBinaryTree<Traits>;
    using Node = typename Base::Node;
    using value_type = typename Traits::T;

private:
    // Mapa para rastrear las alturas de cada nodo
    std::map<Node*, int> m_heights;

protected:
    // Obtiene la altura de un nodo (0 si es nullptr)
    int getHeight(Node* pNode) const {
        if (!pNode) return 0;
        auto it = m_heights.find(pNode);
        return it != m_heights.end() ? it->second : 1;
    }

    // Actualiza la altura de un nodo basándose en sus hijos
    void updateHeight(Node* pNode) {
        if (!pNode) return;
        int leftHeight = getHeight(pNode->getChild(0));
        int rightHeight = getHeight(pNode->getChild(1));
        m_heights[pNode] = 1 + std::max(leftHeight, rightHeight);
    }

    // Calcula el factor de balance (altura_izquierda - altura_derecha)
    // Valores permitidos: -1, 0, 1 (fuera de rango requiere rotación)
    int getBalanceFactor(Node* pNode) const {
        if (!pNode) return 0;
        return getHeight(pNode->getChild(0)) - getHeight(pNode->getChild(1));
    }

    // Rotación simple a la derecha (caso Left-Left)
    Node* rotateRight(Node* y) {
        Node* x = y->getChild(0);
        Node* B = x->getChild(1);
        
        // Realizar rotación
        x->getChildRef(1) = y;
        y->getChildRef(0) = B;
        
        // Actualizar padres
        if (B) B->m_pParent = y;
        x->m_pParent = y->m_pParent;
        y->m_pParent = x;
        
        // Actualizar alturas (primero y, luego x)
        updateHeight(y);
        updateHeight(x);
        return x;
    }

    // Rotación simple a la izquierda (caso Right-Right)
    Node* rotateLeft(Node* x) {
        Node* y = x->getChild(1);
        Node* B = y->getChild(0);
        
        // Realizar rotación
        y->getChildRef(0) = x;
        x->getChildRef(1) = B;
        
        // Actualizar padres
        if (B) B->m_pParent = x;
        y->m_pParent = x->m_pParent;
        x->m_pParent = y;
        
        // Actualizar alturas (primero x, luego y)
        updateHeight(x);
        updateHeight(y);
        return y;
    }

    // Balancea un nodo después de inserción
    Node* balance(Node* pNode) {
        if (!pNode) return nullptr;
        
        // Actualizar altura del nodo actual
        updateHeight(pNode);
        int balanceFactor = getBalanceFactor(pNode);
        
        // Caso Left-Left: hijo izquierdo está desbalanceado a la izquierda
        if (balanceFactor > 1 && getBalanceFactor(pNode->getChild(0)) >= 0) {
            return rotateRight(pNode);
        }
        
        // Caso Left-Right: hijo izquierdo está desbalanceado a la derecha
        // Requiere rotación doble: primero left en hijo, luego right en nodo
        if (balanceFactor > 1 && getBalanceFactor(pNode->getChild(0)) < 0) {
            pNode->getChildRef(0) = rotateLeft(pNode->getChild(0));
            return rotateRight(pNode);
        }
        
        // Caso Right-Right: hijo derecho está desbalanceado a la derecha
        if (balanceFactor < -1 && getBalanceFactor(pNode->getChild(1)) <= 0) {
            return rotateLeft(pNode);
        }
        
        // Caso Right-Left: hijo derecho está desbalanceado a la izquierda
        // Requiere rotación doble: primero right en hijo, luego left en nodo
        if (balanceFactor < -1 && getBalanceFactor(pNode->getChild(1)) > 0) {
            pNode->getChildRef(1) = rotateRight(pNode->getChild(1));
            return rotateLeft(pNode);
        }
        
        // Nodo ya está balanceado
        return pNode;
    }

    // Override de inserción para agregar balanceo automático
    virtual Node* internal_insert(value_type elem, Ref ref, Node* pParent, Node*& rpOrigin) override {
        // Caso base: crear nuevo nodo
        if (!rpOrigin) {
            ++this->m_size;
            Node* newNode = this->CreateNode(pParent, elem, ref);
            m_heights[newNode] = 1; // Altura inicial de hoja
            return (rpOrigin = newNode);
        }
        
        // Inserción recursiva usando la función de comparación
        size_t branch = this->Compfn(elem, rpOrigin->getDataRef()) ? 0 : 1;
        rpOrigin->getChildRef(branch) = internal_insert(elem, ref, rpOrigin, rpOrigin->getChildRef(branch));
        
        // Balancear el nodo después de la inserción
        return balance(rpOrigin);
    }

public:
    CAVLTree() : Base() {}

    // Imprime el árbol mostrando altura (h) y factor de balance (bf) de cada nodo
    void printWithBalance(std::ostream& os) {
        printWithBalanceImpl(this->m_pRoot, 0, os);
    }

private:
    // Implementación recursiva de impresión con información de balance
    void printWithBalanceImpl(Node* pNode, size_t level, std::ostream& os) {
        if (pNode) {
            // Imprimir subárbol derecho primero (para visualización horizontal)
            printWithBalanceImpl(pNode->getChild(1), level + 1, os);
            
            Node* pParent = pNode->getParent();
            os << std::string(level * 3, ' ') << "|- " << pNode->getDataRef()
               << " (h:" << getHeight(pNode)           // Altura del nodo
               << " bf:" << getBalanceFactor(pNode)    // Factor de balance
               << " p:" << (pParent ? std::to_string(pParent->getData()) : "Root") << ")"
               << std::endl;
            
            // Imprimir subárbol izquierdo
            printWithBalanceImpl(pNode->getChild(0), level + 1, os);
        }
    }
};

// Reuso los Traits del binarytree.h para el AVL
template <typename _T>
using AVLAscTraits = BinaryTreeAscTraits<_T>;

template <typename _T>
using AVLDescTraits = BinaryTreeDescTraits<_T>;

#endif