#ifndef __AVL_H__
#define __AVL_H__

#include "binarytree.h"

template <typename Traits>
class CAVLTree;
template <typename Traits>
class CAVLNode : public CBinaryTreeNode<Traits>{
public:
    using value_type = typename Traits::T;
    using Node       = CAVLNode<Traits>;

protected:
    int m_balanceFactor = 0;

public:
    CAVLNode(Node* pParent, value_type data, Ref ref, Node* p0 = nullptr, Node* p1 = nullptr)
        : CBinaryTreeNode<Traits>(pParent, data, ref, p0, p1), m_balanceFactor(0)
    {}

    int getBalanceFactor() const { return m_balanceFactor; }
    void setBalanceFactor(int bf) { m_balanceFactor = bf; }
    void updateBalanceFactor(int delta) { m_balanceFactor += delta; }

    friend class CAVLTree<Traits>;
};


template <typename _T>
struct AVLAscTraits{
    using  T = _T;
    using  Node       = CAVLNode<AVLAscTraits<T>>;
    using  CompareFn  = less<T>;
};

template <typename _T>
struct AVLDescTraits{
    using  T = _T;
    using  Node       = CAVLNode<T>;
    using  CompareFn  = greater<T>;
};

template <typename Traits>
class CAVLTree : public CBinaryTree<Traits> {
public:
    using Base       = CBinaryTree<Traits>;
    using BaseNode   = typename Base::Node;
    using Node       = typename Traits::Node;
    using value_type = typename Traits::T;
    using CompareFn  = typename Traits::CompareFn;

protected:
    BaseNode* internal_insert(value_type &elem, Ref ref,
                              BaseNode* pParent, BaseNode*& rpOrigin) override
    {
        BaseNode* newNode = Base::internal_insert(elem, ref, pParent, rpOrigin);

        return newNode;
    }

private:
    void rotateLeft(Node*& root) {
        Node* newRoot = root->getRight();
        root->getRightRef() = newRoot->getLeft();
        if (root->getRight()) root->getRight()->m_pParent = root;

        newRoot->m_pParent = root->m_pParent;
        root->m_pParent = newRoot;
        newRoot->getLeftRef() = root;
        root = newRoot;

        // Actualizar factores de balance
    }

    void rotateRight(Node*& root) {
        Node* newRoot = root->getLeft();
        root->getLeftRef() = newRoot->getRight();
        if (root->getLeft()) root->getLeft()->m_pParent = root;

        newRoot->m_pParent = root->m_pParent;
        root->m_pParent = newRoot;
        newRoot->getRightRef() = root;
        root = newRoot;

        // Actualizar factores de balance 
        
    }

public:
    CAVLTree() : Base() {} // Empty tree

};

#endif // __AVL_H__