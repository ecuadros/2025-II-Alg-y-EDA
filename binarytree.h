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

    




protected: // TODO Hecho : Add this class as friend of the BinaryTree
        // and make these methods private

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
        m_pRoot = internal_insert(elem, ref, nullptr, m_pRoot);
    }

    // inorder 
    template <typename F, typename... Args>
    void inorder(F&& f, Args&&... args) {
        inorder_impl(m_pRoot, 0, std::forward<F>(f), std::forward<Args>(args)...);
    }

    //Preorder 
    template <typename F, typename... Args>
    void preorder(F&& f, Args&&... args) {
        preorder_impl(m_pRoot, 0, std::forward<F>(f), std::forward<Args>(args)...);
    }

    //postorder
    template <typename F, typename... Args>
    void postorder(F&& f, Args&&... args) {
        postorder_impl(m_pRoot, 0, std::forward<F>(f), std::forward<Args>(args)...);
    }

    void inorder_print(std::ostream& os)  { inorder([&](value_type& x){ os << " --> " << x; }); }
    void preorder_print(std::ostream& os) { preorder([&](value_type& x){ os << " --> " << x; }); }
    void postorder_print(std::ostream& os){ postorder([&](value_type& x){ os << " --> " << x; }); }




protected:
    Node* CreateNode(Node* pParent,const value_type elem, Ref ref) {
        return new Node(pParent, elem, ref);
    }
    virtual Node* internal_insert(const value_type &elem, Ref ref,
                                  Node* pParent, Node*& rpOrigin)
    {
        if (!rpOrigin) {
            ++m_size;
            return (rpOrigin = CreateNode(pParent, elem, ref));
        }

        const bool goLeft = Compfn(elem, rpOrigin->getDataRef());
        Node*& next = rpOrigin->getChildRef(goLeft ? 0 : 1);
        return internal_insert(elem, ref, rpOrigin, next);
    }

    // Llamador generico que adapta la llamada al visitor
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


    // Inorden: IZ, Nodo, DR
    template <typename F, typename... Args>
    static void inorder_impl(Node* n, size_t level, F&& f, Args&&... args) {
        if (!n) return;
        inorder_impl(n->getChild(0), level + 1, std::forward<F>(f), std::forward<Args>(args)...);
        call_visit(std::forward<F>(f), n, level, std::forward<Args>(args)...);
        inorder_impl(n->getChild(1), level + 1, std::forward<F>(f), std::forward<Args>(args)...);
    }

    // Preorden: Nodo, IZ, DR
    template <typename F, typename... Args>
    static void preorder_impl(Node* n, size_t level, F&& f, Args&&... args) {
        if (!n) return;
        call_visit(std::forward<F>(f), n, level, std::forward<Args>(args)...);
        preorder_impl(n->getChild(0), level + 1, std::forward<F>(f), std::forward<Args>(args)...);
        preorder_impl(n->getChild(1), level + 1, std::forward<F>(f), std::forward<Args>(args)...);
    }

    // Postorden: IZ, DR, Nodo
    template <typename F, typename... Args>
    static void postorder_impl(Node* n, size_t level, F&& f, Args&&... args) {
        if (!n) return;
        postorder_impl(n->getChild(0), level + 1, std::forward<F>(f), std::forward<Args>(args)...);
        postorder_impl(n->getChild(1), level + 1, std::forward<F>(f), std::forward<Args>(args)...);
        call_visit(std::forward<F>(f), n, level, std::forward<Args>(args)...);
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