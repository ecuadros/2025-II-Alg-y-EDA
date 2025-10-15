#ifndef __AVL_H__
#define __AVL_H__

#include "binarytree.h"

/**
 * Árbol AVL auto-balanceado que hereda de CBinaryTree.
 * Mantiene balance |altura_izq - altura_der| <= 1 mediante rotaciones.
 * Reutiliza nodos de CBinaryTree, solo override internal_insert().
 */

template <typename _T>
struct AVLAscTraits{
    using  T         = _T;
    using  Node      = CBinaryTreeNode<AVLAscTraits<_T>>;
    using  CompareFn = less<_T>;
};

template <typename _T>
struct AVLDescTraits{
    using  T         = _T;
    using  Node      = CBinaryTreeNode<AVLDescTraits<_T>>;
    using  CompareFn = greater<_T>;
};

template <typename Traits>
class CAVLTree : public CBinaryTree<Traits> {
public:
    using Base       = CBinaryTree<Traits>;
    using Node       = typename Traits::Node;
    using value_type = typename Traits::T;  
    using CompareFn  = typename Traits::CompareFn;

protected:
    // Calcula altura de un nodo (número de niveles hasta hoja más lejana)
    int height(Node* n) {
        if (!n) return 0;
        int lh = height(static_cast<Node*>(n->m_pChild[0]));
        int rh = height(static_cast<Node*>(n->m_pChild[1]));
        return 1 + max(lh, rh);
    }
    
    // Factor de balance: height(izq) - height(der). Debe estar en [-1, 0, 1]
    int getBalance(Node* n) {
        if (!n) return 0;
        return height(static_cast<Node*>(n->m_pChild[0])) - 
               height(static_cast<Node*>(n->m_pChild[1]));
    }
    
    // Rotación derecha: y se vuelve hijo derecho de x
    Node* rotateRight(Node* y) {
        Node* x = static_cast<Node*>(y->m_pChild[0]);
        Node* T2 = static_cast<Node*>(x->m_pChild[1]);
        
        x->m_pChild[1] = y;
        y->m_pChild[0] = T2;
        
        if (T2) T2->m_pParent = y;
        x->m_pParent = y->m_pParent;
        y->m_pParent = x;
        
        return x;
    }
    
    // Rotación izquierda: x se vuelve hijo izquierdo de y
    Node* rotateLeft(Node* x) {
        Node* y = static_cast<Node*>(x->m_pChild[1]);
        Node* T2 = static_cast<Node*>(y->m_pChild[0]);
        
        y->m_pChild[0] = x;
        x->m_pChild[1] = T2;
        
        if (T2) T2->m_pParent = x;
        y->m_pParent = x->m_pParent;
        x->m_pParent = y;
        
        return y;
    }
    
    // Override de internal_insert para añadir auto-balanceo
    // Primero inserta como BST estándar, luego verifica y corrige balance
    Node* internal_insert(value_type &elem, Ref ref,
                          Node* pParent, Node*& rpOrigin) override
    {
        // Inserción BST estándar (recursiva)
        if (!rpOrigin) {
            ++this->m_size;
            return (rpOrigin = this->CreateNode(pParent, elem, ref));
        }

        size_t branch = this->Compfn(elem, rpOrigin->m_data) ? 0 : 1;
        rpOrigin->m_pChild[branch] = internal_insert(elem, ref, rpOrigin, 
                                      reinterpret_cast<Node*&>(rpOrigin->m_pChild[branch]));
        
        // TODO: Verificar balance y realizar rotaciones si es necesario
        int balance = getBalance(rpOrigin);
        
        // Caso LL: desbalance izquierdo-izquierdo (balance > 1, insert en left-left)
        if (balance > 1 && rpOrigin->m_pChild[0] && 
            this->Compfn(elem, rpOrigin->m_pChild[0]->m_data))
            return (rpOrigin = rotateRight(rpOrigin));
        
        // Caso RR: desbalance derecho-derecho (balance < -1, insert en right-right)
        if (balance < -1 && rpOrigin->m_pChild[1] && 
            !this->Compfn(elem, rpOrigin->m_pChild[1]->m_data))
            return (rpOrigin = rotateLeft(rpOrigin));
        
        // Caso LR: desbalance izquierdo-derecho (balance > 1, insert en left-right)
        if (balance > 1 && rpOrigin->m_pChild[0] && 
            !this->Compfn(elem, rpOrigin->m_pChild[0]->m_data)) {
            rpOrigin->m_pChild[0] = rotateLeft(static_cast<Node*>(rpOrigin->m_pChild[0]));
            return (rpOrigin = rotateRight(rpOrigin));
        }
        
        // Caso RL: desbalance derecho-izquierdo (balance < -1, insert en right-left)
        if (balance < -1 && rpOrigin->m_pChild[1] && 
            this->Compfn(elem, rpOrigin->m_pChild[1]->m_data)) {
            rpOrigin->m_pChild[1] = rotateRight(static_cast<Node*>(rpOrigin->m_pChild[1]));
            return (rpOrigin = rotateLeft(rpOrigin));
        }
        
        return rpOrigin;
    }

public:
    CAVLTree() : Base() {}
};

#endif // __AVL_H__