#ifndef __BINARY_TREE_H__  
#define __BINARY_TREE_H__ 
#include <cassert>
#include <fstream>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <string>
#include <functional>
#include <utility>
#include "types.h"
#include "general_iterator.h"
using namespace std;

template <typename Traits> class CBinaryTree;
template <typename Traits> class CBinaryTreeNode;

template <typename Traits>
class CBinaryTreeNode{
public:
  using value_type = typename Traits::T;
  using Node       = CBinaryTreeNode<Traits>;

protected:
    value_type       m_data;
    Node *  m_pParent = nullptr;
    Ref     m_ref;
    vector<Node *> m_pChild = {nullptr, nullptr}; // 2 hijos inicializados en nullptr

public:
    CBinaryTreeNode(Node* pParent, value_type data, Ref ref, Node* p0 = nullptr, Node* p1 = nullptr)
        : m_data(data), m_pParent(pParent), m_ref(ref)
    {
        m_pChild[0] = p0;
        m_pChild[1] = p1;
    }
    
    ~CBinaryTreeNode(){}

    value_type getData() const { return m_data; }
    value_type &getDataRef() { return m_data; }
    Ref getRef() const { return m_ref; }
 
private: // TODO: Add this class as friend of the BinaryTree
        // and make these methods private
    void setpChild(Node *pChild, size_t pos) { m_pChild[pos] = pChild; }
    Node* getChild(size_t branch) const { return m_pChild[branch]; }
    Node*& getChildRef(size_t branch) { return m_pChild[branch]; }
    Node* getParent() const { return m_pParent; }
    void setParent(Node* parent) { m_pParent = parent; }

    template <typename Container>
    friend class binary_tree_iterator;
    
    friend class CBinaryTree<Traits>;
};

template <typename _T>
struct BinaryTreeAscTraits{
    using T = _T;
    using Node = CBinaryTreeNode<BinaryTreeAscTraits<_T>>;
    using CompareFn = less<T>;
};

template <typename _T>
struct BinaryTreeDescTraits {
    using T = _T;
    using Node = CBinaryTreeNode<BinaryTreeDescTraits<_T>>;
    using CompareFn = greater<T>;
};

template <typename Container>
class binary_tree_iterator : public general_iterator<Container, binary_tree_iterator<Container>> {
public:
    using Parent = general_iterator<Container, binary_tree_iterator<Container>>;
    using Node = typename Container::Node;
    using value_type = typename Container::value_type;
    using iterator_category = std::bidirectional_iterator_tag;

public:
    binary_tree_iterator(Container* pContainer, Node* pNode) : Parent(pContainer, pNode) {}
    binary_tree_iterator(const binary_tree_iterator& other) : Parent(other) {}
    binary_tree_iterator(binary_tree_iterator&& other) : Parent(std::move(other)) {}

    // TODO: Revisar el avance de un iterator
    binary_tree_iterator& operator++() {
        Node* current = Parent::getNode();
        if(current == nullptr) return *this;

        if(current->getChild(1) != nullptr) {
            current = current->getChild(1);
            while(current->getChild(0) != nullptr) {
                current = current->getChild(0);
            }
            Parent::setNode(current);
        } else {
            Node* parent = current->getParent();
            while(parent != nullptr && current == parent->getChild(1)) {
                current = parent;
                parent = parent->getParent();
            }
            Parent::setNode(parent);
        }
        return *this;
    }

    binary_tree_iterator operator++(int) {
        binary_tree_iterator tmp(*this);
        ++(*this);
        return tmp;
    }

    binary_tree_iterator& operator--() {
        Node* current = Parent::getNode();
        
        if(current == nullptr) {
            current = Parent::getContainer()->m_pRoot;
            if(current != nullptr) {
                while(current->getChild(1) != nullptr) {
                    current = current->getChild(1);
                }
            }
            Parent::setNode(current);
            return *this;
        }

        if(current->getChild(0) != nullptr) {
            current = current->getChild(0);
            while(current->getChild(1) != nullptr) {
                current = current->getChild(1);
            }
            Parent::setNode(current);
        } else {
            Node* parent = current->getParent();
            while(parent != nullptr && current == parent->getChild(0)) {
                current = parent;
                parent = parent->getParent();
            }
            Parent::setNode(parent);
        }
        return *this;
    }

    binary_tree_iterator operator--(int) {
        binary_tree_iterator tmp(*this);
        --(*this);
        return tmp;
    }
};

template <typename Traits>
class CBinaryTree{
public:
    using value_type = typename Traits::T;
    using Node = typename Traits::Node;
    using CompareFn = typename Traits::CompareFn;
    using Container = CBinaryTree<Traits>;
    using iterator = binary_tree_iterator<Container>;

protected:
    Node* m_pRoot = nullptr;
    size_t m_size = 0;
    CompareFn Compfn;

private:
    void destroyTree(Node* node) {
        if(node == nullptr) return;
        destroyTree(node->getChild(0));
        destroyTree(node->getChild(1));
        delete node;
    }

    Node* copyTree(Node* original, Node* parent) {
        if(original == nullptr) return nullptr;
        Node* newNode = CreateNode(parent, original->getData(), original->getRef());
        newNode->setpChild(copyTree(original->getChild(0), newNode), 0);
        newNode->setpChild(copyTree(original->getChild(1), newNode), 1);
        return newNode;
    }

    Node* findLeftmost(Node* node) const {
        if(node == nullptr) return nullptr;
        while(node->getChild(0) != nullptr) {
            node = node->getChild(0);
        }
        return node;
    }

    Node* readPreorder(istream& is, Node* parent) {
        string token;
        if(!(is >> token) || token == "#") return nullptr;
        
        value_type data;
        Ref ref;
        istringstream dataStream(token);
        dataStream >> data;
        
        if(!(is >> ref)) throw runtime_error("Error reading Ref from stream");
        
        Node* newNode = CreateNode(parent, data, ref);
        ++m_size;
        
        newNode->setpChild(readPreorder(is, newNode), 0);
        newNode->setpChild(readPreorder(is, newNode), 1);
        
        return newNode;
    }

    void writePreorder(ostream& os, Node* node) const {
        if(node == nullptr) {
            os << "# ";
            return;
        }
        os << node->getData() << " " << node->getRef() << " ";
        writePreorder(os, node->getChild(0));
        writePreorder(os, node->getChild(1));
    }

public: 
    size_t size() const { return m_size; }
    bool empty() const { return size() == 0; }

    void insert(value_type elem, Ref ref) {
        m_pRoot = internal_insert(elem, ref, nullptr, m_pRoot);
    }

    iterator begin() { return iterator(this, findLeftmost(m_pRoot)); }
    iterator end() { return iterator(this, nullptr); }

protected:
    Node* CreateNode(Node* pParent, value_type elem, Ref ref) {
        return new Node(pParent, elem, ref);
    }
    
    virtual Node* internal_insert(value_type& elem, Ref ref, Node* pParent, Node*& rpOrigin) {
        if(!rpOrigin) {
            ++m_size;
            rpOrigin = CreateNode(pParent, elem, ref);
            return rpOrigin;
        }
        size_t branch = Compfn(elem, rpOrigin->getDataRef()) ? 0 : 1;
        internal_insert(elem, ref, rpOrigin, rpOrigin->getChildRef(branch));
        return rpOrigin;
    }

public:
    CBinaryTree() = default;
    
    CBinaryTree(const CBinaryTree& other) : m_size(other.m_size), Compfn(other.Compfn) {
        m_pRoot = copyTree(other.m_pRoot, nullptr);
    }
    
    CBinaryTree(CBinaryTree&& other)
        : m_pRoot(std::exchange(other.m_pRoot, nullptr)), 
          m_size(std::exchange(other.m_size, 0)), 
          Compfn(std::move(other.Compfn)) {}
    
    virtual ~CBinaryTree() {
        destroyTree(m_pRoot);
        m_pRoot = nullptr;
        m_size = 0;
    } 

    CBinaryTree& operator=(const CBinaryTree& other) {
        if(this != &other) {
            destroyTree(m_pRoot);
            m_pRoot = nullptr;
            m_pRoot = copyTree(other.m_pRoot, nullptr);
            m_size = other.m_size;
            Compfn = other.Compfn;
        }
        return *this;
    }
    
    template <typename Function, typename... Args>
    void inorder(Function func, Args const&... args) {
        inorder_impl(m_pRoot, 0, func, args...);
    }

    // TODO: Generalize this function by using iterators and apply any function
    template <typename Function, typename... Args>
    void inorder_impl(Node* pNode, size_t level, Function func, Args const&... args) {
        if(pNode) {
            inorder_impl(pNode->getChild(0), level + 1, func, args...);
            func(pNode, level, args...);
            inorder_impl(pNode->getChild(1), level + 1, func, args...);
        }
    }

    template <typename Function, typename... Args>
    void preorder(Function func, Args const&... args) {
        preorder_impl(m_pRoot, 0, func, args...);
    }

    // TODO: Generalize this function to apply any function
    template <typename Function, typename... Args>
    void preorder_impl(Node* pNode, size_t level, Function func, Args const&... args) {
        if(pNode) {
            func(pNode, level, args...);
            preorder_impl(pNode->getChild(0), level + 1, func, args...);
            preorder_impl(pNode->getChild(1), level + 1, func, args...);
        }
    }

    template <typename Function, typename... Args>
    void postorder(Function func, Args const&... args) {
        postorder_impl(m_pRoot, 0, func, args...);
    }

    // TODO: generalize this function to apply any function
    template <typename Function, typename... Args>
    void postorder_impl(Node* pNode, size_t level, Function func, Args const&... args) {
        if(pNode) {
            postorder_impl(pNode->getChild(0), level + 1, func, args...);
            postorder_impl(pNode->getChild(1), level + 1, func, args...);
            func(pNode, level, args...);
        }
    }

    template <typename Function, typename... Args>
    void print(Function func, Args const&... args) {
        print_impl(m_pRoot, 0, func, args...);
    }

    template <typename Function, typename... Args>
    void print_impl(Node* pNode, size_t level, Function func, Args const&... args) {
        if(pNode) {
            print_impl(pNode->getChild(1), level + 1, func, args...);
            func(pNode, level, args...);
            print_impl(pNode->getChild(0), level + 1, func, args...);
        }
    }

    void inorder(ostream& os) {
        inorder_impl(m_pRoot, 0, [&os](Node* pNode, size_t level) { 
            os << " --> " << pNode->getDataRef(); 
        });
    }

    void preorder(ostream& os) {
        preorder_impl(m_pRoot, 0, [&os](Node* pNode, size_t level) { 
            os << " --> " << pNode->getDataRef(); 
        });
    }

    // TODO: Generalize this function to apply any function
    void postorder(ostream& os) {
        postorder_impl(m_pRoot, 0, [&os](Node* pNode, size_t level) { 
            os << " --> " << pNode->getDataRef(); 
        });
    }

    // TODO: generalize this function to apply any function
    // Google: C++ parameter packs cplusplus
    void print(ostream& os) {
        if(!m_pRoot) {
            os << "Empty tree" << endl;
            return;
        }
        print_impl(m_pRoot, 0, [&os](Node* pNode, size_t level) {
            if(pNode) {
                Node* pParent = pNode->getParent();
                for(size_t i = 0; i < level; ++i) os << "  ";
                os << pNode->getDataRef() 
                   << "(" << (pParent ? to_string(pParent->getData()) : "Root") << ")" << endl;
            }
        });
    }

    // TODO: Tip: recorrer el arbol en preorden
    void Write(ostream& os) { 
        writePreorder(os, m_pRoot); 
    }

    // TODO: Leer en el arbol desde un stream asumiendo que esta en preorden
    void Read(istream& is) { 
        destroyTree(m_pRoot);
        m_pRoot = nullptr;
        m_size = 0;
        try {
            m_pRoot = readPreorder(is, nullptr);
        } catch(const exception& e) {
            destroyTree(m_pRoot);
            m_pRoot = nullptr;
            m_size = 0;
            throw;
        }
    }

    template <typename Container>
    friend class binary_tree_iterator;
};

// TODO: este operator << debe seguir estando fuera de la clase
template <typename Traits>
ostream& operator<<(ostream& os, CBinaryTree<Traits>& obj) {
    obj.Write(os);
    return os;
}

template <typename Traits>
istream& operator>>(istream& is, CBinaryTree<Traits>& obj) {
    obj.Read(is);
    return is;
}

void DemoBinaryTree();

#endif // __BINARY_TREE_H__