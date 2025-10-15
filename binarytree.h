#ifndef __BINARY_TREE_H__  
#define __BINARY_TREE_H__ 
#include <utility>
//#include <algorithm>
#include <cassert>
#include <fstream>
#include "types.h"
#include <vector>
#include "general_iterator.h"
//#include "util.h"
#include "treeprinter.h"
using namespace std;

template <typename Traits>
class CBinaryTreeNode{
public:
  using T = typename Traits::T;
  using Node       = CBinaryTreeNode<Traits>;

protected:
    T     m_data;
    Node  *m_pParent = nullptr;
    Ref            m_ref;
    vector<Node *> m_pChild  = {nullptr, nullptr}; // 2 hijos inicializados en nullptr

public:
    CBinaryTreeNode(Node* pParent, T data, Ref ref, Node* p0 = nullptr, Node* p1 = nullptr)
        : m_pParent(pParent), m_data(data), m_ref(ref)
    {
        m_pChild[0] = p0;
        m_pChild[1] = p1;
    }
    ~CBinaryTreeNode(){
        delete m_pChild[0]; m_pChild[0] = nullptr;
        delete m_pChild[1]; m_pChild[1] = nullptr;
    }

    T getData()                    { return m_data;    }
    T &getDataRef()                { return m_data;    }
    auto        getRef()     const { return m_ref;     }
    void        setRef(Ref ref)    { m_ref = ref;      }
    Node *getpParent()     const  { return m_pParent; }
    Node* const* getpChildren()   const  { return m_pChild.data(); }

// protected: // TODO: Add this class as friend of the BinaryTree
        // and make these methods private
    void      setpChild(const Node *pChild, size_t pos)  { m_pChild[pos] = pChild;  }
    Node    * getChild(size_t branch){ return m_pChild[branch];  }
    Node    *&getChildRef(size_t branch){ return m_pChild[branch];  }
    Node    * getParent() { return m_pParent;   }
    auto      getChildren() { return m_pChild;  }
};

template <typename Container>
class binary_tree_forward_iterator
{
    private:
        using T = typename Container::T;
        using Node = typename Container::Node;
        using iterator = binary_tree_forward_iterator<Container>;
        Container* m_tree = nullptr;
        Node* m_node = nullptr;
    public:
        binary_tree_forward_iterator(Container* tree, Node* node)
            : m_tree(tree), m_node(node)
        {}
        binary_tree_forward_iterator(binary_tree_forward_iterator &other)
            : m_tree(other.m_tree), m_node(other.m_node)
        {}
        bool operator==(binary_tree_forward_iterator& other)
        {
            return m_node == other.m_node;
        }
        bool operator!=(binary_tree_forward_iterator& other)
        {
            return !(*this == other);
        }
        iterator& operator++()
        {
            if (m_node == nullptr) return *this;
            if (m_node->getChildren()[1]) {
                m_node = m_node->getChildren()[1];
                while (m_node->getChildren()[0]) {
                    m_node = m_node->getChildren()[0];
                }
            } else {
                Node* parent = m_node->getParent();
                while (parent && m_node == parent->getChildren()[1]) {
                    m_node = parent;
                    parent = parent->getParent();
                }
                m_node = parent;
            }
            return *this;
        }
        T& operator*()
        {
            return m_node->getDataRef();
        }
        Node* operator->()
        {
            return m_node;
        }
};

template <typename Container>
class binary_tree_backward_iterator
{
    private:
        using T = typename Container::T;
        using Node = typename Container::Node;
        using iterator = binary_tree_backward_iterator<Container>;
        Container* m_tree = nullptr;
        Node* m_node = nullptr;
    public:
        binary_tree_backward_iterator(Container* tree, Node* node)
            : m_tree(tree), m_node(node)
        {}
        binary_tree_backward_iterator(binary_tree_backward_iterator &other)
            : m_tree(other.m_tree), m_node(other.m_node)
        {}
        bool operator==(binary_tree_backward_iterator& other)
        {
            return m_node == other.m_node;
        }
        bool operator!=(binary_tree_backward_iterator& other)
        {
            return !(*this == other);
        }
        iterator& operator++()
        {
            if (m_node == nullptr) return *this;
            if (m_node->getChildren()[0]) {
                m_node = m_node->getChildren()[0];
                while (m_node->getChildren()[1]) {
                    m_node = m_node->getChildren()[1];
                }
            } else {
                Node* parent = m_node->getParent();
                while (parent && m_node == parent->getChildren()[0]) {
                    m_node = parent;
                    parent = parent->getParent();
                }
                m_node = parent;
            }
            return *this;
        }
        T& operator*()
        {
            return m_node->getDataRef();
        }
        Node* operator->()
        {
            return m_node;
        }
};

template <typename _T>
struct BinaryTreeAscTraits{
    using  T   = _T;
    using  Node         = CBinaryTreeNode<BinaryTreeAscTraits<_T>>;
    using  CompareFn    = less<T>;
};

template <typename _T>
struct BinaryTreeDescTraits
{
    using  T   = _T;
    using  Node         = CBinaryTreeNode<BinaryTreeDescTraits<_T>>;
    using  CompareFn    = greater<T>;
};

template <typename Traits>
class CBinaryTree{
public:
    using T    = typename Traits::T;
    using Node          = typename Traits::Node;
    
    using CompareFn         = typename Traits::CompareFn;
    using Container         = CBinaryTree<Traits>;
    using forward_iterator  = binary_tree_forward_iterator<Container>;
    using backward_iterator = binary_tree_backward_iterator<Container>;

protected:
    Node    *m_pRoot = nullptr;
    size_t   m_size  = 0;
    CompareFn Compfn;

public: 
    size_t  size()  const       { return m_size;       }
    bool    empty() const       { return size() == 0;  }
    Node*   getRoot() const     { return m_pRoot;      }

    void insert(T elem, Ref ref) {
        m_pRoot = internal_insert(elem, ref, nullptr, m_pRoot);
    }

     Node* getExtremeNode(Node* startNode, int direction) const {
        if (!startNode) return nullptr;
        
        Node* pNode = startNode;
        while (pNode->getChild(direction)) {
            pNode = pNode->getChild(direction);
        }
        return pNode;
    }

    backward_iterator rbegin() { return backward_iterator(this, getExtremeNode(m_pRoot, 1)); }
    backward_iterator rend()   { return backward_iterator(this, nullptr); }
    forward_iterator  begin()  { return forward_iterator (this, getExtremeNode(m_pRoot, 0)); }
    forward_iterator  end()    { return forward_iterator (this, nullptr); }

protected:
    Node* CreateNode(Node* pParent, T elem, Ref ref) {
        return new Node(pParent, elem, ref);
    }
    virtual Node* internal_insert(T &elem, Ref ref,
                                  Node* pParent, Node*& rpOrigin)
    {
        if (!rpOrigin) {
            ++m_size;
            return (rpOrigin = CreateNode(pParent, elem, ref));
        }

        size_t branch = Compfn(elem, rpOrigin->getDataRef()) ? 0 : 1;
        rpOrigin->getChildRef(branch) = internal_insert(elem, ref, rpOrigin, rpOrigin->getChildRef(branch));
        return rpOrigin;
    }
    template <typename Function, typename... Args>
    void internal_inorder(Node* pNode, Function func, Args&&... args) {
        if (!pNode) return;
        internal_inorder(pNode->getChild(0), func, std::forward<Args>(args)...);
        func(pNode, std::forward<Args>(args)...);
        internal_inorder(pNode->getChild(1), func, std::forward<Args>(args)...);
    }

    template <typename Function, typename... Args>
    void internal_preorder(Node* pNode, Function func, Args&&... args) {
        if (!pNode) return;
        func(pNode, std::forward<Args>(args)...);
        internal_preorder(pNode->getChild(0), func, std::forward<Args>(args)...);
        internal_preorder(pNode->getChild(1), func, std::forward<Args>(args)...);
    }

    template <typename Function, typename... Args>
    void internal_postorder(Node* pNode, Function func, Args&&... args) {
        if (!pNode) return;
        internal_postorder(pNode->getChild(0), func, std::forward<Args>(args)...);
        internal_postorder(pNode->getChild(1), func, std::forward<Args>(args)...);
        func(pNode, std::forward<Args>(args)...);
    }

public:
    CBinaryTree(){} // Empty tree
    
    // TODO: Copy Constructor. We have duplicate each node
    CBinaryTree(CBinaryTree  &other);
    
    // TODO: Done: Move Constructor
    CBinaryTree(CBinaryTree  &&other)
        : m_pRoot(std::exchange(other.m_pRoot, nullptr)), 
          m_size (std::exchange(other.m_size, 0)), 
          Compfn (std::exchange(other.Compfn, nullptr))
    { }

    // TODO: Recursivo y seguro. Destruir Nodes recursivamente
    virtual ~CBinaryTree(){  } 
    
    // TODO: begin debe comenzar el el nodo mas a la derecha (1)
    // riterator rbegin(){ 
    //     if (!m_pRoot) return rend();
    //     return iterator(this, getExtremeNode(m_pRoot, 1));
    //  }
    // riterator rend()  { return iterator(this, nullptr); }

    // Google: C++ parameter packs cplusplus
    template <typename Function, typename... Args>
    void inorder(Function func, Args&&... args){
        internal_inorder(m_pRoot, func, std::forward<Args>(args)...);
    }

    void inorder(){
        auto print = [](Node* pNode, const string& suffix){ 
            cout << pNode->getData() << "(" << pNode->getRef() << ")" << suffix;
        };
        inorder(print, " ");
    }

    // Variadic templates (See foreach.h)
    template <typename Function, typename... Args>
    void postorder(Function func, Args&&... args){
        internal_postorder(m_pRoot, func, std::forward<Args>(args)...);
    }
    void postorder(){
        auto print = [](Node* pNode, const string& suffix){ 
            cout << pNode->getData() << "(" << pNode->getRef() << ")" << suffix;
        };
        postorder(print, " ");
    }

    template <typename Function, typename... Args>
    void preorder(Function func, Args&&... args){
        internal_preorder(m_pRoot, func, std::forward<Args>(args)...);
    }
    void preorder(){
        auto print = [](Node* pNode, const string& suffix){ 
            cout << pNode->getData() << "(" << pNode->getRef() << ")" << suffix;
        };
        preorder(print, " ");
    }

    void print    (ostream &os)    {   print    (m_pRoot, 0, os);  }
    // TODO: generalize this function to apply any function
    // Google: C++ parameter packs cplusplus
    void print(Node  *pNode, size_t level, ostream &os){
        if( pNode ){
            Node *pParent = pNode->getParent();
            print(pNode->getChild(1), level+1, os);
            for (size_t i = 0; i < level; ++i) {
                os << " | ";
            }
            os << pNode->getDataRef() << "(" << (pParent?to_string(pParent->getData()):"Root") << ")" <<endl;
            print(pNode->getChild(0), level+1, os);
        }
    }
    void print(){
        cout << getTreeDisplay(m_pRoot) << endl;
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
    return os;
}

template <typename Traits>
istream & operator>>(istream &is, CBinaryTree<Traits> &obj){
    // Leer el arbol
    return is;
}

void DemoBinaryTree();

#endif // __BINARY_TREE_H__