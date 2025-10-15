#ifndef __AVL_H__
#define __AVL_H__

#include "binarytree.h"

template <typename Traits>
class CAVLNode : public CBinaryTreeNode<Traits>
{
    public:
        using T          = typename Traits::T;
        using Node       = CAVLNode<Traits>;
        using BaseNode   = CBinaryTreeNode<Traits>;
protected:
    int     m_balanceFactor = 0; // Balance factor for AVL tree

        template <typename N> // to allow access to private members
        friend std::string getTreeDisplay(const N* root);
    public:
        CAVLNode(BaseNode* parent, const T& value, Ref ref)
            : CBinaryTreeNode<Traits>(parent, value, ref) {}
        int getBalanceFactor() const { return m_balanceFactor; }
        void setBalanceFactor(int bf) { m_balanceFactor = bf; }
};

template <typename _T>
struct AVLAscTraits{
    using  T          = _T;
    using  Node       = CBinaryTreeNode<AVLAscTraits<_T>>;
    using  CompareFn  = less<T>;
};

template <typename Traits>
struct AVLDescTraits{
    using  T          = typename Traits::T;
    using  Node       = CBinaryTreeNode<AVLDescTraits<T>>;
    using  CompareFn  = greater<T>;
};

template <typename Traits>
class CAVLTree : public CBinaryTree<Traits> {
public:
    using Base       = CBinaryTree<Traits>;
    using Node       = typename Base::Node;
    using AVLNode    = CAVLNode<Traits>;
    using T         = typename Base::T;
    using CompareFn = typename Base::CompareFn;
    using Container = CAVLTree<Traits>;
    using iterator  = binary_tree_forward_iterator<Container>;
protected:
    Node* createNode(Node* parent, const T& value, Ref ref)
    {
            return new AVLNode(parent, value, ref);
        }

        int getHeight(Node* node) const
        {
            if (node == nullptr) return 0;
            int leftHeight = getHeight(node->m_children[0]);
            int rightHeight = getHeight(node->m_children[1]);
            return 1 + std::max(leftHeight, rightHeight);
        }

        int getBalance(Node* node) const
        {
            if (node == nullptr) return 0;
            return getHeight(node->m_children[0]) - getHeight(node->m_children[1]);
        }

        Node* internal_insert(const T& value, Ref ref, Node* parent, Node*& node) override
        {
            // paso 1: bst.insert() normal
            if (node == nullptr) {
                this->m_size++;
                AVLNode* newNode = new AVLNode(parent, value, ref);
                newNode->setBalanceFactor(0);
                return newNode;
            }

            size_t branch = this->m_compare(value, node->getValue()) ? 0 : 1;
            node->m_children[branch] = internal_insert(value, ref, node, node->m_children[branch]);

            // paso 2: actualizar factor de balance
            AVLNode* avlNode = static_cast<AVLNode*>(node);
            int balance = getBalance(node);
            avlNode->setBalanceFactor(balance);

            // paso 3: si el nodo está desbalanceado, realizar rotaciones
            if (balance > 1 && getBalance(node->m_children[0]) >= 0) { // LL
                return rotateRight(avlNode);
            }

            if (balance < -1 && getBalance(node->m_children[1]) <= 0) { // RR
                return rotateLeft(avlNode);
            }

            if (balance > 1 && getBalance(node->m_children[0]) < 0) { // LR
                node->m_children[0] = rotateLeft(static_cast<AVLNode*>(node->m_children[0]));
                return rotateRight(avlNode);
            }

            if (balance < -1 && getBalance(node->m_children[1]) > 0) { // RL
                node->m_children[1] = rotateRight(static_cast<AVLNode*>(node->m_children[1]));
                return rotateLeft(avlNode);
            }

            return node;
        }

        Node* rotateLeft(AVLNode* node)
        {
            AVLNode* newRoot = static_cast<AVLNode*>(node->m_children[1]);
            Node* temp = newRoot->m_children[0];

            newRoot->m_children[0] = node;
            node->m_children[1] = temp;

            newRoot->m_parent = node->m_parent;
            node->m_parent = newRoot;
            if (temp) temp->m_parent = node;

            node->setBalanceFactor(getBalance(node));
            newRoot->setBalanceFactor(getBalance(newRoot));

            return newRoot;
        }

        Node* rotateRight(AVLNode* node)
        {
            AVLNode* newRoot = static_cast<AVLNode*>(node->m_children[0]);
            Node* temp = newRoot->m_children[1];

            newRoot->m_children[1] = node;
            node->m_children[0] = temp;
            
            newRoot->m_parent = node->m_parent;
            node->m_parent = newRoot;
            if (temp) temp->m_parent = node;
            
            node->setBalanceFactor(getBalance(node));
            newRoot->setBalanceFactor(getBalance(newRoot));
            
            return newRoot;
        }

public:
    CAVLTree() : Base() {} // Empty tree

};

#endif // __AVL_H__