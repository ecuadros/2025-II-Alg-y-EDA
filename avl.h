#ifndef __AVL_H__
#define __AVL_H__

#include "binarytree.h"

template <typename Traits>
class CAVLTree;

template <typename Traits>
class CAVLNode : public CBinaryTreeNode<Traits>{
public:
    friend class CAVLTree<Traits>;
    using value_type = typename Traits::T;
    using Node       = CAVLNode<Traits>;
    using Base       = CBinaryTreeNode<Traits>;

protected:
    int     m_balanceFactor = 0; // Balance factor for AVL tree

public:
    CAVLNode(Base* pBase, value_type data, Ref ref) : Base(pBase, data, ref) {}

};

template <typename _T>
struct AVLAscTraits{
    using  T = _T;
    using  Node       = CAVLNode<AVLAscTraits<T>>;
    using  CompareFn  = std::less<T>;
};

template <typename _T>
struct AVLDescTraits{
    using  T = _T;
    using  Node       = CAVLNode<AVLDescTraits<T>>;
    using  CompareFn  = std::greater<T>;
};

template <typename Traits>
class CAVLTree : public CBinaryTree<Traits> {
public:
    using Base       = CBinaryTree<Traits>;
    using Node       = typename Traits::Node;
    using value_type = typename Traits::T;  
    using CompareFn  = typename Traits::CompareFn;
    using Container  = CAVLTree<Traits>;
    using iterator   = binary_tree_iterator<Container>;

protected:
    /* ----- Rotaciones AVL ----- */
    Node* rotacionIzq(Node* A){
        Node* B = static_cast<Node*>(A->getChild(1));
        Node* B_izq = static_cast<Node*>(B->getChild(0));

        B->setChild(A, 0);
        A->setChild(B_izq, 1);

        B->setParent(A->getParent());
        A->setParent(B);

        if (B_izq) B_izq->setParent(A);

        A->m_balanceFactor = A->m_balanceFactor - 1 - std::max(B->m_balanceFactor, 0);
        B->m_balanceFactor = B->m_balanceFactor - 1 + std::min(A->m_balanceFactor, 0);

        return B;
    }

    Node* rotacionDer(Node* A){
        Node* B = static_cast<Node*>(A->getChild(0));
        Node* B_der = static_cast<Node*>(B->getChild(1));

        B->setChild(A, 1);
        A->setChild(B_der, 0);

        B->setParent(A->getParent());
        A->setParent(B);
        if (B_der) B_der->setParent(A);

        A->m_balanceFactor = A->m_balanceFactor + 1 - std::min(B->m_balanceFactor, 0);
        B->m_balanceFactor = B->m_balanceFactor + 1 + std::max(A->m_balanceFactor, 0);

        return B;
    }

    void balance(Node* A){
        Node* newRoot = nullptr;

        if (A->m_balanceFactor > 1){
            Node* B = static_cast<Node*>(A->getChild(1));
            if (B->m_balanceFactor < 0){
                A->setChild(rotacionDer(B), 1);
            }
            newRoot = rotacionIzq(A);
        } else if (A->m_balanceFactor < -1){
            Node* B = static_cast<Node*>(A->getChild(0));
            if (B->m_balanceFactor > 0){
                A->setChild(rotacionIzq(B), 0);
            }
            newRoot = rotacionDer(A);
        }

        if (newRoot) {
            Node* pParent = static_cast<Node*>(newRoot->getParent());

            if (!pParent) {
                this->m_pRoot = newRoot;
            } else if (pParent->getChild(0) == A) {
                pParent->setChild(newRoot, 0);
            } else {
                pParent->setChild(newRoot, 1);
            }
        }
    }

public:
    void insert(value_type elem, Ref ref){
        Node* pParent = nullptr;
        Node* pCurrent = static_cast<Node*>(this->m_pRoot);

        while (pCurrent) {
            pParent = pCurrent;
            if (this->Compfn(elem, pCurrent->getData())) {
                pCurrent = static_cast<Node*>(pCurrent->getChild(0));
            } else {
                pCurrent = static_cast<Node*>(pCurrent->getChild(1));
            }
        }

        Node* newNode = static_cast<Node*>(this->CreateNode(pParent, elem, ref));
        this->m_size++;

        if (!pParent) {
            this->m_pRoot = newNode;
        } else if (this->Compfn(elem, pParent->getData())) {
            pParent->getChildRef(0) = newNode;
        } else {
            pParent->getChildRef(1) = newNode;
        }

        Node* pNode = static_cast<Node*>(newNode);
        while (pNode != this->m_pRoot) {
            Node* pParent = static_cast<Node*>(pNode->getParent());
            if (pParent->getChild(0) == pNode) {
                pParent->m_balanceFactor--;
            } else {
                pParent->m_balanceFactor++;
            }

            if (pParent->m_balanceFactor == 0) {
                break;
            }

            if (std::abs(pParent->m_balanceFactor) > 1) {
                balance(pParent);
                break;
            }
            pNode = pParent;
        }       
    }

    CAVLTree() : Base() {} // Empty tree
};

void DemoAVLTree();
#endif // __AVL_H__