#ifndef __AVL_H__
#define __AVL_H__
#include <algorithm>   
#include <functional>
#include "binarytree.h"
#include "types.h"

template <typename Traits>
class CAVLNode : public CBinaryTreeNode<value_type>{
public:
  using value_type = typename Traits::T;
  using Node       = CBinaryTreeNode<value_type>;
  int& bf()       { return m_balanceFactor; }
  int  bf() const { return m_balanceFactor; }
protected:
    int     m_balanceFactor = 0; // Balance factor for AVL tree
    static inline Node*& L(Node* n) { return n->getChildRef(0); }
    static inline Node*& R(Node* n) { return n->getChildRef(1); }
    static inline Node*  P(Node* n) { return n ? n->getParent() : nullptr; }
public:
void link(Node* p, Node* child, int pos) {
    if (p) p->setupChild(child, static_cast<size_t>(pos));
    if (child) child->m_pParent = p; // asegúrate de tener acceso/friend
}

void attach_as_root(Node* n) {
    this->m_pRoot = n;
    if (n) n->m_pParent = nullptr;
}

Node* rotateLeft(Node* x) {
    Node* y = R(x);
    Node* B = L(y);
    link(x, B, 1);         // x->right = B
    link(y, x, 0);         // y->left  = x
    int xbf = x->bf();
    int ybf = y->bf();

    Node* px = P(y);       // antiguo padre de x
    Node* old_px = P(x);
    if (!old_px) attach_as_root(y);
    else if (L(old_px) == x) link(old_px, y, 0);
    else                     link(old_px, y, 1);

    // Actualiza BF (fórmulas constantes o recomputo local)
    // versión simple (segura): recalcula por alturas o usa casos conocidos
    x->bf() = xbf - 1 - std::max(ybf, 0);
    y->bf() = ybf - 1 + std::min(xbf, 0);
    return y;
}

Node* rotateRight(Node* y) {
    Node* x = L(y);
    Node* B = R(x);
    link(y, B, 0);
    link(x, y, 1);
    int ybf = y->bf();
    int xbf = x->bf();

    Node* old_py = P(y);
    if (!old_py) attach_as_root(x);
    else if (L(old_py) == y) link(old_py, x, 0);
    else                     link(old_py, x, 1);

    y->bf() = ybf + 1 + std::max(xbf, 0);
    x->bf() = xbf + 1 - std::min(ybf, 0);
    return x;
}

Node* rebalance(Node* z) {
    if (z->bf() == 2) {              // pesado a izq
        if (L(z)->bf() < 0) rotateLeft(L(z));  // LR
        return rotateRight(z);                  // LL o tras LR
    }
    if (z->bf() == -2) {             // pesado a der
        if (R(z)->bf() > 0) rotateRight(R(z)); // RL
        return rotateLeft(z);                   // RR o tras RL
    }
    return z;
}

void fixUpAfterInsert(Node* x) {
    Node* p = P(x);
    while (p) {
        if (L(p) == x) p->bf() += 1; else p->bf() -= 1;

        if (p->bf() == 0) break;            // altura no cambió -> parar
        if (p->bf() == 2 || p->bf() == -2) {
            rebalance(p);                   // corrige con rotación
            break;                          // en inserción, una rotación basta
        }
        x = p;
        p = P(p);
    }
}



};

template <typename _T>
struct AVLAscTraits{
    using  value_type = _T;
    using  Node       = CAVLNode<T>;
    using  CompareFn  = less<T>;
};

template <typename _T>
struct AVLDescTraits{
    using  value_type = _T;
    using  Node       = CAVLNode<T>;
    using  CompareFn  = greater<T>;
};

template <typename Traits>
class CAVLTree : public CBinaryTree<Traits> {
public:
    using Base       = CBinaryTree<Traits>;
    using Node       = typename Traits::Node;
    using value_type = typename Traits::value_type;  
    using CompareFn  = typename Traits::CompareFn;
    using Container  = CAVLTree<Traits>;
    using iterator   = binary_tree_iterator<Container>;

protected:
    // Additional members for AVL tree balancing can be added here
    static inline Node*& L(Node* n) { return n->getChildRef(0); }  
    static inline Node*& R(Node* n) { return n->getChildRef(1); }
    static inline Node*  P(Node* n) { return n ? n->getParent() : nullptr; }

    // Conectar p->child en pos (0: left, 1: right) y fijar parent
    static inline void link(Node* p, Node* child, int pos) {
        if (p) p->setpChild(child, static_cast<size_t>(pos));  
        if (child) child->m_pParent = p;                       
    }

    inline void attach_as_root(Node* n) {
        this->m_pRoot = n;                                     
        if (n) n->m_pParent = nullptr;                         
    }
    Node* rotateLeft(Node* x) {
        Node* y  = R(x);
        Node* B  = L(y);

        link(x, B, 1); // x->right = B
        link(y, x, 0); // y->left  = x

        Node* old_px = P(x);
        if (!old_px)             attach_as_root(y);
        else if (L(old_px) == x) link(old_px, y, 0);
        else                     link(old_px, y, 1);

        int xbf = static_cast<CAVLNode<Traits>*>(x)->bf();
        int ybf = static_cast<CAVLNode<Traits>*>(y)->bf();
        static_cast<CAVLNode<Traits>*>(x)->bf() = xbf - 1 - std::max(ybf, 0);
        static_cast<CAVLNode<Traits>*>(y)->bf() = ybf - 1 + std::min(xbf, 0);
        return y;
    }

    Node* rotateRight(Node* y) {
        Node* x  = L(y);
        Node* B  = R(x);

        link(y, B, 0); // y->left  = B
        link(x, y, 1); // x->right = y

        Node* old_py = P(y);
        if (!old_py)             attach_as_root(x);
        else if (L(old_py) == y) link(old_py, x, 0);
        else                     link(old_py, x, 1);

        int ybf = static_cast<CAVLNode<Traits>*>(y)->bf();
        int xbf = static_cast<CAVLNode<Traits>*>(x)->bf();
        static_cast<CAVLNode<Traits>*>(y)->bf() = ybf + 1 + std::max(xbf, 0);
        static_cast<CAVLNode<Traits>*>(x)->bf() = xbf + 1 - std::min(ybf, 0);
        return x;
    }
    Node* rebalance(Node* z) {
        auto* az = static_cast<CAVLNode<Traits>*>(z);
        if (az->bf() > 1) {                           // pesado a la izquierda
            if (static_cast<CAVLNode<Traits>*>(L(z))->bf() < 0)
                rotateLeft(L(z));                     // LR
            return rotateRight(z);                    // LL o tras LR
        }
        if (az->bf() < -1) {                          // pesado a la derecha
            if (static_cast<CAVLNode<Traits>*>(R(z))->bf() > 0)
                rotateRight(R(z));                    // RL
            return rotateLeft(z);                     // RR o tras RL
        }
        return z;
    }
    void fixUpAfterInsert(Node* x) {
        Node* p = P(x);
        while (p) {
            auto* ap = static_cast<CAVLNode<Traits>*>(p);
            if (L(p) == x) ap->bf() += 1; else ap->bf() -= 1;

            if (ap->bf() == 0) break;                 // altura de p no cambió
            if (ap->bf() == 2 || ap->bf() == -2) {    // desbalance -> rotar
                rebalance(p);
                break;                                // inserción: basta una rotación
            }
            x = p;
            p = P(p);
        }
    }
 
    Node *internal_insert(value_type &elem, Ref ref,
                          Node* pParent, Node*& rpOrigin) override
    {
        // Call base class insert
        Node* newNode = Base::internal_insert(elem, ref, pParent, rpOrigin);
        // TODO Hecho: Verificar balance y realizar rotaciones si es necesario
        if (!n) return nullptr;

        static_cast<CAVLNode<Traits>*>(n)->bf() = 0; // nuevo nodo balanceado
        fixUpAfterInsert(n);  
        return newNode;
    }
public:
    CAVLTree() : Base() {} // Empty tree

};

#endif // __AVL_H__