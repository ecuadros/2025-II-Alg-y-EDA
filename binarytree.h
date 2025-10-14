#ifndef __BINARY_TREE_H__  
#define __BINARY_TREE_H__ 
#include <utility>
#include <algorithm>
#include <cassert>
#include <fstream>
#include "types.h"
#include "util.h"
using namespace std;

template <typename Container> 
class binary_tree_forward_iterator;

template <typename Container> 
class binary_tree_backward_iterator;

template <typename Traits>
class CBinaryTree;

template <typename Traits>
class CBinaryTreeNode{
public:
  using value_type = typename Traits::T;
  using Node       = CBinaryTreeNode<Traits>;

protected:
    value_type     m_data;
    Node          *m_pParent = nullptr;
    Ref            m_ref;
    vector<Node *> m_pChild  = {nullptr, nullptr}; // 2 hijos inicializados en nullptr

public:
    CBinaryTreeNode(Node* pParent, value_type data, Ref ref, Node* p0 = nullptr, Node* p1 = nullptr)
        : m_data(data), m_pParent(pParent), m_ref(ref)
    {
        m_pChild[0] = p0;
        m_pChild[1] = p1;
    }
    /*~CBinaryTreeNode(){
        delete m_pChild[0]; m_pChild[0] = nullptr;
        delete m_pChild[1]; m_pChild[1] = nullptr;
    }*/

    value_type  getData()                {   return m_data;    }
    value_type &getDataRef()             {   return m_data;    }
 
protected: // TODO: Add this class as friend of the BinaryTree
        // and make these methods private
    void      setpChild(const Node *pChild, size_t pos)  {   m_pChild[pos] = pChild;  }
    Node    * getChild(size_t branch){ return m_pChild[branch];  }
    Node    *&getChildRef(size_t branch){ return m_pChild[branch];  }
    Node    * getParent() { return m_pParent;   }

public:
    Node* getNext() {
        if (m_pChild[1]) {
            Node* p = m_pChild[1];
            while (p->m_pChild[0])
                p = p->m_pChild[0];
            return p;
        }
        Node* p = m_pParent;
        Node* c = this;
        while (p && c == p->m_pChild[1]) {
            c = p;
            p = p->m_pParent;
        }
        return p;
    }   

    Node* getPrev() {
        if (m_pChild[0]) {
            Node* p = m_pChild[0];
            while (p->m_pChild[1])
                p = p->m_pChild[1];
            return p;
        }
        Node* p = m_pParent;
        Node* c = this;
        while (p && c == p->m_pChild[0]) {
            c = p;
            p = p->m_pParent;
        }
        return p;
    }

    friend class CBinaryTree<Traits>;
    template <typename Container> 
    friend class binary_tree_forward_iterator;

    template <typename Container> 
    friend class binary_tree_backward_iterator;

};

template <typename Container, typename Iterator>
class general_iterator {
protected:
    using Node = typename Container::Node;
    using value_type = typename Container::value_type;

    Container* m_pContainer;
    Node* m_pNode;

public:
    general_iterator(Container* pContainer = nullptr, Node* pNode = nullptr)
        : m_pContainer(pContainer), m_pNode(pNode) {}

    value_type& operator*() const { return m_pNode->getDataRef(); }

    bool operator==(const Iterator& other) const { return m_pNode == other.m_pNode; }
    bool operator!=(const Iterator& other) const { return !(*this == other); }

};

template <typename Container>
class binary_tree_backward_iterator :  public general_iterator<Container,  class binary_tree_backward_iterator<Container> >
{  
public:
    using Parent    = class general_iterator<Container, binary_tree_backward_iterator<Container> >;     
    using Node      = typename Container::Node;
    using iterator = binary_tree_backward_iterator<Container>;

public:
    binary_tree_backward_iterator(Container *pContainer, Node *pNode) : Parent (pContainer,pNode) {}
    binary_tree_backward_iterator(const iterator& other)
        : Parent(other.m_pContainer, other.m_pNode) {}
    //binary_tree_backward_iterator(iterator &&other) : Parent(other) {} // Move constructor C++11 en adelante

public:
    // TODO: Revisar el avance de un iterator
    iterator& operator++() {
        this->m_pNode = this->m_pNode ? this->m_pNode->getPrev() : nullptr;
        return *this;
    }

};

template <typename Container>
class binary_tree_forward_iterator 
    : public general_iterator<Container, binary_tree_forward_iterator<Container>> 
{
public:
    using Parent    = general_iterator<Container, binary_tree_forward_iterator<Container>>;
    using Node      = typename Container::Node;
    using iterator  = binary_tree_forward_iterator<Container>;

public:
    binary_tree_forward_iterator(Container *pContainer, Node *pNode) 
        : Parent(pContainer, pNode) {}

    binary_tree_forward_iterator(const iterator& other)
        : Parent(other.m_pContainer, other.m_pNode) {}

    iterator& operator++() {
        this->m_pNode = this->m_pNode ? (Node*)this->m_pNode->getNext() : nullptr;
        return *this;
    }
};


template <typename _T>
struct BinaryTreeAscTraits{
    using  T         = _T;
    using  Node      = CBinaryTreeNode<T>;
    using  CompareFn = less<T>;
};

template <typename _T>
struct BinaryTreeDescTraits
{
    using  T         = _T;
    using  Node      = CBinaryTreeNode<T>;
    using  CompareFn = greater<T>;
};

template <typename Traits>
class CBinaryTree{
public:
    using Node          = CBinaryTreeNode<Traits>;
    using value_type    = typename Traits::T;
    
    using CompareFn     = typename Traits::CompareFn;
    using Binary     = CBinaryTree<Traits>;
    using backward_iterator      = binary_tree_backward_iterator<Binary>;
    using forward_iterator      = binary_tree_forward_iterator<Binary>;

protected:
    Node    *m_pRoot = nullptr;
    size_t   m_size  = 0;
    CompareFn Compfn;
public: 
    size_t  size()  const       { return m_size;       }
    bool    empty() const       { return size() == 0;  }

    void insert(value_type elem, Ref ref) {
        internal_insert(elem, ref, nullptr, m_pRoot);
    }

    Node* getExtremeNode(Node* startNode, int direction) const {
        if (!startNode) return nullptr;
        
        Node* pNode = startNode;
        while (pNode->getChild(direction)) {
            pNode = pNode->getChild(direction);
        }
        return pNode;
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
public:
    CBinaryTree(){} // Empty tree
    
    // TODO: Copy Constructor. We have duplicate each node
    CBinaryTree(Binary &other);
    
    // TODO: Done: Move Constructor
    CBinaryTree(Binary &&other)
        : m_pRoot(std::exchange(other.m_pRoot, nullptr)), 
          m_size (std::exchange(other.m_size, 0)), 
          Compfn (std::exchange(other.Compfn, nullptr))
    { }

    // TODO: Recursivo y seguro. Destruir Nodes recursivamente
    virtual ~CBinaryTree(){  
        Destroy(m_pRoot);
        m_pRoot = nullptr;
        m_size  = 0;
    } 
    
    // TODO: begin dede comenzar el el nodo mas a la izquierda (0)
    backward_iterator rbegin() { 
        if (!m_pRoot) return rend();
        return backward_iterator(this, getExtremeNode(m_pRoot, 1));
    }
    backward_iterator rend()   { return backward_iterator(this, nullptr); }

    forward_iterator fbegin() {
        if (!m_pRoot) return fend();
        return forward_iterator(this, getExtremeNode(m_pRoot, 0));
    }

    forward_iterator fend() {
        return forward_iterator(this, nullptr);
    }

    // TODO: begin debe comenzar el el nodo mas a la derecha (1)
    // riterator rbegin(){ 
    //     if (!m_pRoot) return rend();
    //     return iterator(this, getExtremeNode(m_pRoot, 1));
    //  }
    // riterator rend()  { return iterator(this, nullptr); }

    // TODO: Generalizar estos recorridos para recibir cualquier funcion
    // con una cantidad flexible de parametros con variadic templates
    // Google: C++ parameter packs cplusplus

    template <typename Function, typename... Args>
    void inorder(Function func, Args const&... args) {
        inorder(m_pRoot, func, args...);
    }

    template <typename Function, typename... Args>
    void inorder(Node* pNode, Function func, Args const&... args) {
        if (pNode) {
            inorder(pNode->getChild(0), func, args...);
            func(pNode, args...);
            inorder(pNode->getChild(1), func, args...);
        }
}

    void inorder  (ostream &os){   inorder_print(m_pRoot, 0, os);  }
    // TODO: 
    void inorder_print(Node  *pNode, size_t level, ostream &os){
        if( pNode ){
            //Node *pParent = pNode->getParent();
            inorder_print(pNode->getChild(0), level+1, os);
            os << " --> " << pNode->getDataRef();
            //cout << "\nVisitando nodo: " << pNode->getDataRef() << endl;
            inorder_print(pNode->getChild(1), level+1, os);
        }
    }

    // TODO: Generalize this function by using iterators and apply any function
    void inorder(Node  *pNode, void (*visit) (value_type& item)){
        if( pNode ){   
            inorder(pNode->getChild(0), *visit);
            (*visit)(pNode->getDataRef());
            inorder(pNode->getChild(1), *visit);
        }
    }

    // Variadic templates (See foreach.h)
    template <typename Function, typename... Args>
    void postorder(Function func, Args const&... args)
    {    postorder(m_pRoot, 0, func, args...);}

    template <typename Function,typename... Args>
    void postorder(Node* pNode, size_t level, 
                   Function func, Args const&... args) {
        if (pNode) {
            postorder(pNode->getChild(0), level + 1, func, args...);
            postorder(pNode->getChild(1), level + 1, func, args...);
            func(pNode, level); 
        }
    }
    // TODO: generalize this function to apply any function
    void postorder (ostream &os)    {   postorder_print (m_pRoot, 0, os);  }

    void postorder_print(Node  *pNode, size_t level, ostream &os){
        if( pNode ){   
            postorder_print(pNode->getChild(0), level+1, os);
            postorder_print(pNode->getChild(1), level+1, os);
            os << " --> " << pNode->getDataRef();
        }
    }

    // TODO: Generalize this function to apply any function
    template <typename Function, typename... Args>
    void preorder(Function func, Args const&... args)
    {    
        preorder(m_pRoot, 0, func, args...);
    }  

    template <typename Function,typename... Args>
    void preorder(Node* pNode, size_t level, Function func, Args const&... args) {
        if (pNode) {   
            func(pNode, level);
            preorder(pNode->getChild(0), level+1, func, args...);
            preorder(pNode->getChild(1), level+1, func, args...);            
        }
    }

    void preorder (ostream &os)    {   preorder_print (m_pRoot, 0, os);  }
    // TODO: Generalize this function to apply any function
    void preorder_print(Node  *pNode, size_t level, ostream &os){
        if( pNode ){   
            os << " --> " << pNode->getDataRef();
            preorder_print(pNode->getChild(0), level+1, os);
            preorder_print(pNode->getChild(1), level+1, os);            
        }
    }

    void print    (ostream &os)    {   print    (m_pRoot, 0, os);  }
    // TODO: generalize this function to apply any function
    // Google: C++ parameter packs cplusplus
    void print(Node  *pNode, size_t level, ostream &os){
        if( pNode ){
            Node *pParent = pNode->getParent();
            print(pNode->getChild(1), level+1, os);
            os << string(level*5, ' ') << " " << pNode->getDataRef() << "(" << (pParent?to_string(pParent->getData()):"Root") << ")" <<endl;
            print(pNode->getChild(0), level+1, os);
        }
    }

    // TODO: Tip: recorrer el arbol en preorden
    void Write(ostream &os) { os << *this;  }

    // TODO: Leer en el arbol desde un stream asumiendo que esta en preorden
    void Read(istream &is)  { /* TODO */  }

private:
    void Destroy(Node* node) {
        if (node) {
            Destroy(node->getChild(0));
            Destroy(node->getChild(1));
            delete node;
        }
    }
};

// TODO: este operator << debe seguir estando fuera de la clase
template <typename Traits>
ostream & operator<<(std::ostream &os, CBinaryTree<Traits> &obj){
    os << "CBinaryTree with " << obj.size() << " elements.";
    obj.inorder(os);
    return os;
}

template <typename Traits>
istream & operator>>(istream &is, CBinaryTree<Traits> &obj){
    // Leer el arbol
    return is;
}

void DemoBinaryTree();

#endif // __BINARY_TREE_H__