#ifndef __BINARY_TREE_H__  
#define __BINARY_TREE_H__ 
#include <utility>
#include <algorithm>
#include <cassert>
#include <fstream>
#include "types.h"
//#include "util.h"
#include "general_iterator.h"
#include <vector>
#include <functional>
using namespace std;

template <typename Traits>
class CBinaryTree;

template <typename Traits>
class CBinaryTreeNode{
public:
  using value_type = typename Traits::T;
  using Node       = CBinaryTreeNode<Traits>;

  friend class CBinaryTree<Traits>;

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
    ~CBinaryTreeNode(){
        m_pChild[0] = nullptr;
        m_pChild[1] = nullptr;
    }

    value_type  getData()                {   return m_data;    }
    value_type &getDataRef()             {   return m_data;    }
    Ref     getRef()                     {   return m_ref;     }
    Node*  getParentNode()                {   return m_pParent; }

    Node* getpNext(bool forward = true) {
        Node* current = this;
        if (current->m_pChild[forward]) 
        {
            current = current->m_pChild[forward];
            while (current->m_pChild[!forward])
                current = current->m_pChild[!forward];
            return current;
        }

        Node* parent = current->m_pParent;
        while (parent && current == parent->m_pChild[forward])
        {
            current = parent;
            parent = parent->m_pParent;
        }
        return parent;
    }

private: // TODO: Add this class as friend of the BinaryTree
        // and make these methods private
    void      setChild(const Node *pChild, size_t pos)  {   m_pChild[pos] = pChild;  }
    Node    * getChild(size_t branch){ return m_pChild[branch];  }
    Node    *&getChildRef(size_t branch){ return m_pChild[branch];  }
    Node    * getParent() { return m_pParent;   }
};

template <typename Container>
class binary_tree_iterator : public general_iterator<Container,  class binary_tree_iterator<Container> > // 
{  
public:
    using Parent    = class general_iterator<Container, binary_tree_iterator<Container> >;     \
    using Node      = typename Container::Node;

  public:
    binary_tree_iterator(Container *pContainer, Node *pNode) : Parent (pContainer,pNode) {}
    binary_tree_iterator(Container &other)  : Parent (other) {}
    binary_tree_iterator(Container &&other) : Parent(other) {} // Move constructor C++11 en adelante

public:
    binary_tree_iterator operator++() {
        Parent::m_pNode = Parent::m_pNode ? (Node*)Parent::m_pNode->getpNext() : nullptr;
        return *this;
    }
};


template <typename Container>
class rbinary_tree_iterator : public general_iterator<Container,  class rbinary_tree_iterator<Container> > // 
{  
public:
    using Parent    = class general_iterator<Container, rbinary_tree_iterator<Container> >;     \
    using Node      = typename Container::Node;

  public:
    rbinary_tree_iterator(Container *pContainer, Node *pNode) : Parent (pContainer,pNode) {}
    rbinary_tree_iterator(Container &other)  : Parent (other) {}
    rbinary_tree_iterator(Container &&other) : Parent(other) {} // Move constructor C++11 en adelante

public:
    rbinary_tree_iterator operator++() {
        Parent::m_pNode = Parent::m_pNode ? (Node*)Parent::m_pNode->getpNext(false) : nullptr;
        return *this;
    }
};

template <typename _T>
struct BinaryTreeAscTraits{
    using  T         = _T;
    using  Node      = CBinaryTreeNode<BinaryTreeAscTraits<_T>>;
    using  CompareFn = less<T>;
};

template <typename _T>
struct BinaryTreeDescTraits
{
    using  T         = _T;
    using  Node      = CBinaryTreeNode<BinaryTreeDescTraits<_T>>;
    using  CompareFn = greater<T>;
};

template <typename Traits>
class CBinaryTree{
public:
    using value_type    = typename Traits::T;
    using Node          = typename Traits::Node;
    
    using CompareFn     = typename Traits::CompareFn;
    using Container     = CBinaryTree<Traits>;
    using iterator      = binary_tree_iterator<Container>;
    using riterator     = rbinary_tree_iterator<Container>;

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

    Node* CopyNode(Node* node, Node* parent) {
        if (!node) return nullptr;
        Node* newNode = CreateNode(parent, node->m_data, node->m_ref);
        newNode->m_pChild[0] = CopyNode(node->m_pChild[0], newNode);
        newNode->m_pChild[1] = CopyNode(node->m_pChild[1], newNode);
        return newNode;
    }

    void DestructorNode(Node* node) {
        if (!node) return;
        DestructorNode(node->m_pChild[0]);
        DestructorNode(node->m_pChild[1]);
        delete node;
    }

public:
    CBinaryTree(){} // Empty tree
    
    // TODO: Copy Constructor. We have duplicate each node
    CBinaryTree(CBinaryTree &other){
        m_pRoot = CopyNode(other.m_pRoot, nullptr);
        m_size  = other.m_size;
        Compfn  = other.Compfn;
    }
    
    // TODO: Done: Move Constructor
    CBinaryTree(CBinaryTree &&other)
        : m_pRoot(std::exchange(other.m_pRoot, nullptr)), 
          m_size (std::exchange(other.m_size, 0)), 
          Compfn (std::exchange(other.Compfn, nullptr))
    { }

    // TODO: Recursivo y seguro. Destruir Nodes recursivamente
    virtual ~CBinaryTree(){ 
        DestructorNode(m_pRoot); 
        m_pRoot = nullptr; 
        m_size  = 0;
     } 
    
    // TODO: begin dede comenzar el el nodo mas a la izquierda (0)
    iterator begin() { 
        if (!m_pRoot) return end();
        return iterator(this, getExtremeNode(m_pRoot, 0));
    }
    iterator end()   { return iterator(this, nullptr); }

    // TODO: begin debe comenzar el el nodo mas a la derecha (1)
    riterator rbegin(){ 
         if (!m_pRoot) return rend();
        return riterator(this, getExtremeNode(m_pRoot, 1));
    }
    riterator rend()  { return riterator(this, nullptr); }

    // TODO: Generalizar estos recorridos para recibir cualquier funcion
    // con una cantidad flexible de parametros con variadic templates
    // Google: C++ parameter packs cplusplus
    template <typename Function, typename... Args>
    void inorder(Function&& func, Args&&... args) {
        inorder_impl(m_pRoot, std::forward<Function>(func), std::forward<Args>(args)...);
    }
    // TODO: 
    template <typename Function, typename... Args>
    void inorder_impl(Node* pNode, Function&& func, Args&&... args) {
        if (!pNode) return;
        inorder_impl(pNode->getChild(0), std::forward<Function>(func), std::forward<Args>(args)...);
        std::invoke(std::forward<Function>(func), pNode, std::forward<Args>(args)...);
        inorder_impl(pNode->getChild(1), std::forward<Function>(func), std::forward<Args>(args)...);
    }

    void inorder(Node  *pNode, size_t level, ostream &os){
        if( pNode ){
            //Node *pParent = pNode->getParent();
            inorder(pNode->getChild(0), level+1, os);
            os << " --> " << pNode->getDataRef();
            inorder(pNode->getChild(1), level+1, os);
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
    void postorder(Function&& func, Args&&... args)
    {    postorder_impl(m_pRoot, 0, std::forward<Function>(func), std::forward<Args>(args)...);}

    template <typename Function,typename... Args>
    void postorder_impl(Node* pNode, Function&& func, Args&&... args) {
        if (pNode) {
            postorder_impl(pNode->getChild(0), std::forward<Function>(func), std::forward<Args>(args)...);
            postorder_impl(pNode->getChild(1), std::forward<Function>(func), std::forward<Args>(args)...);
            std::invoke(std::forward<Function>(func), pNode, std::forward<Args>(args)...);
        }
    }

    // TODO: generalize this function to apply any function
    void postorder(Node  *pNode, size_t level, ostream &os){
        if( pNode ){   
            postorder(pNode->getChild(0), level+1, os);
            postorder(pNode->getChild(1), level+1, os);
            os << " --> " << pNode->getDataRef();
        }
    }

    // TODO: Generalize this function to apply any function
    template <typename Function, typename... Args>
    void preorder(Function&& func, Args&&... args) {
        preorder_impl(m_pRoot, 0, std::forward<Function>(func), std::forward<Args>(args)...);
    }

    template <typename Function, typename... Args>
    void preorder_impl(Node* pNode, Function&& func, Args&&... args) {
        if (!pNode) return;
        std::invoke(std::forward<Function>(func), pNode, std::forward<Args>(args)...);
        preorder_impl(pNode->getChild(0), std::forward<Function>(func), std::forward<Args>(args)...);
        preorder_impl(pNode->getChild(1), std::forward<Function>(func), std::forward<Args>(args)...);
    }

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
    // Google: C++ parameter packs cplusplus
    void print(Node  *pNode, size_t level, ostream &os){
        if( pNode ){
            Node *pParent = pNode->getParent();
            print(pNode->getChild(1), level+1, os);
            for(size_t i = 0; i < level; ++i){
				os << string(" | ");
			}
            os << pNode->getDataRef() << "(" << (pParent?to_string(pParent->getData()):"Root") << ")" <<endl;
            print(pNode->getChild(0), level+1, os);
        }
    }

    template <typename F, typename... Params>
    void print_generic(F&& action, Params&&... params) {
        print_node(m_pRoot, 0, std::forward<F>(action), std::forward<Params>(params)...);
    }

    template <typename F, typename... Params>
    void print_node(Node* node, std::size_t depth, F&& action, Params&&... params) {
        if (!node) return;

        print_node(node->getChild(1), depth + 1, std::forward<F>(action), std::forward<Params>(params)...);

        std::invoke(std::forward<F>(action), node, depth, std::forward<Params>(params)...);

        print_node(node->getChild(0), depth + 1, std::forward<F>(action), std::forward<Params>(params)...);
    }

    // TODO: Tip: recorrer el arbol en preorden
    void Write(ostream &os) { 
        WriteRecursive(m_pRoot, os);
     }

    void WriteRecursive(Node* node, std::ostream &os) {
        if (!node) {
            os << "@ ";
            return;
        }
        os << node->getData() << " " << node->getRef() << " ";
        WriteRecursive(node->getChild(0), os);
        WriteRecursive(node->getChild(1), os);
    }

    // TODO: Leer en el arbol desde un stream asumiendo que esta en preorden
    void Read(istream &is)  { 
        DestructorNode(m_pRoot);
        m_size  = 0;
        m_pRoot = ReadRecursive(nullptr, is);
    }

    Node* ReadRecursive(Node* pParent, std::istream &is) {
        std::string token;
        if (!(is >> token)) return nullptr;
        if (token == "@") return nullptr;

        value_type value = static_cast<value_type>(std::stoi(token)); // convierte valor
        Ref ref;
        is >> ref;

        Node* node = CreateNode(pParent, value, ref);
        node->m_pChild[0] = ReadRecursive(node, is);
        node->m_pChild[1] = ReadRecursive(node, is);
        ++m_size;
        return node;
    }
};

// TODO: este operator << debe seguir estando fuera de la clase
template <typename Traits>
std::ostream& operator<<(std::ostream& os, CBinaryTree<Traits>& obj) {
    os << "CBinaryTree with " << obj.size() << " elements.";
    obj.inorder([&os](typename CBinaryTree<Traits>::Node* pNode) {
        os << " --> " << pNode->getData();
    });
    return os;
}

template <typename Traits>
istream & operator>>(istream &is, CBinaryTree<Traits> &obj){
    // Leer el arbol
    return is;
}

void DemoBinaryTree();

#endif // __BINARY_TREE_H__