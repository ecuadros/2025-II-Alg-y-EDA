#ifndef __AVL_H__
#define __AVL_H__

#include "binarytree.h"

template <typename Traits>
class CAVLNode : public CBinaryTreeNode<Traits>{
public:
  using value_type = typename Traits::T;
  using BaseNode   = CBinaryTreeNode<Traits>;
  using Node       = CAVLNode<Traits>;

protected:
    int m_balanceFactor = 0;

public:
    CAVLNode(Node* pParent, value_type data, Ref ref, Node* p0 = nullptr, Node* p1 = nullptr)
        : BaseNode(pParent, data, ref, p0, p1), m_balanceFactor(0) {}

    int getBalanceFactor() const { return m_balanceFactor; }
    void setBalanceFactor(int bf) { m_balanceFactor = bf; }
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
    using BaseNode   = typename Base::Node;
    using Node       = typename Traits::Node;
    using value_type = typename Traits::T;
    using CompareFn  = typename Traits::CompareFn;
    using Container  = CAVLTree<Traits>;
    using iterator   = binary_tree_iterator<Container>;

protected:
    int getHeight(Node* pNode) {
        if (!pNode) return 0;
        int leftHeight = getHeight((Node*)pNode->getChild(0));
        int rightHeight = getHeight((Node*)pNode->getChild(1));
        return 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
    }

    int getBalanceFactor(Node* pNode) {
        if (!pNode) return 0;
        return getHeight((Node*)pNode->getChild(0)) - getHeight((Node*)pNode->getChild(1));
    }

    Node* rotateRight(Node* y) {
        Node* x = (Node*)y->getChild(0);
        Node* T2 = (Node*)x->getChild(1);

        x->m_pChild[1] = y;
        y->m_pChild[0] = T2;

        x->m_pParent = y->m_pParent;
        y->m_pParent = x;
        if (T2) T2->m_pParent = y;

        y->setBalanceFactor(getBalanceFactor(y));
        x->setBalanceFactor(getBalanceFactor(x));

        return x;
    }

    Node* rotateLeft(Node* x) {
        Node* y = (Node*)x->getChild(1);
        Node* T2 = (Node*)y->getChild(0);

        y->m_pChild[0] = x;
        x->m_pChild[1] = T2;

        y->m_pParent = x->m_pParent;
        x->m_pParent = y;
        if (T2) T2->m_pParent = x;

        x->setBalanceFactor(getBalanceFactor(x));
        y->setBalanceFactor(getBalanceFactor(y));

        return y;
    }

    Node* CreateNode(Node* pParent, value_type elem, Ref ref) {
        return new Node(pParent, elem, ref);
    }

    Node* internal_insert(value_type &elem, Ref ref,
                          Node* pParent, Node*& rpOrigin) override
    {
        if (!rpOrigin) {
            ++Base::m_size;
            return (rpOrigin = CreateNode(pParent, elem, ref));
        }

        CompareFn Compfn;
        size_t branch = Compfn(elem, rpOrigin->getDataRef()) ? 0 : 1;
        rpOrigin->m_pChild[branch] = internal_insert(elem, ref, rpOrigin, (Node*&)rpOrigin->m_pChild[branch]);

        int balance = getBalanceFactor(rpOrigin);

        if (balance > 1 && Compfn(elem, ((Node*)rpOrigin->getChild(0))->getDataRef())) {
            return rotateRight(rpOrigin);
        }

        if (balance < -1 && !Compfn(elem, ((Node*)rpOrigin->getChild(1))->getDataRef())) {
            return rotateLeft(rpOrigin);
        }

        if (balance > 1 && !Compfn(elem, ((Node*)rpOrigin->getChild(0))->getDataRef())) {
            rpOrigin->m_pChild[0] = rotateLeft((Node*)rpOrigin->getChild(0));
            return rotateRight(rpOrigin);
        }

        if (balance < -1 && Compfn(elem, ((Node*)rpOrigin->getChild(1))->getDataRef())) {
            rpOrigin->m_pChild[1] = rotateRight((Node*)rpOrigin->getChild(1));
            return rotateLeft(rpOrigin);
        }

        rpOrigin->setBalanceFactor(getBalanceFactor(rpOrigin));
        return rpOrigin;
    }

public:
    CAVLTree() : Base() {}

};

#endif // __AVL_H__