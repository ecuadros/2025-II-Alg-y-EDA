#ifndef __AVL_H__
#define __AVL_H__

#include "binarytree.h"
#include <algorithm>

template <typename Traits>
class CAVLNode : public CBinaryTreeNode<Traits>{
public:
  using value_type = typename Traits::T;
  using Node       = CAVLNode<Traits>;
  using Base       = CBinaryTreeNode<Traits>;
protected:
    int     m_balanceFactor = 0;
public:
    CAVLNode(Node* pParent, value_type data, Ref ref, Node* p0 = nullptr, Node* p1 = nullptr)
        : Base(pParent, data, ref, p0, p1), m_balanceFactor(0) {}

    int getBalanceFactor() const { return m_balanceFactor; }
    void setBalanceFactor(int bf) { m_balanceFactor = bf; }

    int updateBalanceFactor() {
        int leftHeight = Base::m_pChild[0] ? static_cast<Node*>(Base::m_pChild[0])->getHeight() : 0;
        int rightHeight = Base::m_pChild[1] ? static_cast<Node*>(Base::m_pChild[1])->getHeight() : 0;
        m_balanceFactor = rightHeight - leftHeight;
        return m_balanceFactor;
    }

    int getHeight() const {
        int leftHeight = Base::m_pChild[0] ? static_cast<Node*>(Base::m_pChild[0])->getHeight() : 0;
        int rightHeight = Base::m_pChild[1] ? static_cast<Node*>(Base::m_pChild[1])->getHeight() : 0;
        return 1 + std::max(leftHeight, rightHeight);
    }
};

template <typename _T>
struct AVLAscTraits{
    using  T         = _T;
    using  Node      = CAVLNode<AVLAscTraits<_T>>;
    using  CompareFn = std::less<_T>;
};

template <typename _T>
struct AVLDescTraits{
    using  T         = _T;
    using  Node      = CAVLNode<AVLDescTraits<_T>>;
    using  CompareFn = std::greater<_T>;
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

protected:
    Node* rotateLeft(Node* pNode) {
        Node* pRight = static_cast<Node*>(pNode->getChildRef(1));
        pNode->getChildRef(1) = pRight->getChildRef(0);
        if (pRight->getChild(0))
            static_cast<BaseNode*>(pRight->getChild(0))->getParentRef() = pNode;

        pRight->getChildRef(0) = pNode;
        pRight->getParentRef() = pNode->getParent();
        pNode->getParentRef() = pRight;

        pNode->updateBalanceFactor();
        pRight->updateBalanceFactor();

        return pRight;
    }

    Node* rotateRight(Node* pNode) {
        Node* pLeft = static_cast<Node*>(pNode->getChildRef(0));
        pNode->getChildRef(0) = pLeft->getChildRef(1);
        if (pLeft->getChild(1))
            static_cast<BaseNode*>(pLeft->getChild(1))->getParentRef() = pNode;

        pLeft->getChildRef(1) = pNode;
        pLeft->getParentRef() = pNode->getParent();
        pNode->getParentRef() = pLeft;

        pNode->updateBalanceFactor();
        pLeft->updateBalanceFactor();

        return pLeft;
    }

    Node* balance(Node* pNode) {
        if (!pNode) return nullptr;

        pNode->updateBalanceFactor();
        int bf = pNode->getBalanceFactor();

        if (bf > 1) {
            Node* pRight = static_cast<Node*>(pNode->getChild(1));
            if (pRight && pRight->getBalanceFactor() < 0)
                pNode->getChildRef(1) = rotateRight(pRight);
            return rotateLeft(pNode);
        }

        if (bf < -1) {
            Node* pLeft = static_cast<Node*>(pNode->getChild(0));
            if (pLeft && pLeft->getBalanceFactor() > 0)
                pNode->getChildRef(0) = rotateLeft(pLeft);
            return rotateRight(pNode);
        }

        return pNode;
    }

    BaseNode* CreateNode(BaseNode* pParent, value_type elem, Ref ref) {
        return new Node(static_cast<Node*>(pParent), elem, ref);
    }

    BaseNode* internal_insert(value_type &elem, Ref ref, BaseNode* pParent, BaseNode*& rpOrigin) {
        if (!rpOrigin) {
            ++Base::m_size;
            return (rpOrigin = CreateNode(pParent, elem, ref));
        }

        CompareFn compFn;
        size_t branch = compFn(elem, rpOrigin->getDataRef()) ? 0 : 1;
        BaseNode*& childRef = reinterpret_cast<BaseNode*&>(rpOrigin->getChildRef(branch));
        internal_insert(elem, ref, rpOrigin, childRef);

        rpOrigin = static_cast<BaseNode*>(balance(static_cast<Node*>(rpOrigin)));

        return rpOrigin;
    }

public:
    CAVLTree() : Base() {}

    CAVLTree(CAVLTree &other) : Base() {
        if (other.m_pRoot) {
            auto copyNode = [this](BaseNode* pNode, size_t level) {
                value_type data = pNode->getDataRef();
                this->insert(data, 0);
            };
            const_cast<CAVLTree&>(other).inorder(copyNode);
        }
    }

    virtual ~CAVLTree() {}

    void insert(value_type elem, Ref ref) {
        Base::m_pRoot = internal_insert(elem, ref, nullptr, Base::m_pRoot);
    }
};

#endif // __AVL_H__