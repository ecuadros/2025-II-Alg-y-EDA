#ifndef __AVL_H__
#define __AVL_H__

#include <algorithm>
#include <functional>
#include "binarytree.h"

template <typename Traits>
class CAVLNode : public CBinaryTreeNode<Traits> {
public:
    using Base = CBinaryTreeNode<Traits>;
    using T    = typename Traits::T;        // alias de valor (consistente con tu BinaryTree)
    using Base::Base;                        // hereda constructores del nodo base

protected:
    int m_balanceFactor = 0;                 // BF = altura(izq) - altura(der)

public:
    int& bf()       { return m_balanceFactor; }
    int  bf() const { return m_balanceFactor; }
};

template <typename _T>
struct AVLAscTraits{
    using  value_type = _T;
    using Node      = CAVLNode<AVLAscTraits<_T>>;
    using CompareFn = std::less<T>;
};

template <typename _T>
struct AVLDescTraits{
    using  value_type = _T;
    using  Node       = CAVLNode<AVLDescTraits<_T>>;
    using  CompareFn  = std::greater<T>;
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

    static inline Node*& L(Node* n) { return n->getChildRef(0); }
    static inline Node*& R(Node* n) { return n->getChildRef(1); }
    static inline Node*  P(Node* n) { return n ? n->getParent() : nullptr; }

    // Conectar p -> hijo en posición pos, y fijar el parent del hijo
    static inline void link(Node* p, Node* child, int pos) {
        if (p) p->setupChild(child, static_cast<size_t>(pos));
        if (child) child->m_pParent = p;     // requiere: friend CAVLTree en CBinaryTreeNode
    }

    // Recolocar sub-raíz en la raíz del árbol
    inline void attach_as_root(Node* n) {
        this->m_pRoot = n;
        if (n) n->m_pParent = nullptr;
    }

    // Para simplificar y evitar errores, recomputo BF local con alturas
    static int height(Node* n) {
        if (!n) return 0;
        return 1 + std::max(height(L(n)), height(R(n)));
    }
    static inline void recomputeBF(Node* n) {
        if (!n) return;
        n->bf() = height(L(n)) - height(R(n));
    }

    Node* rotateLeft(Node* x) {
        Node* y  = R(x);
        Node* yl = L(y);

        // y sube; x queda como hijo izquierdo de y
        link(x,  yl, 1);   // x->right = yl ; if(yl) yl->parent = x
        link(y,  x,  0);   // y->left  = x  ; x->parent  = y

        // reenganchar y con el antiguo padre de x
        Node* px = P(y);   // ojo: tras link(y,x,0), y->parent == old parent de x
        Node* old_px = P(x);
        if (!old_px) {
            attach_as_root(y);
        } else if (L(old_px) == x) {
            link(old_px, y, 0);
        } else {
            link(old_px, y, 1);
        }

        // BF locales (seguro y claro)
        recomputeBF(x);
        recomputeBF(y);
        return y;          // nueva sub-raíz del subárbol
    }

    Node* rotateRight(Node* y) {
        Node* x  = L(y);
        Node* xr = R(x);

        link(y,  xr, 0);   // y->left  = xr
        link(x,  y,  1);   // x->right = y

        Node* old_py = P(y);
        if (!old_py) {
            attach_as_root(x);
        } else if (L(old_py) == y) {
            link(old_py, x, 0);
        } else {
            link(old_py, x, 1);
        }

        recomputeBF(y);
        recomputeBF(x);
        return x;
    }

    // Rebalance local
    Node* rebalance(Node* z) {
        if (!z) return z;
        if (z->bf() > 1) {                 // pesado a la izquierda
            if (L(z)->bf() < 0)            // LR
                rotateLeft(L(z));
            return rotateRight(z);          // LL o tras LR
        }
        if (z->bf() < -1) {                // pesado a la derecha
            if (R(z)->bf() > 0)            // RL
                rotateRight(R(z));
            return rotateLeft(z);           // RR o tras RL
        }
        return z;
    }
    void fixUpAfterInsert(Node* x) {
        Node* p = P(x);
        while (p) {
            // +1 si insertaste como hijo IZQ de p, -1 si DER
            if (L(p) == x) p->bf() += 1;
            else           p->bf() -= 1;

            if (p->bf() == 0) break;              // altura de p no cambió
            if (p->bf() == 2 || p->bf() == -2) {  // desbalance -> rotar
                Node* newSub = rebalance(p);
                (void)newSub;
                // Tras una rotación por inserción, puede detenerse
                break;
            }
            x = p;
            p = P(p);
        }
    }
    Node *internal_insert(const value_type &elem, Ref ref,
                          Node* pParent, Node*& rpOrigin) override
    {
        Node* newNode = Base::internal_insert(elem, ref, pParent, rpOrigin);
        if (!newNode) return nullptr;

        static_cast<CAVLNode<Traits>*>(newNode)->bf() = 0;  // nodo nuevo balanceado
        fixUpAfterInsert(newNode);
        return newNode;
    }


public:
    CAVLTree() : Base() {} // Empty tree

};

#endif // __AVL_H__