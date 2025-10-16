#ifndef __BINARY_TREE_H__  
#define __BINARY_TREE_H__ 
#include <algorithm>
#include <cassert>
#include <vector>
#include <fstream>
#include <string>
#include <functional> 
#include "types.h"
using namespace std;

// Nodo del árbol binario - miembros públicos para que AVL pueda acceder
template <typename Traits>
class CBinaryTreeNode{
public:
  using value_type = typename Traits::T;
  using Node       = CBinaryTreeNode<Traits>;

    value_type       m_data;
    Node *           m_pParent = nullptr;
    Ref              m_ref;
    vector<Node *>   m_pChild = {nullptr, nullptr};

public:
    CBinaryTreeNode(Node* pParent, value_type data, Ref ref, Node* p0 = nullptr, Node* p1 = nullptr)
        : m_data(data), m_pParent(pParent), m_ref(ref)
    {
        m_pChild[0] = p0;
        m_pChild[1] = p1;
    }
    ~CBinaryTreeNode(){
        delete m_pChild[0]; m_pChild[0] = nullptr;
        delete m_pChild[1]; m_pChild[1] = nullptr;
    }

    value_type  getData()      { return m_data; }
    value_type &getDataRef()   { return m_data; }

public:
    void      setpChild(const Node *pChild, size_t pos)  { m_pChild[pos] = pChild; }
    Node    * getChild(size_t branch) const { return m_pChild[branch]; }
    Node    *&getChildRef(size_t branch)    { return m_pChild[branch]; }
    Node    * getParent() const             { return m_pParent; }
};

// Iterador bidireccional para recorrido inorder
template <typename Container>
class binary_tree_iterator {
public:
    using Node = typename Container::Node;
    using value_type = typename Container::value_type;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;
    
protected:
    Container* m_pContainer = nullptr;
    Node* m_pNode = nullptr;
    
public:
    binary_tree_iterator() : m_pContainer(nullptr), m_pNode(nullptr) {}

    binary_tree_iterator(Container* pContainer, Node* pNode) 
        : m_pContainer(pContainer), m_pNode(pNode) {}
    
    binary_tree_iterator(const binary_tree_iterator& other) 
        : m_pContainer(other.m_pContainer), m_pNode(other.m_pNode) {}

    binary_tree_iterator& operator=(const binary_tree_iterator& other) {
        if (this != &other) {
            m_pContainer = other.m_pContainer;
            m_pNode = other.m_pNode;
        }
        return *this;
    }

    value_type& operator*() { return m_pNode->getDataRef(); }
    value_type* operator->() { return &(m_pNode->getDataRef()); }
   
    // Avanzar al siguiente nodo en inorder (izq -> raíz -> der)
    binary_tree_iterator& operator++() {
        if (!m_pNode) return *this;
        
        // Si tiene hijo derecho, ir al más a la izquierda del subárbol derecho
        if (m_pNode->getChild(1)) {
            m_pNode = m_pNode->getChild(1);
            while (m_pNode->getChild(0)) {
                m_pNode = m_pNode->getChild(0);
            }
        } else { 
            // Sino, subir hasta encontrar un ancestro del cual seamos hijo izquierdo
            Node* pPadre = m_pNode->getParent();
            while (pPadre && m_pNode == pPadre->getChild(1)) {
                m_pNode = pPadre;
                pPadre = pPadre->getParent();
            }
            m_pNode = pPadre;
        }
        return *this;
    }

    // Retroceder al anterior en inorder (es el espejo de operator++)
    binary_tree_iterator& operator--() {
        if (!m_pNode) {
            // Caso especial end(): ir al nodo más a la derecha
            if (m_pContainer && !m_pContainer->empty()) {
                m_pNode = m_pContainer->m_pRoot;
                while (m_pNode->getChild(1)) {
                    m_pNode = m_pNode->getChild(1);
                }
            }
            return *this;
        }

        // Si tiene hijo izquierdo, ir al más a la derecha del subárbol izquierdo
        if (m_pNode->getChild(0)) {
            m_pNode = m_pNode->getChild(0);
            while (m_pNode->getChild(1)) {
                m_pNode = m_pNode->getChild(1);
            }
        } else {
            // Sino, subir hasta encontrar un ancestro del cual seamos hijo derecho
            Node* pPadre = m_pNode->getParent();
            while (pPadre && m_pNode == pPadre->getChild(0)) {
                m_pNode = pPadre;
                pPadre = pPadre->getParent();
            }
            m_pNode = pPadre;
        }
        return *this;
    }

    bool operator==(const binary_tree_iterator& other) const {
        return m_pNode == other.m_pNode;
    }
    bool operator!=(const binary_tree_iterator& other) const {
        return m_pNode != other.m_pNode;
    }
};

// Traits para árbol ascendente (menor a la izquierda)
template <typename _T>
struct BinaryTreeAscTraits{
    using  T         = _T;
    using  Node      = CBinaryTreeNode<BinaryTreeAscTraits<_T>>;
    using  CompareFn = less<T>;
};

// Traits para árbol descendente (mayor a la izquierda)
template <typename _T>
struct BinaryTreeDescTraits
{
    using  T         = _T;
    using  Node      = CBinaryTreeNode<BinaryTreeAscTraits<_T>>;
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
    using reverse_iterator = std::reverse_iterator<iterator>;  // Usa el iterador estándar de C++

    friend class binary_tree_iterator<Container>;

protected:
    Node    *m_pRoot = nullptr;
    size_t   m_size  = 0;
    CompareFn Compfn;
public: 
    size_t  size()  const       { return m_size; }
    bool    empty() const       { return size() == 0; }
    
    void insert(value_type elem, Ref ref) {
        m_pRoot = internal_insert(elem, ref, nullptr, m_pRoot);
    }

    // Iteradores forward (inorder: izq -> raíz -> der)
    iterator begin() {
        if (!m_pRoot) return end();
        Node* pNode = m_pRoot;
        while (pNode->getChild(0)) {
            pNode = pNode->getChild(0);
        }
        return iterator(this, pNode);
    }
    
    iterator end() {
        return iterator(this, nullptr);
    }

    // Iteradores reverse (usa std::reverse_iterator - uno es el reverso del otro)
    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }
    
    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

protected:
    // Virtual para que AVL pueda crear sus propios nodos si quiere
    virtual Node* CreateNode(Node* pParent, value_type elem, Ref ref) {
        return new Node(pParent, elem, ref);
    }
    
    // Virtual para que AVL pueda agregar lógica de balanceo
    virtual Node* internal_insert(value_type elem, Ref ref,
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
    CBinaryTree(){}
    
    CBinaryTree(CBinaryTree &other){
        m_pRoot = nullptr; 
        m_size  = 0; 
        Compfn  = other.Compfn;
        if(other.m_pRoot){
            m_pRoot = copyNode(other.m_pRoot, nullptr);
            m_size = other.m_size;
        }
    };
    
    CBinaryTree(CBinaryTree&& other) noexcept  
        : m_pRoot(nullptr), m_size(0), Compfn()
    {
        swap(m_pRoot, other.m_pRoot);
        swap(m_size, other.m_size);
        swap(Compfn, other.Compfn);
    } 

    virtual ~CBinaryTree(){
        clear();
    } 
    
    // Método conveniente para imprimir la estructura del árbol
    void print(ostream &os) { 
        print([&os](Node* pNode, size_t level) {
            Node *pParent = pNode->getParent();
            for(size_t i = 0; i < level; ++i) os << "   ";
            os << pNode->getDataRef() 
               << "(" << (pParent ? to_string(pParent->getData()) : "Root") << ")" << endl;
        });
    }

    // Métodos convenientes - usan variadic templates internamente
    void inorder(ostream &os) {
        inorder([&os](value_type val, size_t) { os << val << " --> "; });
    }
    
    void preorder(ostream &os) {
        preorder([&os](value_type val, size_t) { os << val << " --> "; });
    }
    
    void postorder(ostream &os) {
        postorder([&os](auto* node, size_t) { os << node->getDataRef() << " --> "; });
    }

    // Recorridos generalizados usando variadic templates - puedes pasar cualquier lambda
    template <typename Function, typename... Args> 
    void inorder(Function func, Args const&... args){ 
        inorder_variadic(m_pRoot, 0, func, args...);
    }

    template <typename Function, typename... Args>
    void postorder(Function func, Args const&... args){    
        postorder_variadic(m_pRoot, 0, func, args...);
    }

    template <typename Function, typename... Args>
    void preorder(Function func, Args const&... args){
        preorder_variadic(m_pRoot, 0, func, args...);
    }

    // Print generalizado - también usa variadic templates
    template <typename Function, typename... Args>
    void print(Function func, Args const&... args){
        print_variadic(m_pRoot, 0, func, args...);
    }    

    void Write(ostream &os) { os << *this; }
    void Read(istream &is)  { /* TODO */ }

private:
    // Copia recursiva del árbol
    Node* copyNode(Node* sourceNode, Node* parentNode){
        if(!sourceNode) return nullptr;
        Node* newNode = CreateNode(parentNode, sourceNode->getDataRef(), sourceNode->m_ref);
        newNode->getChildRef(0) = copyNode(sourceNode->getChild(0), newNode);
        newNode->getChildRef(1) = copyNode(sourceNode->getChild(1), newNode);
        return newNode;
    }
    
    void clear(){
        delete m_pRoot;
        m_pRoot = nullptr;
        m_size = 0;
    }
    
    // Helpers privados para los recorridos generalizados
    template <typename Function, typename... Args>
    void inorder_variadic(Node  *pNode, size_t level, Function func, Args const&... args){
        if( pNode ){
            inorder_variadic(pNode->getChild(0), level+1, func, args...);
            func(pNode->getDataRef(), level, args...);
            inorder_variadic(pNode->getChild(1), level+1, func, args...);
        }
    }
    
    template <typename Function,typename... Args>
    void postorder_variadic(Node* pNode, size_t level, Function func, Args const&... args) {
        if (pNode) {
            postorder_variadic(pNode->getChild(0), level + 1, func, args...);
            postorder_variadic(pNode->getChild(1), level + 1, func, args...);
            func(pNode, level, args...);  // Postorder recibe el nodo completo
        }
    }
    
    template <typename Function, typename... Args>
    void preorder_variadic(Node  *pNode, size_t level, Function func, Args const&... args){
        if( pNode ){   
            func(pNode->getDataRef(), level, args...);
            preorder_variadic(pNode->getChild(0), level+1, func, args...);
            preorder_variadic(pNode->getChild(1), level+1, func, args...);
        }
    }

    // Print visita: derecha -> nodo -> izquierda (para visualizar horizontalmente)
    template <typename Function, typename... Args>
    void print_variadic(Node *pNode, size_t level, Function func, Args const&... args){
        if( pNode ){
            print_variadic(pNode->getChild(1), level+1, func, args...);
            func(pNode, level, args...);
            print_variadic(pNode->getChild(0), level+1, func, args...);
        }
    }
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