#ifndef __AVL_H__
#define __AVL_H__

#include "binarytree.h"

// ==================== AVL NODE ====================
template <typename Traits>
class CAVLNode : public CBinaryTreeNode<Traits> {
public:
    using value_type = typename Traits::T;
    using Node       = CAVLNode<Traits>;
    int m_balanceFactor = 0;

    CAVLNode(Node* pParent, value_type data, Ref ref, Node* p0 = nullptr, Node* p1 = nullptr)
        : CBinaryTreeNode<Traits>(pParent, data, ref, p0, p1), m_balanceFactor(0) {}

    int getBalance() const { return m_balanceFactor; }
    void setBalance(int bf) { m_balanceFactor = bf; }
    void setParent(Node* parent) { this->m_pParent = parent; }
    Node* getParent() { return static_cast<Node*>(this->m_pParent); }
};

// ========== TRAITS ==========
template <typename _T>
struct AVLAscTraits {
    using T         = _T;
    using Node      = CAVLNode<AVLAscTraits<_T>>;
    using CompareFn = std::less<_T>;
};

template <typename _T>
struct AVLDescTraits {
    using T         = _T;
    using Node      = CAVLNode<AVLDescTraits<_T>>;
    using CompareFn = std::greater<_T>;
};

// ================== AVL TREE =====================
template <typename Traits>
class CAVLTree : public CBinaryTree<Traits> {
public:
    using Base       = CBinaryTree<Traits>;
    using Node       = typename Traits::Node;
    using value_type = typename Traits::T;
    using CompareFn  = typename Traits::CompareFn;
    using Container  = CAVLTree<Traits>;
    using iterator   = binary_tree_forward_iterator<Container>;

protected:
    int getHeight(Node* node) {
        if (!node) return 0;
        return 1 + std::max(getHeight(static_cast<Node*>(node->getChild(0))),
                            getHeight(static_cast<Node*>(node->getChild(1))));
    }

    int updateBalance(Node* node) {
        if (!node) return 0;
        int leftH  = getHeight(static_cast<Node*>(node->getChild(0)));
        int rightH = getHeight(static_cast<Node*>(node->getChild(1)));
        int bf = rightH - leftH;
        node->setBalance(bf);
        return std::max(leftH, rightH) + 1;
    }

    Node* rotateLeft(Node* x) {
        Node* y = static_cast<Node*>(x->getChild(1));
        x->setpChild(y->getChild(0), 1);
        if (y->getChild(0)) static_cast<Node*>(y->getChild(0))->setParent(x);
        y->setpChild(x, 0);
        y->setParent(x->getParent());
        x->setParent(y);
        updateBalance(x);
        updateBalance(y);
        return y;
    }

    Node* rotateRight(Node* y) {
        Node* x = static_cast<Node*>(y->getChild(0));
        y->setpChild(x->getChild(1), 0);
        if (x->getChild(1)) static_cast<Node*>(x->getChild(1))->setParent(y);
        x->setpChild(y, 1);
        x->setParent(y->getParent());
        y->setParent(x);
        updateBalance(y);
        updateBalance(x);
        return x;
    }

    Node* rotateLeftRight(Node* z) {
        z->setpChild(rotateLeft(static_cast<Node*>(z->getChild(0))), 0);
        return rotateRight(z);
    }

    Node* rotateRightLeft(Node* z) {
        z->setpChild(rotateRight(static_cast<Node*>(z->getChild(1))), 1);
        return rotateLeft(z);
    }

    Node* balance(Node* node) {
        if (!node) return node;
        int bf = node->getBalance();
        if (bf < -1) {
            if (static_cast<Node*>(node->getChild(0))->getBalance() <= 0)
                return rotateRight(node);          // LL Case
            else
                return rotateLeftRight(node);      // LR Case
        } else if (bf > 1) {
            if (static_cast<Node*>(node->getChild(1))->getBalance() >= 0)
                return rotateLeft(node);           // RR Case
            else
                return rotateRightLeft(node);      // RL Case
        }
        return node;
    }

    Node *internal_insert(value_type &elem, Ref ref, Node* pParent, Node*& rpOrigin) {
        if (!rpOrigin) {
            rpOrigin = new Node(pParent, elem, ref);
            this->m_size++;
            return rpOrigin;
        }
        CompareFn comp;
        Node* inserted = nullptr;
        if (comp(elem, rpOrigin->getDataRef())) {
            inserted = internal_insert(elem, ref, rpOrigin, reinterpret_cast<Node*&>(rpOrigin->getChildRef(0)));
        } else {
            inserted = internal_insert(elem, ref, rpOrigin, reinterpret_cast<Node*&>(rpOrigin->getChildRef(1)));
        }
        updateBalance(rpOrigin);
        rpOrigin = balance(rpOrigin);
        return inserted;
    }

public:
    CAVLTree() : Base() {}
    void insert(value_type elem, Ref ref) { internal_insert(elem, ref, nullptr, this->m_pRoot); }
};

#endif // __AVL_H__