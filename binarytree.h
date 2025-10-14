#ifndef __BINARY_TREE_H__  
#define __BINARY_TREE_H__ 
//#include <utility>
//#include <algorithm>
#include <cassert>
#include <functional>
#include <fstream>
#include <utility>
#include <ostream>
#include <type_traits>
#include <utility> 
#include "types.h"
//#include "util.h"
using namespace std;

template <typename Traits>
class CBinaryTreeNode{
public:
  using value_type = typename Traits::T;
  using Node       = CBinaryTreeNode<Traits>;

protected:
    value_type       m_data{};
    Node *  m_pParent = nullptr;
    Ref     m_ref{};
    vector<Node *> m_pChild = {nullptr, nullptr}; // 2 hijos inicializados en nullptr 

public:
    CBinaryTreeNode(Node* pParent, value_type data, Ref ref, Node* p0 = nullptr, Node* p1 = nullptr)
        : m_pParent(pParent), m_data(data), m_ref(ref)
    {
        m_pChild[0] = p0;
        m_pChild[1] = p1;
    }
    ~CBinaryTreeNode(){
        delete m_pChild[0]; m_pChild[0] = nullptr;
        delete m_pChild[1]; m_pChild[1] = nullptr;
    }

    // TODO: Keynode 
    value_type        getData()           const{   return m_data;    }
    value_type       &getDataRef()             {   return m_data;    }
    const value_type &getDataRef()        const{   return m_data;    }

    // preorder 
    template <typename F, typename... Args>
    void preorder(F&& f, Args&&... args) {
        preorder_impl(m_pRoot, 0, std::forward<F>(f), std::forward<Args>(args)...);
    }

    //postorder
    template <typename F, typename... Args>
    void postorder(F&& f, Args&&... args) {
        postorder_impl(m_pRoot, 0, std::forward<F>(f), std::forward<Args>(args)...);
    }




protected: // TODO Hecho : Add this class as friend of the BinaryTree
        // and make these methods private

    template <typename F, typename... Args>
    static void call_visit(F&& f, Node* n, size_t level, Args&&... args) {
        if constexpr (std::is_invocable_v<F, Node*, size_t, Args...>) {
            std::invoke(std::forward<F>(f), n, level, std::forward<Args>(args)...);
        } else if constexpr (std::is_invocable_v<F, value_type&, size_t, Args...>) {
            std::invoke(std::forward<F>(f), n->getDataRef(), level, std::forward<Args>(args)...);
        } else if constexpr (std::is_invocable_v<F, value_type&, Args...>) {
            std::invoke(std::forward<F>(f), n->getDataRef(), std::forward<Args>(args)...);
        } else if constexpr (std::is_invocable_v<F, Node*, Args...>) {
            std::invoke(std::forward<F>(f), n, std::forward<Args>(args)...);
        } else {
            static_assert([]{return false;}(), "Visitor con firma no soportada");
        }
    }

    // Implementación recursiva de preorden
    template <typename F, typename... Args>
    static void preorder_impl(Node* n, size_t level, F&& f, Args&&... args) {
        if (!n) return;
        call_visit(std::forward<F>(f), n, level, std::forward<Args>(args)...);
        preorder_impl(n->getChild(0), level + 1, std::forward<F>(f), std::forward<Args>(args)...);
        preorder_impl(n->getChild(1), level + 1, std::forward<F>(f), std::forward<Args>(args)...);
    }

    // Implementación recursiva de posorden
    template <typename F, typename... Args>
    static void postorder_impl(Node* n, size_t level, F&& f, Args&&... args) {
        if (!n) return;
        postorder_impl(n->getChild(0), level + 1, std::forward<F>(f), std::forward<Args>(args)...);
        postorder_impl(n->getChild(1), level + 1, std::forward<F>(f), std::forward<Args>(args)...);
        call_visit(std::forward<F>(f), n, level, std::forward<Args>(args)...);
    }

    void      setupChild( Node *pChild, size_t pos)  {   m_pChild[pos] = pChild;  }
    Node    * getChild(size_t branch){ return m_pChild[branch];  }
    Node    *&getChildRef(size_t branch){ return m_pChild[branch];  }
    Ref      getRef()   const{   return m_ref;     }
    Node    * getParent()    { return m_pParent;   }
};

template <typename Container>
class binary_tree_iterator : public general_iterator<Container,  class binary_tree_iterator<Container> > // 
{  
public:
    using Parent    = class general_iterator<Container, binary_tree_iterator<Container> >;     \
    using Node      = typename Container::Node;
    using Container = binary_tree_iterator<Container>;

  public:
    binary_tree_iterator(Container *pContainer, Node *pNode) : Parent (pContainer,pNode) {}
    binary_tree_iterator(Container &other)  : Parent (other) {}
    binary_tree_iterator(Container &&other) : Parent(other) {} // Move constructor C++11 en adelante

public:
    // TODO: Revisar el avance de un iterator
    binary_tree_iterator operator++() {
        Parent::m_pNode = Parent::m_pNode ? (Node*)Parent::m_pNode->getpNext() : nullptr;
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
    using value_type    = typename Traits::T;
    using Node          = typename Traits::Node;
    
    using CompareFn     = typename Traits::CompareFn;
    using Container     = CBinaryTree<Traits>;
    using iterator      = binary_tree_iterator<Container>;

protected:
    Node    *m_pRoot = nullptr;
    size_t   m_size  = 0;
    CompareFn Compfn{};
public: 
    CBinaryTree() = default;
    CBinaryTree(const CBinaryTree&) = delete; //disable copy
    CBinaryTree& operator=(const CBinaryTree&) = delete;   // disable copy assignment

    // Move Constructor
    CBinaryTree(CBinaryTree&& other) noexcept
        : m_pRoot(std::exchange(other.m_pRoot, nullptr))
        , m_size (std::exchange(other.m_size, 0))
        , Compfn(std::move(other.Compfn))
    {}

    // Move assignment
    CBinaryTree& operator=(CBinaryTree&& other) noexcept {
        if (this != &other) {
            clear(); // libera lo actual
            m_pRoot = std::exchange(other.m_pRoot, nullptr);
            m_size  = std::exchange(other.m_size, 0);
            Compfn  = std::move(other.Compfn);
        }
        return *this;
    }

    // Destructor SEGURO
    virtual ~CBinaryTree() noexcept { clear(); }

    // Limpieza  
    void clear() noexcept {
        delete m_pRoot;   // ~CBinaryTreeNode borra recursivamente toda la descendencia
        m_pRoot = nullptr;
        m_size  = 0;
    }
    

    size_t  size()  const       { return m_size;       }
    bool    empty() const       { return size() == 0;  }
    // TODO: insert must receive two paramaters: elem and Ref value
    void insert(value_type& elem, Ref ref) {
        m_pRoot = internal_insert(elem, ref, nullptr, nullptr, m_pRoot);
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
        Node *pNode = internal_insert(elem, ref, nullptr, rpOrigin, rpOrigin->getChildRef(branch));
        return pNode;
    }
public:
    CBinaryTree(){} // Empty tree
    
    // TODO: Copy Constructor. We have duplicate each node
    CBinaryTree(Binary &other);
    
    // Move Constructor
    CBinaryTree(Binary &&other)
        : m_pRoot(std::exchange(other.m_pRoot, nullptr)), 
          m_size (std::exchange(other.m_size, 0)), 
          Compfn (std::exchange(other.Compfn, nullptr))
    { }

    // TODO: Recursivo y seguro. Destruir Nodes recursivamente
    virtual ~CBinaryTree(){  } 
    
    // TODO: Generalizar estos recorridos para recibir cualquier funcion
    // con una cantidad flexible de parametros con variadic templates
    // Google: C++ parameter packs cplusplus
        void inorder  (ostream &os)    {   inorder  (m_pRoot, 0, os);  }
    // TODO: 
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
    // Google: C++ parameter packs cplusplus
    void print(Node  *pNode, size_t level, ostream &os){
        if( pNode ){
            Node *pParent = pNode->getParent();
            print(pNode->getChild(1), level+1, os);
            os << string(" | ") * level << pNode->getDataRef() << "(" << (pParent?to_string(pParent->getData()):"Root") << ")" <<endl;
            print(pNode->getChild(0), level+1, os);
        }
    }

    // TODO: Tip: recorrer el arbol en preorden
    void Write(ostream &os) { os << *this;  }

    // TODO: Leer en el arbol desde un stream asumiendo que esta en preorden
    void Read(istream &is)  { /* TODO */  }
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