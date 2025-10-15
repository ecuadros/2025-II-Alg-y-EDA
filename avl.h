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

    void printBF(ostream &os) {
        os << "Node: " << this->getDataRef() << " (BF: " << m_balanceFactor << ")\n";
    }

    friend class CAVLTree<Traits>;
};


template <typename _T>
struct AVLAscTraits{
    using  T = _T;
    using  Node       = CAVLNode<AVLAscTraits<T>>;
    using  CompareFn  = less<_T>;
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
        // Insertar el nodo en el árbol base
        BaseNode* newNode = Base::internal_insert(elem, ref, pParent, rpOrigin);

        // Recalcular balance factor de todo el árbol
        calculateBalanceFactor((Node*)(Base::m_pRoot));

        CompareFn comp;
        Node* current = (Node*)newNode;

        while (current) {
            current->printBF(cout);
            int balance = current->getBalanceFactor();

            if (balance < -1 && current->getChild(0)&& comp(elem, current->getChild(0)->getData())) {
                rotateRight(current);
            }
            else if (balance > 1 && current->getChild(1)&& comp(current->getChild(1)->getData(), elem)) {
                cout<<"Rotating Left on " << current->getData() << endl;
                rotateLeft(current);
            }
            else if (balance <-1&& current->getChild(0) && comp(current->getChild(0)->getData(), elem)) {
                Node* leftChild = (Node*)current->getChildRef(0);
                rotateLeft(leftChild);
                rotateRight(current); 
            }
            else if (balance >1 && current->getChild(1)&& comp(elem, current->getChild(1)->getData())) {
                Node* rightChild = (Node*)current->getChildRef(1);
                rotateRight(rightChild);
                rotateLeft(current); 
            }

            current = (Node*)current->m_pParent; // subir al siguiente ancestro
        }


        return (BaseNode*)newNode;
    }


private:
    int calculateBalanceFactor(Node* node) {
        if (!node) return 0;
        int left_bf = calculateBalanceFactor((Node*)node->getChild(0));
        int right_bf = calculateBalanceFactor((Node*)node->getChild(1));

        node->setBalanceFactor(right_bf - left_bf);

        //cout<<"BF("<<node->getData()<<") = "<<node->getBalanceFactor()<<endl;
        return 1 + std::max(left_bf, right_bf);
    }

    Node* rotateLeft(Node* root) {
        if (!root) return nullptr;

        Node* newRoot = (Node*)root->getChildRef(1);
        Node* subT = (Node*)newRoot->getChildRef(0);

        if (!newRoot) return root;

        newRoot->getChildRef(0) = root;
        root->getChildRef(1) = subT;
        if (subT) subT->m_pParent = root;

        Node* parent = (Node*)root->m_pParent;

        newRoot->m_pParent = parent;
        root->m_pParent = newRoot;

        if (parent) {
            if (parent->getChild(0) == root)
                parent->getChildRef(0) = newRoot;
            else
                parent->getChildRef(1) = newRoot;
        } else {
            // la raíz
            Base::m_pRoot = newRoot;
        }

        calculateBalanceFactor((Node*)(Base::m_pRoot));
        return newRoot;
    }

    Node* rotateRight(Node* root) {
        if (!root) return nullptr;
        
        Node* newRoot = (Node*)root->getChildRef(0);
        Node* subT = (Node*)newRoot->getChildRef(1);

        if (!newRoot) return root;

        newRoot->getChildRef(1) = root;
        root->getChildRef(0) = subT;
        if (subT) subT->m_pParent = root;

        Node* parent = (Node*)root->m_pParent;

        newRoot->m_pParent = parent;
        root->m_pParent = newRoot;

        if (parent) {
            if (parent->getChild(0) == root)
                parent->getChildRef(0) = newRoot;
            else
                parent->getChildRef(1) = newRoot;
        } else {
            // la raíz
            Base::m_pRoot = newRoot;
        }

        // Actualizar factores de balance 
        calculateBalanceFactor( (Node*)(Base::m_pRoot));
        return root;
    }

public:
    CAVLTree() : Base() {} // Empty tree

    void printBF(ostream &os) {
        printBF((Node*)Base::m_pRoot, 0, os);
    }

    void printBF(Node* pNode, size_t level, ostream &os) {
        if (pNode) {
            Node* pParent = (Node*)pNode->getParent();
            printBF((Node*)pNode->getChild(1), level + 1, os);
            os << string(level * 5, ' ') << " " << pNode->getDataRef() << "("
               << (pParent ? to_string(pParent->getRef()) : "Root") << ")"
               << " [BF=" << pNode->getBalanceFactor() << "]" << endl;
            printBF((Node*)pNode->getChild(0), level + 1, os);
        }
    }

};

#endif // __AVL_H__