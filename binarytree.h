#ifndef __BINARY_TREE_H__  
#define __BINARY_TREE_H__ 

#include <cassert>
#include <fstream>
#include <vector>
#include <iostream>
#include <functional>
#include <type_traits>
#include "types.h"
using namespace std;

template <typename Traits>
class CBinaryTreeNode{
public:
    using value_type = typename Traits::T;
    using Node       = CBinaryTreeNode<Traits>;

protected:
    value_type  m_data;
    Node*       m_pParent = nullptr;
    Ref         m_ref;
    vector<Node*> m_pChild = {nullptr, nullptr}; // 2 hijos inicializados en nullptr

public:
    CBinaryTreeNode(Node* pParent, value_type data, Ref ref, Node* p0 = nullptr, Node* p1 = nullptr)
        : m_pParent(pParent), m_data(data), m_ref(ref)
    {
        m_pChild[0] = p0;
        m_pChild[1] = p1;
    }

    ~CBinaryTreeNode(){
        delete m_pChild[0];
        delete m_pChild[1];
        m_pChild[0] = m_pChild[1] = nullptr;
    }

    // TODO: Keynode 
    value_type   getData()      { return m_data; }
    value_type&  getDataRef()   { return m_data; }
    Ref          getRef()       { return m_ref;  }

public: // TODO: Add this class as friend of the BinaryTree
        // and make these methods private
    void      setpChild(Node* pChild, size_t pos)   { m_pChild[pos] = pChild; }
    Node*     getChild(size_t branch)               { return m_pChild[branch]; }
    Node*&    getChildRef(size_t branch)            { return m_pChild[branch]; }
    Node*     getParent()                           { return m_pParent; }
};

// ===========================================================
// ITERATOR
// ===========================================================
template <typename Container>
class binary_tree_iterator {
public:
    using Node       = typename Container::Node;
    using value_type = typename Container::value_type;

private:
    Container* m_pContainer = nullptr;
    Node*      m_pNode = nullptr;

public:
    binary_tree_iterator(Container* pContainer, Node* pNode)
        : m_pContainer(pContainer), m_pNode(pNode) {}
    binary_tree_iterator(const binary_tree_iterator& other)
        : m_pContainer(other.m_pContainer), m_pNode(other.m_pNode) {}

    bool operator==(const binary_tree_iterator& other) const {
        return m_pNode == other.m_pNode;
    }
    bool operator!=(const binary_tree_iterator& other) const {
        return !(*this == other);
    }

    // TODO: Revisar el avance de un iterator
    binary_tree_iterator operator++() {
        if (m_pNode)
            m_pNode = m_pNode->getChild(1); // Avanza hacia el hijo derecho
        return *this;
    }

    value_type& operator*() { return m_pNode->getDataRef(); }
};

// ===========================================================
// TRAITS
// ===========================================================
template <typename _T>
struct BinaryTreeAscTraits{
    using  T         = _T;
    using  Node      = CBinaryTreeNode<BinaryTreeAscTraits<T>>;
    using  CompareFn = less<T>;
};

template <typename _T>
struct BinaryTreeDescTraits{
    using  T         = _T;
    using  Node      = CBinaryTreeNode<BinaryTreeDescTraits<T>>;
    using  CompareFn = greater<T>;
};

// ===========================================================
// CLASS CBinaryTree
// ===========================================================
template <typename Traits>
class CBinaryTree{
public:
    using value_type    = typename Traits::T;
    using Node          = typename Traits::Node;
    using CompareFn     = typename Traits::CompareFn;
    using Container     = CBinaryTree<Traits>;
    using iterator      = binary_tree_iterator<Container>;

protected:
    Node*    m_pRoot = nullptr;
    size_t   m_size  = 0;
    CompareFn Compfn;

public:
    size_t  size()  const { return m_size; }
    bool    empty() const { return size() == 0; }

    // TODO: insert must receive two parameters: elem and Ref value
    void insert(value_type elem, Ref ref) {
        m_pRoot = internal_insert(elem, ref, nullptr, m_pRoot);
    }

protected:
    Node* CreateNode(Node* pParent, value_type elem, Ref ref) {
        return new Node(pParent, elem, ref);
    }

    virtual Node* internal_insert(value_type &elem, Ref ref,
                                  Node* pParent, Node*& rpOrigin)
    {
        if (!rpOrigin) {
            ++m_size;
            return (rpOrigin = CreateNode(pParent, elem, ref));
        }

        size_t branch = Compfn(elem, rpOrigin->getDataRef()) ? 0 : 1;
        Node *pNode = internal_insert(elem, ref, rpOrigin, rpOrigin->getChildRef(branch));
        return pNode;
    }

    // Función auxiliar para el constructor de copia
    Node* clone_subtree(Node* pNode, Node* pParent){
        if(!pNode) return nullptr;
        Node* newNode = new Node(pParent, pNode->getDataRef(), pNode->getRef());
        newNode->getChildRef(0) = clone_subtree(pNode->getChild(0), newNode);
        newNode->getChildRef(1) = clone_subtree(pNode->getChild(1), newNode);
        return newNode;
    }

public:
    CBinaryTree(){} // Empty tree

    // TODO: Copy Constructor. We have duplicate each node
    CBinaryTree(CBinaryTree &other){
        m_pRoot = clone_subtree(other.m_pRoot, nullptr);
        m_size  = other.m_size;
        Compfn  = other.Compfn;
    }

    // Move Constructor
    CBinaryTree(CBinaryTree &&other)
        : m_pRoot(std::exchange(other.m_pRoot, nullptr)), 
          m_size (std::exchange(other.m_size, 0)), 
          Compfn (std::move(other.Compfn))
    { }

    // TODO: Recursivo y seguro. Destruir Nodes recursivamente
    virtual ~CBinaryTree(){  
        delete m_pRoot;
        m_pRoot = nullptr;
        m_size = 0;
    } 
    
    // ===========================================================
    // TODO: Generalizar estos recorridos para recibir cualquier funcion
    // ===========================================================
    void inorder  (ostream &os)    {   inorder  (m_pRoot, 0, os);  }

    void inorder(Node  *pNode, size_t level, ostream &os){
        if( pNode ){
            inorder(pNode->getChild(0), level+1, os);
            os << " --> " << pNode->getDataRef();
            inorder(pNode->getChild(1), level+1, os);
        }
    }

    // TODO: Generalize this function by using iterators and apply any function
    void inorder(Node  *pNode, void (*visit) (value_type& item)){
        if( pNode ){   
            inorder(pNode->getChild(0), visit);
            (*visit)(pNode->getDataRef());
            inorder(pNode->getChild(1), visit);
        }
    }

   

    void postorder(ostream &os) { postorder(m_pRoot, 0, os); }

    // TODO: generalize this function to apply any function
    void postorder(Node  *pNode, size_t level, ostream &os){
        if( pNode ){   
            postorder(pNode->getChild(0), level+1, os);
            postorder(pNode->getChild(1), level+1, os);
            os << " --> " << pNode->getDataRef();
        }
    }

    // TODO: Generalize this function to apply any function
    void preorder (ostream &os)    {   preorder (m_pRoot, 0, os);  }

    // TODO: Generalize this function to apply any function
    void preorder(Node  *pNode, size_t level, ostream &os){
        if( pNode ){   
            os << " --> " << pNode->getDataRef();
            preorder(pNode->getChild(0), level+1, os);
            preorder(pNode->getChild(1), level+1, os);            
        }
    }

    void print    (ostream &os)    {   print    (m_pRoot, 0, os);  }

    // TODO: generalize this function to apply any function
    void print(Node  *pNode, size_t level, ostream &os){
        if( pNode ){
            Node *pParent = pNode->getParent();
            print(pNode->getChild(1), level+1, os);
            os << string(level * 3, ' ') 
               << pNode->getDataRef() << "(" 
               << (pParent ? to_string(pParent->getData()) : "Root") 
               << ")" << endl;
            print(pNode->getChild(0), level+1, os);
        }
    }

    // TODO: Tip: recorrer el arbol en preorden
    void Write(ostream &os) { os << *this;  }

    // TODO: Leer en el arbol desde un stream asumiendo que esta en preorden
    void Read(istream &is)  { 
        value_type val;
        while (is >> val) {
            Ref r = 0;
            insert(val, r);
        }
    }
};

// ===========================================================
// TODO: este operator << debe seguir estando fuera de la clase
// ===========================================================
template <typename Traits>
ostream & operator<<(std::ostream &os, CBinaryTree<Traits> &obj){
    os << "CBinaryTree with " << obj.size() << " elements.";
    obj.inorder(os);
    return os;
}

template <typename Traits>
istream & operator>>(istream &is, CBinaryTree<Traits> &obj){
    obj.Read(is);
    return is;
}

void DemoBinaryTree();

#endif // __BINARY_TREE_H__