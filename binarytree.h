#ifndef __BINARY_TREE_H__  
#define __BINARY_TREE_H__ 
#include <cassert>
#include <fstream>
#include "general_iterator.h"
#include "types.h"
using namespace std;

// Forward declaration of CBinaryTree so we can friend it from CBinaryTreeNode
template <typename Traits>
class CBinaryTree;

template <typename Traits>
class CBinaryTreeNode{
public:
    using value_type = typename Traits::T;
    using Node       = CBinaryTreeNode<Traits>;

protected:
    friend class CBinaryTree<Traits>;

    value_type     m_data;
    Ref            m_ref;
    Node          *m_pParent = nullptr;
    vector<Node *> m_pChild  = {nullptr, nullptr};

public:
    CBinaryTreeNode(Node* pParent, value_type data, Ref ref, Node* p0 = nullptr, Node* p1 = nullptr)
        : m_data(data), m_ref(ref), m_pParent(pParent)
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
    void      setpChild(Node *pChild, size_t pos)  {   m_pChild[pos] = pChild;  }
    Node    * getChild(size_t branch){ return m_pChild[branch];  }
    Node    *&getChildRef(size_t branch){ return m_pChild[branch];  }
    Node    * getParent() { return m_pParent;   }
    void     setParent(Node* pParent) { m_pParent = pParent; }
};

//forward iterator
template <typename Container>
class binary_tree_forward_iterator : public general_iterator<Container, binary_tree_forward_iterator<Container>> {
public:
    using Parent = general_iterator<Container, binary_tree_forward_iterator<Container>>;
    using Node = typename Container::Node;
    using value_type = typename Container::value_type;

    binary_tree_forward_iterator(Container* pContainer = nullptr, Node* pNode = nullptr): Parent(pContainer, pNode) {}

    binary_tree_forward_iterator(const binary_tree_forward_iterator&) = default;
    binary_tree_forward_iterator(binary_tree_forward_iterator&&) = default;
    binary_tree_forward_iterator& operator=(const binary_tree_forward_iterator&) = default;

    binary_tree_forward_iterator& operator++() {
        Node* cur = Parent::m_pNode;
        if (!cur) return *this;
        if (cur->getChild(1)) {
            cur = static_cast<Node*>(cur->getChild(1));
            while (cur->getChild(0)) cur = static_cast<Node*>(cur->getChild(0));
            Parent::m_pNode = cur;
            return *this;
        }
        Node* parent = static_cast<Node*>(cur->getParent());
        while (parent && cur == parent->getChild(1)){
            cur = parent;
            parent = static_cast<Node*>(parent->getParent());
        }
        Parent::m_pNode = parent;
        return *this;
    }

    value_type& operator*() const { return Parent::m_pNode->getDataRef(); }
    Node* node() const { return Parent::m_pNode; }
};

//backward iterator
template <typename Container>
class binary_tree_backward_iterator : public general_iterator<Container, binary_tree_backward_iterator<Container>> {
public:
    using value_type = typename Container::value_type;
    using Node = typename Container::Node;
    using Parent = general_iterator<Container, binary_tree_backward_iterator<Container>>;

    binary_tree_backward_iterator(Container* pContainer = nullptr, Node* pNode = nullptr)
        : Parent(pContainer, pNode) {}

    binary_tree_backward_iterator(const binary_tree_backward_iterator&) = default;
    binary_tree_backward_iterator(binary_tree_backward_iterator&&) = default;
    binary_tree_backward_iterator& operator=(const binary_tree_backward_iterator&) = default;

    binary_tree_backward_iterator& operator++() {
        Node* cur = Parent::m_pNode;
        if (!cur) return *this;
        if (cur->getChild(0)) {
            cur = static_cast<Node*>(cur->getChild(0));
            while (cur->getChild(1)) cur = static_cast<Node*>(cur->getChild(1));
            Parent::m_pNode = cur;
            return *this;
        }
        Node* parent = static_cast<Node*>(cur->getParent());
        while (parent && cur == parent->getChild(0)){
            cur = parent;
            parent = static_cast<Node*>(parent->getParent());
        }
        Parent::m_pNode = parent; 
        return *this;
    }

    value_type& operator*() const { return Parent::m_pNode->getDataRef(); }
    Node* node() const { return Parent::m_pNode; }
};

template <typename _T>
struct BinaryTreeAscTraits{
    using  T         = _T;
    using  Node      = CBinaryTreeNode< BinaryTreeAscTraits<_T> >;
    using  CompareFn = std::less<T>;
};

template <typename _T>
struct BinaryTreeDescTraits
{
    using  T         = _T;
    using  Node      = CBinaryTreeNode< BinaryTreeDescTraits<_T> >;
    using  CompareFn = std::greater<T>;
};

template <typename Traits>
class CBinaryTree{
public:
    using value_type    = typename Traits::T;
    using Node          = typename Traits::Node;
    using CompareFn     = typename Traits::CompareFn;
    using Container     = CBinaryTree<Traits>;
    using forward_iterator = binary_tree_forward_iterator<Container>;
    using backward_iterator = binary_tree_backward_iterator<Container>;

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
        Node *pNode = internal_insert(elem, ref, rpOrigin, *(Node**)(&rpOrigin->getChildRef(branch)));
        return pNode;
    }
public:
    CBinaryTree() : Compfn(CompareFn()) {} // Empty tree with default comparator
    
    CBinaryTree(const CBinaryTree &other);
    CBinaryTree(CBinaryTree &&other)
            : m_pRoot(std::exchange(other.m_pRoot, nullptr)), 
                m_size (std::exchange(other.m_size, 0)), 
                Compfn (std::move(other.Compfn))
    { }

    virtual ~CBinaryTree(){  } 
    
    forward_iterator begin() { 
        if (!m_pRoot) return end();
        return forward_iterator(this, getExtremeNode(m_pRoot, 0));
    }
    forward_iterator end()   { return forward_iterator(this, nullptr); }

    backward_iterator rbegin() {
        if (!m_pRoot) return rend();
        return backward_iterator(this, getExtremeNode(m_pRoot, 1));
    }
    backward_iterator rend() { return backward_iterator(this, nullptr); }

    void inorder  (ostream &os)    {   inorder  (m_pRoot, 0, os);  }
    void inorder(Node  *pNode, size_t level, ostream &os){
        if( pNode ){
            inorder(static_cast<Node*>(pNode->getChild(0)), level+1, os);
            os << " --> " << pNode->getDataRef();
            inorder(static_cast<Node*>(pNode->getChild(1)), level+1, os);
        }
    }

    void inorder(Node  *pNode, void (*visit) (value_type& item)){
        if( pNode ){   
            inorder(static_cast<Node*>(pNode->getChild(0)), *visit);
            (*visit)(pNode->getDataRef());
            inorder(static_cast<Node*>(pNode->getChild(1)), *visit);
        }
    }

    template <typename Function, typename... Args>
    void postorder_apply(Function func, Args const&... args)
    {    postorder_apply(m_pRoot, 0, func, args...);} 

    void postorder(std::ostream &os) { postorder(m_pRoot, 0, os); }

    template <typename Function,typename... Args>
    void postorder_apply(Node* pNode, size_t level, 
                   Function func, Args const&... args) {
        if (pNode) {
            postorder_apply(static_cast<Node*>(pNode->getChild(0)), level + 1, func, args...);
            postorder_apply(static_cast<Node*>(pNode->getChild(1)), level + 1, func, args...);
            func(pNode, level, args...); 
        }
    }
    void postorder(Node  *pNode, size_t level, ostream &os){
        if( pNode ){   
            postorder(static_cast<Node*>(pNode->getChild(0)), level+1, os);
            postorder(static_cast<Node*>(pNode->getChild(1)), level+1, os);
            os << " --> " << pNode->getDataRef();
        }
    }

    void preorder (ostream &os)    {   preorder (m_pRoot, 0, os);  }
    void preorder(Node  *pNode, size_t level, ostream &os){
        if( pNode ){   
            os << " --> " << pNode->getDataRef();
            preorder(static_cast<Node*>(pNode->getChild(0)), level+1, os);
            preorder(static_cast<Node*>(pNode->getChild(1)), level+1, os);            
        }
    }

    void print    (ostream &os)    {   print    (m_pRoot, 0, os);  }
    void print(Node  *pNode, size_t level, ostream &os){
        if( pNode ){
            Node *pParent = static_cast<Node*>(pNode->getParent());
            print(static_cast<Node*>(pNode->getChild(1)), level+1, os);
            for(size_t i=0;i<level;i++) os << "    ";
            if (pParent) {
                os << pNode->getDataRef() << "(" << pParent->getData() << ")" << endl;
            } else {
                os << pNode->getDataRef() << "(Root)" << endl;
            }
            print(static_cast<Node*>(pNode->getChild(0)), level+1, os);
        }
    }

    void Write(ostream &os) { os << *this;  }
    void Read(istream &is)  { /* TODO */  }
};

template <typename Traits>
ostream & operator<<(std::ostream &os, CBinaryTree<Traits> &obj){
    os << "CBinaryTree with " << obj.size() << " elements.";
    obj.inorder(os);
    return os;
}

template <typename Traits>
istream & operator>>(istream &is, CBinaryTree<Traits> &obj){
    return is;
}

void DemoBinaryTree();

#endif // __BINARY_TREE_H__