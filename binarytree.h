#ifndef __BINARY_TREE_H__  
#define __BINARY_TREE_H__ 
//#include <utility>
//#include <algorithm>
#include <cassert>
#include <fstream>
#include "types.h"
//#include "util.h"
using namespace std;

template <typename Traits>
class CBinaryTreeNode{
public:
  using value_type = typename Traits::T;
  using Node       = CBinaryTreeNode<T>;

protected:
    T       m_data;
    Node *  m_pParent = nullptr;
    Ref     m_ref;
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

    T         getKey()                 {   return m_data;    }
    T         getData()                {   return m_data;    }
    T        &getDataRef()             {   return m_data;    }
 
public: // TODO: Add this class as friend of the BinaryTree
        // and make these methods private
    void      setpChild(const Node *pChild, size_t pos)  {   m_pChild[pos] = pChild;  }
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
    using Container = binary_tree_iterator<Container>;

  public:
    binary_tree_iterator(Container *pContainer, Node *pNode) : Parent (pContainer,pNode) {}
    binary_tree_iterator(Container &other)  : Parent (other) {}
    binary_tree_iterator(Container &&other) : Parent(other) {} // Move constructor C++11 en adelante

public:
    binary_tree_iterator operator++() {
        if (!Parent::m_pNode) {
            return *this;
        }

        if (Parent::m_pNode->getChild(1)) {
            Parent::m_pNode = Parent::m_pNode->getChild(1);
            while (Parent::m_pNode->getChild(0)) {
                Parent::m_pNode = Parent::m_pNode->getChild(0);
            }
        } else {
            Node *pParent = Parent::m_pNode->getParent();
            while (pParent && Parent::m_pNode == pParent->getChild(1)) {
                Parent::m_pNode = pParent;
                pParent = pParent->getParent();
            }
            Parent::m_pNode = pParent;
        }

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
    CompareFn Compfn;
public: 
    size_t  size()  const       { return m_size;       }
    bool    empty() const       { return size() == 0;  }
    void insert(value_type elem, Ref ref) {
        internal_insert(elem, ref, m_pRoot, nullptr);
    }

protected:
    void internal_insert(value_type &elem, Ref ref, Node *&pNode, Node *pParent) {
        if (!pNode) {
            pNode = new Node(pParent, elem, ref);
            m_size++;
        } else {
            if (Compfn(elem, pNode->getData())) {
                internal_insert(elem, ref, pNode->getChildRef(0), pNode);
            } else {
                internal_insert(elem, ref, pNode->getChildRef(1), pNode);
            }
        }
    }
public:

    CBinaryTree(){} // Empty tree
    
    CBinaryTree(const CBinaryTree<Traits> &other) {
        m_pRoot = copy_nodes(other.m_pRoot, nullptr);
        m_size = other.m_size;
    }

private:
    Node *copy_nodes(Node *pNode, Node *pParent) {
        if (!pNode) {
            return nullptr;
        }
        Node *pNewNode = new Node(pParent, pNode->getData(), pNode->m_ref);
        pNewNode->getChildRef(0) = copy_nodes(pNode->getChild(0), pNewNode);
        pNewNode->getChildRef(1) = copy_nodes(pNode->getChild(1), pNewNode);
        return pNewNode;
    }
public:

    
    // Move Constructor
    CBinaryTree(Binary &&other)
        : m_pRoot(std::exchange(other.m_pRoot, nullptr)), 
          m_size (std::exchange(other.m_size, 0)), 
          Compfn (std::exchange(other.Compfn, nullptr))
    { }

    virtual ~CBinaryTree() {
        clear(m_pRoot);
    }

private:
    void clear(Node *pNode) {
        if (pNode) {
            clear(pNode->getChild(0));
            clear(pNode->getChild(1));
            delete pNode;
        }
    }
public:

    template <typename Function, typename... Args>
    void inorder(Function func, Args const&... args)
    {    inorder(m_pRoot, 0, func, args...);}

    template <typename Function,typename... Args>
    void inorder(Node* pNode, size_t level, 
                   Function func, Args const&... args) {
        if (pNode) {
            inorder(pNode->getChild(0), level + 1, func, args...);
            func(pNode, level);
            inorder(pNode->getChild(1), level + 1, func, args...);
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


    template <typename Function, typename... Args>
    void preorder(Function func, Args const&... args)
    {    preorder(m_pRoot, 0, func, args...);}

    template <typename Function,typename... Args>
    void preorder(Node* pNode, size_t level, 
                   Function func, Args const&... args) {
        if (pNode) {
            func(pNode, level);
            preorder(pNode->getChild(0), level + 1, func, args...);
            preorder(pNode->getChild(1), level + 1, func, args...);
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

    void Read(istream &is) {
        clear(m_pRoot);
        m_pRoot = nullptr;
        m_size = 0;
        read_preorder(is, m_pRoot, nullptr);
    }

private:
    void read_preorder(istream &is, Node *&pNode, Node *pParent) {
        value_type data;
        Ref ref;
        if (is >> data >> ref) {
            pNode = new Node(pParent, data, ref);
            m_size++;
            read_preorder(is, pNode->getChildRef(0), pNode);
            read_preorder(is, pNode->getChildRef(1), pNode);
        } else {
            is.clear();
            string token;
            is >> token;
        }
    }
public:

};

// TODO: este operator << debe seguir estando fuera de la clase
template <typename Traits>
ostream & operator<<(std::ostream &os, CBinaryTree<Traits> &obj){
    os << "CBinaryTree with " << obj.size() << " elements.";
    obj.preorder([&os](auto pNode, auto level){
        os << " --> " << pNode->getDataRef();
    });
    return os;
}


template <typename Traits>
istream & operator>>(istream &is, CBinaryTree<Traits> &obj){
    obj.Read(is);
    return is;
}

void DemoBinaryTree();

#endif // __BINARY_TREE_H__