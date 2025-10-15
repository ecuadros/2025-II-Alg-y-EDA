#ifndef __AVL_H__
#define __AVL_H__

#include "binarytree.h"

template <typename Traits>
class CAVLNode : public CBinaryTreeNode<Traits>{
public:
  using value_type = typename Traits::T;
  using Node       = CAVLNode<Traits>; // Self-reference
  using Base      = CBinaryTreeNode<Traits>;
protected:
    int     m_balanceFactor = 0; // Balance factor for AVL tree
public:
    // Constructor que utiliza el constructor base
    CAVLNode(typename Base::Node* pParent, value_type data, Ref ref, typename Base::Node* p0 = nullptr, typename Base::Node* p1 = nullptr)
        : Base(pParent, data, ref, p0, p1), m_balanceFactor(0) {}
    // Destructor que utiliza el destructor base
    ~CAVLNode() { 
        // Destructor de CBinaryTreeNode se llama automáticamente
    }

    int  getBalanceFactor() const       { return m_balanceFactor; }
    void setBalanceFactor(int factor)   { m_balanceFactor = factor; }
    void updateBalanceFactor(int delta) { m_balanceFactor += delta; }
    
    // Métodos para obtener hijos y padre con el tipo correcto
    Node* getAVLChild(size_t branch) {
        return static_cast<Node*>(this->getChild(branch));
    }
    
    Node* getAVLParent() {
        return static_cast<Node*>(this->getParent());
    }
};

template <typename _T>
struct AVLAscTraits{
    using  T = _T;
    using  Node       = CAVLNode<AVLAscTraits<_T>>;
    using  CompareFn  = less<_T>;
};

template <typename _T>
struct AVLDescTraits{
    using  T = _T;
    using  Node       = CAVLNode<AVLDescTraits<_T>>;
    using  CompareFn  = greater<_T>;
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
    // Additional members for AVL tree balancing can be added here
    Node* CreateAVLNode(Node* pParent, value_type elem, Ref ref) {
        return new Node(pParent, elem, ref, nullptr, nullptr);
    }

    int getHeight(typename Base::Node* pNode){
        if (!pNode) return -1;
        int leftHeight  = getHeight(pNode->getAVLChild(0));
        int rightHeight = getHeight(pNode->getAVLChild(1));
        return 1 + max(leftHeight, rightHeight);
    }
    
    void updateBalance(Node* pNode){
        if (!pNode) return;
        int leftHeight = getHeight(pNode->getAVLChild(0));
        int rightHeight = getHeight(pNode->getAVLChild(1));
        pNode->setBalanceFactor(rightHeight - leftHeight);
    }
    typename Base::Node *internal_insert(value_type elem, Ref ref,
                          typename Base::Node* pParent, typename Base::Node*& rpOrigin) override
    {
        // Inserción normal en BST
        if (!rpOrigin) {
            ++this->m_size;
            Node* newNode = CreateAVLNode(static_cast<Node*>(pParent), elem, ref);
            rpOrigin = static_cast<typename Base::Node*>(newNode);
            return rpOrigin;
        }
        // Castear rpOrigin a Node*
        Node* avlOrigin = static_cast<Node*>(rpOrigin);
        // Comparar y decidir la rama
        size_t branch = this->Compfn(elem, rpOrigin->getDataRef()) ? 0 : 1;
        // Inserción recursiva con cast correcto
        typename Base::Node* childBase = rpOrigin->getChildRef(branch);
        childBase = internal_insert(elem, ref, rpOrigin, childBase);
        rpOrigin->getChildRef(branch) = childBase;
        
        // TODO: Verificar balance y realizar rotaciones si es necesario
        avlOrigin = balanceNode(avlOrigin);
        rpOrigin = static_cast<typename Base::Node*>(avlOrigin);
        return rpOrigin;
    }
    Node* balanceNode(Node* pNode){
        if (!pNode) return pNode; // Nada que balancear

        updateBalance(pNode);
        int balance = pNode->getBalanceFactor();
        //Rotaciones
        // Caso 1: Rotación derecha 
        if (balance < -1){
            Node* leftChild = pNode->getAVLChild(0);
            if (leftChild && leftChild->getBalanceFactor() <= 0){
                return rotateRight(pNode);
            }
            // Caso 2: Rotación izquierda-derecha 
            if( leftChild && leftChild->getBalanceFactor() > 0){
                pNode->setChild(0, rotateLeft(leftChild));
                return rotateRight(pNode);
            }
        }
        // Caso 3: Rotación izquierda
        if (balance > 1){
            Node* rightChild = pNode->getAVLChild(1);
            if (rightChild && rightChild->getBalanceFactor() >= 0){
                return rotateLeft(pNode);
            }
            // Caso 4: Rotación derecha-izquierda
            if (rightChild && rightChild->getBalanceFactor() < 0){
                pNode->setChild(1, rotateRight(rightChild));
                return rotateLeft(pNode);
            }
        }
        return pNode;
    }
    Node* rotateRight(Node* pNode){
        Node* newRoot = pNode->getAVLChild(0); // Hijo izquierdo
        Node* temp = newRoot->getAVLChild(1);  // Subárbol derecho que rota  
    
        // Rotación
        newRoot->getChildRef(1) = pNode;    // pNode se convierte en derecho
        pNode->getChildRef(0) = temp;      // temp se convierte en izquierdo

        // Actualizar padres
        if (temp) {
            typename Base::Node* tempBase = static_cast<typename Base::Node*>(temp);
            tempBase->m_pParent = static_cast<typename Base::Node*>(pNode);
        }
        static_cast<typename Base::Node*>(pNode)->m_pParent = static_cast<typename Base::Node*>(newRoot);

        updateBalance(pNode);
        updateBalance(newRoot);
        return newRoot; 
    
    }
    Node* rotateLeft(Node* pNode){
        Node* newRoot = pNode->getAVLChild(1); // Hijo derecho
        Node* temp = newRoot->getAVLChild(0);  // Subárbol izquierdo que rota

        // Rotación
        newRoot->getChildRef(0) = pNode;    // pNode se convierte en hijo izquierdo
        pNode->getChildRef(1) = temp;      // temp se convierte en hijo derecho

        // Actualizar padres
        if (temp) {
            typename Base::Node* tempBase = static_cast<typename Base::Node*>(temp);
            tempBase->m_pParent = static_cast<typename Base::Node*>(pNode);
        }
        static_cast<typename Base::Node*>(pNode)->m_pParent = static_cast<typename Base::Node*>(newRoot);

        updateBalance(pNode);
        updateBalance(newRoot);
        return newRoot; 
    
    }

public:
    CAVLTree() : Base() {} // Empty tree

};

#endif // __AVL_H__