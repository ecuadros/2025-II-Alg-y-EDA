#ifndef __BINARY_TREE_H__  
#define __BINARY_TREE_H__ 
#include <utility>
#include <algorithm>
#include <cassert>
#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include <functional>
#include "types.h"
#include "util.h"
using namespace std;

template <typename Traits>
class CBinaryTreeNode{
public:
  using value_type = typename Traits::T;
  using Node       = CBinaryTreeNode<Traits>;

  // Friend class so CBinaryTree can access protected members
  template <typename T> friend class CBinaryTree;

protected:
    value_type  m_data;
    Node *      m_pParent = nullptr;
    Ref         m_ref;
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
    value_type  getData()                {   return m_data;    }
    value_type &getDataRef()             {   return m_data;    }
 
protected: // TODO (DONE): Friend class added - CBinaryTree can access these protected members
    void      setpChild(const Node *pChild, size_t pos)  {   m_pChild[pos] = pChild;  }
    Node    * getChild(size_t branch){ return m_pChild[branch];  }
    Node    *&getChildRef(size_t branch){ return m_pChild[branch];  }
    Node    * getParent() { return m_pParent;   }
};

// Comentado temporalmente
/*
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
*/

template <typename _T>
struct BinaryTreeAscTraits{
    using  T         = _T;
    using  Node      = CBinaryTreeNode<BinaryTreeAscTraits<_T>>;
    using  CompareFn = less<_T>;
};

template <typename _T>
struct BinaryTreeDescTraits
{
    using  T         = _T;
    using  Node      = CBinaryTreeNode<BinaryTreeDescTraits<_T>>;
    using  CompareFn = greater<_T>;
};

template <typename Traits>
class CBinaryTree{
public:
    using value_type    = typename Traits::T;
    using Node          = typename Traits::Node;
    
    using CompareFn     = typename Traits::CompareFn;
    using Container     = CBinaryTree<Traits>;
    // TODO: forward iterator
    // using iterator      = binary_tree_iterator<Container>;

protected:
    Node    *m_pRoot = nullptr;
    size_t   m_size  = 0;
    CompareFn Compfn;
public: 
    size_t  size()  const       { return m_size;       }
    bool    empty() const       { return size() == 0;  }
    // TODO: insert must receive two paramaters: elem and Ref value
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
        rpOrigin->getChildRef(branch) = internal_insert(elem, ref, rpOrigin, rpOrigin->getChildRef(branch));
        return rpOrigin;
    }
public:
    CBinaryTree(){} // Empty tree
    
    // TODO: Copy Constructor. We have duplicate each node
    CBinaryTree(const Container &other);
    
    // Move Constructor
    CBinaryTree(Container &&other)
        : m_pRoot(std::exchange(other.m_pRoot, nullptr)), 
          m_size (std::exchange(other.m_size, 0)), 
          Compfn (std::move(other.Compfn))
    { }

    // TODO: Recursivo y seguro. Destruir Nodes recursivamente
    virtual ~CBinaryTree(){  } 
    
    // TODO (DONE): Generalized inorder traversal - accepts any function/lambda
    // Overload for ostream (backward compatibility)
    void inorder(ostream &os) { inorder(m_pRoot, 0, os); }
    
    // Generalized version - accepts any callable (function, lambda, functor)
    template <typename Function>
    void inorder(Function func) {
        inorder(m_pRoot, func);
    }
    
private:
    // Private helper for ostream version
    void inorder(Node *pNode, size_t level, ostream &os) {
        if (pNode) {
            inorder(pNode->getChild(0), level + 1, os);
            os << " --> " << pNode->getDataRef();
            inorder(pNode->getChild(1), level + 1, os);
        }
    }
    
    // Private helper for generalized version
    template <typename Function>
    void inorder(Node *pNode, Function func) {
        if (pNode) {
            inorder(pNode->getChild(0), func);
            func(pNode->getDataRef());
            inorder(pNode->getChild(1), func);
        }
    }

public:

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
    
    // TODO (DONE): Generalized postorder traversal - accepts any function/lambda
    // Overload for ostream (backward compatibility)
    void postorder(ostream &os) { postorder_ostream(m_pRoot, 0, os); }
    
    // Generalized version - accepts any callable (function, lambda, functor)
    template <typename Function>
    void postorder(Function func) {
        postorder_impl(m_pRoot, func);
    }

private:
    // Private helper for ostream version
    void postorder_ostream(Node  *pNode, size_t level, ostream &os){
        if( pNode ){   
            postorder_ostream(pNode->getChild(0), level+1, os);
            postorder_ostream(pNode->getChild(1), level+1, os);
            os << " --> " << pNode->getDataRef();
        }
    }
    
    // Private helper for generalized version
    template <typename Function>
    void postorder_impl(Node *pNode, Function func) {
        if (pNode) {
            postorder_impl(pNode->getChild(0), func);
            postorder_impl(pNode->getChild(1), func);
            func(pNode->getDataRef());
        }
    }

public:

    // TODO (DONE): Generalized preorder traversal - accepts any function/lambda
    // Overload for ostream (backward compatibility)
    void preorder(ostream &os) { preorder(m_pRoot, 0, os); }
    
    // Generalized version - accepts any callable (function, lambda, functor)
    template <typename Function>
    void preorder(Function func) {
        preorder(m_pRoot, func);
    }
    
private:
    // Private helper for ostream version
    void preorder(Node *pNode, size_t level, ostream &os) {
        if (pNode) {
            os << " --> " << pNode->getDataRef();
            preorder(pNode->getChild(0), level + 1, os);
            preorder(pNode->getChild(1), level + 1, os);
        }
    }
    
    // Private helper for generalized version
    template <typename Function>
    void preorder(Node *pNode, Function func) {
        if (pNode) {
            func(pNode->getDataRef());
            preorder(pNode->getChild(0), func);
            preorder(pNode->getChild(1), func);
        }
    }

public:
    void print    (ostream &os)    {   print    (m_pRoot, 0, os);  }
    // TODO: generalize this function to apply any function
    // Google: C++ parameter packs cplusplus
    void print(Node  *pNode, size_t level, ostream &os){
        if( pNode ){
            Node *pParent = pNode->getParent();
            print(pNode->getChild(1), level+1, os);
            for(size_t i = 0; i < level; ++i) os << " | ";
            os << pNode->getDataRef() << "(" << (pParent?to_string(pParent->getData()):"Root") << ")" <<endl;
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