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
class CBinaryTree;

template <typename Traits>
class CBinaryTreeNode{
    friend class CBinaryTree<Traits>;

public:
  using value_type = typename Traits::T;
  using Node       = CBinaryTreeNode<T>;

protected:
    value_type     m_data;
    Node          *m_pParent = nullptr;
    Ref            m_ref;
    vector<Node *> m_pChild  = {nullptr, nullptr};

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

private:
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
        if (!Parent::m_pNode) return *this;

        Node* current = Parent::m_pNode;

        if (current->getChild(1)) {
            current = current->getChild(1);
            while (current->getChild(0)) {
                current = current->getChild(0);
            }
            Parent::m_pNode = current;
        } else {
            Node* parent = current->getParent();
            while (parent && current == parent->getChild(1)) {
                current = parent;
                parent = parent->getParent();
            }
            Parent::m_pNode = parent;
        }

        return *this;
    }

    binary_tree_iterator operator--() {
        if (!Parent::m_pNode) return *this;

        Node* current = Parent::m_pNode;

        if (current->getChild(0)) {
            current = current->getChild(0);
            while (current->getChild(1)) {
                current = current->getChild(1);
            }
            Parent::m_pNode = current;
        } else {
            Node* parent = current->getParent();
            while (parent && current == parent->getChild(0)) {
                current = parent;
                parent = parent->getParent();
            }
            Parent::m_pNode = parent;
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
    using riterator     = binary_tree_iterator<Container>;

protected:
    Node    *m_pRoot = nullptr;
    size_t   m_size  = 0;
    CompareFn Compfn;
public: 
    size_t  size()  const       { return m_size;       }
    bool    empty() const       { return size() == 0;  }

    void insert(value_type elem, Ref ref) {
        m_pRoot = internal_insert(elem, ref, nullptr, nullptr, m_pRoot);
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
        Node *pNode = internal_insert(elem, ref, nullptr, rpOrigin, rpOrigin->getChildRef(branch));
        return pNode;
    }
protected:
    Node* copyNode(Node* sourceNode, Node* pParent) {
        if (!sourceNode) return nullptr;

        Node* newNode = CreateNode(pParent, sourceNode->getData(), sourceNode->m_ref);
        newNode->m_pChild[0] = copyNode(sourceNode->getChild(0), newNode);
        newNode->m_pChild[1] = copyNode(sourceNode->getChild(1), newNode);

        return newNode;
    }

public:
    CBinaryTree(){}

    CBinaryTree(const Container &other)
        : m_size(other.m_size), Compfn(other.Compfn)
    {
        m_pRoot = copyNode(other.m_pRoot, nullptr);
    }

    CBinaryTree(Container &&other)
        : m_pRoot(std::exchange(other.m_pRoot, nullptr)), 
          m_size (std::exchange(other.m_size, 0)), 
          Compfn (std::exchange(other.Compfn, nullptr))
    { }

    virtual ~CBinaryTree(){
        delete m_pRoot;
        m_pRoot = nullptr;
        m_size = 0;
    }

    iterator begin() {
        if (!m_pRoot) return end();
        return iterator(this, getExtremeNode(m_pRoot, 0));
    }
    iterator end()   { return iterator(this, nullptr); }

    riterator rbegin(){
        if (!m_pRoot) return rend();
        return riterator(this, getExtremeNode(m_pRoot, 1));
    }
    riterator rend()  { return riterator(this, nullptr); }

    template <typename Function, typename... Args>
    void inorder(Function func, Args const&... args) {
        inorder(m_pRoot, 0, func, args...);
    }

    template <typename Function, typename... Args>
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
    void preorder(Function func, Args const&... args) {
        preorder(m_pRoot, 0, func, args...);
    }

    template <typename Function, typename... Args>
    void preorder(Node* pNode, size_t level,
                  Function func, Args const&... args) {
        if (pNode) {
            func(pNode, level);
            preorder(pNode->getChild(0), level + 1, func, args...);
            preorder(pNode->getChild(1), level + 1, func, args...);
        }
    }

    template <typename Function, typename... Args>
    void print(Function func, Args const&... args) {
        print(m_pRoot, 0, func, args...);
    }

    template <typename Function, typename... Args>
    void print(Node* pNode, size_t level,
               Function func, Args const&... args) {
        if (pNode) {
            print(pNode->getChild(1), level + 1, func, args...);
            func(pNode, level);
            print(pNode->getChild(0), level + 1, func, args...);
        }
    }

    void Write(ostream &os) {
        os << m_size << " ";
        preorder([&os](Node* pNode, size_t level) {
            os << pNode->getDataRef() << " ";
        });
    }

    void Read(istream &is) {
        delete m_pRoot;
        m_pRoot = nullptr;

        size_t count;
        is >> count;

        for (size_t i = 0; i < count && is; ++i) {
            value_type value;
            is >> value;
            insert(value, Ref());
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