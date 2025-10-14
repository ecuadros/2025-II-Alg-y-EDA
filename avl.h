#ifndef __AVL_H__
#define __AVL_H__

#include "binarytree.h"

template <typename Traits>
class CAVLNode : public CBinaryTreeNode<Traits>{
public:
  using value_type = typename Traits::T;
  using Node       = CAVLNode<Traits>;
protected:
    int m_height = 0;  // Altura del nodo para control de balance
public:
    CAVLNode(Node* pParent, value_type data, Ref ref, Node* p0 = nullptr, Node* p1 = nullptr)
        : CBinaryTreeNode<Traits>(pParent, data, ref, p0, p1), m_height(0) {}
    
    int getHeight() const { return m_height; }
    
    // Balance = altura(izq) - altura(der)
    int getBalanceFactor() {
        int leftH = this->m_pChild[0] ? static_cast<Node*>(this->m_pChild[0])->getHeight() : -1;
        int rightH = this->m_pChild[1] ? static_cast<Node*>(this->m_pChild[1])->getHeight() : -1;
        return leftH - rightH;
    }
    
    // Recalcula altura basada en hijos
    void updateHeight() {
        int leftH = this->m_pChild[0] ? static_cast<Node*>(this->m_pChild[0])->getHeight() : -1;
        int rightH = this->m_pChild[1] ? static_cast<Node*>(this->m_pChild[1])->getHeight() : -1;
        m_height = 1 + max(leftH, rightH);
    }
};

template <typename _T>
struct AVLAscTraits{
    using  T         = _T;
    using  Node      = CAVLNode<AVLAscTraits<_T>>;
    using  CompareFn = less<_T>;
};

template <typename _T>
struct AVLDescTraits{
    using  T         = _T;
    using  Node      = CAVLNode<AVLDescTraits<_T>>;
    using  CompareFn = greater<_T>;
};

template <typename Traits>
class CAVLTree : public CBinaryTree<Traits> {
public:
    using Base       = CBinaryTree<Traits>;
    using Node       = typename Traits::Node;
    using value_type = typename Traits::T;  
    using CompareFn  = typename Traits::CompareFn;
    using Container  = CAVLTree<Traits>;
    //using iterator   = binary_tree_iterator<Container>;

protected:
    Node* CreateNode(Node* pParent, value_type elem, Ref ref) {
        return new Node(pParent, elem, ref);
    }
    
    // Rotacion derecha: para casos LL

    Node* rotateRight(Node* y) {
        Node* x = static_cast<Node*>(y->getChild(0));
        Node* T2 = static_cast<Node*>(x->getChild(1));
        
        x->getChildRef(1) = y;
        y->getChildRef(0) = T2;
        
        if (T2) T2->m_pParent = y;
        x->m_pParent = y->m_pParent;
        y->m_pParent = x;
        
        y->updateHeight();
        x->updateHeight();
        
        return x;
    }
    
    // Rotacion izquierda: para casos RR

    Node* rotateLeft(Node* x) {
        Node* y = static_cast<Node*>(x->getChild(1));
        Node* T2 = static_cast<Node*>(y->getChild(0));
        
        y->getChildRef(0) = x;
        x->getChildRef(1) = T2;
        
        if (T2) T2->m_pParent = x;
        y->m_pParent = x->m_pParent;
        x->m_pParent = y;
        
        x->updateHeight();
        y->updateHeight();
        
        return y;
    }
    
    // Detecta desbalance y aplica rotacion correcta
    // Balance > 1: arbol cargado a la izquierda (LL o LR)
    // Balance < -1: arbol cargado a la derecha (RR o RL)
    Node* balance(Node* pNode) {
        if (!pNode) return nullptr;
        
        pNode->updateHeight();
        int bf = pNode->getBalanceFactor();
        
        // Izquierda pesada
        if (bf > 1) {
            Node* left = static_cast<Node*>(pNode->getChild(0));
            if (left && left->getBalanceFactor() >= 0)
                return rotateRight(pNode);  // Caso LL
            if (left && left->getBalanceFactor() < 0) {
                pNode->getChildRef(0) = rotateLeft(left);  // Caso LR
                return rotateRight(pNode);
            }
        }
        
        // Derecha pesada
        if (bf < -1) {
            Node* right = static_cast<Node*>(pNode->getChild(1));
            if (right && right->getBalanceFactor() <= 0)
                return rotateLeft(pNode);  // Caso RR
            if (right && right->getBalanceFactor() > 0) {
                pNode->getChildRef(1) = rotateRight(right);  // Caso RL
                return rotateLeft(pNode);
            }
        }
        
        return pNode;
    }
    
    // Override insert para agregar balanceo automatico
    Node* internal_insert(value_type &elem, Ref ref,
                          Node* pParent, Node*& rpOrigin) override
    {
        // Insercion BST normal
        if (!rpOrigin) {
            ++this->m_size;
            return (rpOrigin = static_cast<Node*>(this->CreateNode(pParent, elem, ref)));
        }
        
        // Insertar recursivamente
        if (this->Compfn(elem, rpOrigin->getDataRef()))
            rpOrigin->getChildRef(0) = internal_insert(elem, ref, rpOrigin, 
                                        reinterpret_cast<Node*&>(rpOrigin->getChildRef(0)));
        else
            rpOrigin->getChildRef(1) = internal_insert(elem, ref, rpOrigin, 
                                        reinterpret_cast<Node*&>(rpOrigin->getChildRef(1)));
        
        // Balancear despues de insertar
        rpOrigin = balance(rpOrigin);
        return rpOrigin;
    }

public:
    CAVLTree() : Base() {}
};

#endif // __AVL_H__