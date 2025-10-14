#ifndef __BINARY_TREE_H__
#define __BINARY_TREE_H__
#include <utility>
#include <algorithm>
#include <cassert>
#include <fstream>
#include <string>
#include <vector>
#include "types.h"
using namespace std;

template <typename Traits>
class CBinaryTreeNode{
public:
  using value_type = typename Traits::T;
  using Node       = CBinaryTreeNode<Traits>;

    template <typename> friend class CBinaryTree;
    template <typename> friend class CAVLTree;

protected:
    value_type     m_data;
    Node          *m_pParent = nullptr;
    Ref            m_ref;
    vector<Node *> m_pChild  = {nullptr, nullptr}; // 2 hijos inicializados en nullptr

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

    value_type  getData()                {   return m_data;    }
    value_type &getDataRef()             {   return m_data;    }
 
protected: // TODO: Add this class as friend of the BinaryTree
        // and make these methods private
    void      setpChild(const Node *pChild, size_t pos)  {   m_pChild[pos] = pChild;  }
    Node    * getChild(size_t branch){ return m_pChild[branch];  }
    Node    *&getChildRef(size_t branch){ return m_pChild[branch];  }
    Node    * getParent() { return m_pParent;   }
    Node    *&getParentRef() { return m_pParent;   }
};

template <typename _T>
struct BinaryTreeAscTraits{
    using  T         = _T;
    using  Node      = CBinaryTreeNode<BinaryTreeAscTraits<_T>>;
    using  CompareFn = std::less<_T>;
};

template <typename _T>
struct BinaryTreeDescTraits
{
    using  T         = _T;
    using  Node      = CBinaryTreeNode<BinaryTreeDescTraits<_T>>;
    using  CompareFn = std::greater<_T>;
};

template <typename Traits>
class CBinaryTree{
public:
    using value_type    = typename Traits::T;
    using Node          = typename Traits::Node;
    
    using CompareFn     = typename Traits::CompareFn;
    using Container     = CBinaryTree<Traits>;

protected:
    Node    *m_pRoot = nullptr;
    size_t   m_size  = 0;
    CompareFn Compfn;
public: 
    size_t  size()  const       { return m_size;       }
    bool    empty() const       { return size() == 0;  }

    void insert(value_type elem, Ref ref) {
        m_pRoot = internal_insert(elem, ref, nullptr, m_pRoot);
    }

     Node* getExtremeNode(Node* startNode, int direction) const {
        if (!startNode) return nullptr;

        Node* pNode = startNode;
        while (pNode->getChild(direction)) {
            pNode = static_cast<Node*>(pNode->getChild(direction));
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
        Node*& childRef = reinterpret_cast<Node*&>(rpOrigin->getChildRef(branch));
        internal_insert(elem, ref, rpOrigin, childRef);
        return rpOrigin;
    }
public:
    CBinaryTree(){} // Empty tree
    
    CBinaryTree(CBinaryTree &other);

    CBinaryTree(CBinaryTree &&other)
        : m_pRoot(std::exchange(other.m_pRoot, nullptr)),
          m_size (std::exchange(other.m_size, 0)),
          Compfn (std::exchange(other.Compfn, CompareFn()))
    { }

    virtual ~CBinaryTree(){
        delete m_pRoot;
        m_pRoot = nullptr;
    } 
    

    void inorder(ostream &os){
        inorderOS(m_pRoot, 0, os);
    }

private:
    void inorderOS(Node  *pNode, size_t level, ostream &os){
        if( pNode ){
            inorderOS(static_cast<Node*>(pNode->getChild(0)), level+1, os);
            os << " --> " << pNode->getDataRef();
            inorderOS(static_cast<Node*>(pNode->getChild(1)), level+1, os);
        }
    }

public:
    template <typename Function, typename... Args>
    void inorder(Function func, Args const&... args){
        inorderGeneric(m_pRoot, 0, func, args...);
    }

private:
    template <typename Function, typename... Args>
    void inorderGeneric(Node* pNode, size_t level,
                 Function func, Args const&... args) {
        if (pNode) {
            inorderGeneric(static_cast<Node*>(pNode->getChild(0)), level + 1, func, args...);
            func(pNode, level);
            inorderGeneric(static_cast<Node*>(pNode->getChild(1)), level + 1, func, args...);
        }
    }

public:
    template <typename Function, typename... Args>
    void postorder(Function func, Args const&... args){
        postorderGeneric(m_pRoot, 0, func, args...);
    }

private:
    template <typename Function,typename... Args>
    void postorderGeneric(Node* pNode, size_t level,
                   Function func, Args const&... args) {
        if (pNode) {
            postorderGeneric(static_cast<Node*>(pNode->getChild(0)), level + 1, func, args...);
            postorderGeneric(static_cast<Node*>(pNode->getChild(1)), level + 1, func, args...);
            func(pNode, level);
        }
    }

public:
    void preorder(ostream &os){
        preorderOS(m_pRoot, 0, os);
    }

private:
    void preorderOS(Node  *pNode, size_t level, ostream &os){
        if( pNode ){
            os << " --> " << pNode->getDataRef();
            preorderOS(static_cast<Node*>(pNode->getChild(0)), level+1, os);
            preorderOS(static_cast<Node*>(pNode->getChild(1)), level+1, os);
        }
    }

public:
    template <typename Function, typename... Args>
    void preorder(Function func, Args const&... args){
        preorderGeneric(m_pRoot, 0, func, args...);
    }

private:
    template <typename Function, typename... Args>
    void preorderGeneric(Node* pNode, size_t level,
                  Function func, Args const&... args) {
        if (pNode) {
            func(pNode, level);
            preorderGeneric(static_cast<Node*>(pNode->getChild(0)), level + 1, func, args...);
            preorderGeneric(static_cast<Node*>(pNode->getChild(1)), level + 1, func, args...);
        }
    }

public:
    void print(ostream &os){
        printInternal(m_pRoot, 0, os);
    }

private:
    void printInternal(Node  *pNode, size_t level, ostream &os){
        if( pNode ){
            Node *pParent = static_cast<Node*>(pNode->getParent());
            printInternal(static_cast<Node*>(pNode->getChild(1)), level+1, os);
            for(size_t i = 0; i < level; ++i) os << " | ";
            os << pNode->getDataRef() << "(" << (pParent?to_string(pParent->getData()):"Root") << ")" <<endl;
            printInternal(static_cast<Node*>(pNode->getChild(0)), level+1, os);
        }
    }

public:
    void Write(ostream &os) { os << *this;  }

    void Read(istream &is)  { }
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