#ifndef __BINARY_TREE_H__  
#define __BINARY_TREE_H__ 
//#include <utility>
//#include <algorithm>
#include <cassert>
#include <fstream>
#include "types.h"
//#include "util.h"
using namespace std;

/* ----- Declaracion adelantada de BinaryTree ----- */
template <typename Traits>
class CBinaryTree;

/* ----- Nodo del arbol binario ----- */
template <typename Traits>
class CBinaryTreeNode{
public:
    friend class CBinaryTree<Traits>;

    using value_type = typename Traits::T;
    using Node       = CBinaryTreeNode<Traits>;

private:
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

    value_type  getRef()                 {   return m_ref;     }

// (DONE) Friend de BinaryTree. Métodos puedes ser privador.
private: 
    void      setpChild(const Node *pChild, size_t pos)  {   m_pChild[pos] = pChild;  }
    Node    * getChild(size_t branch){ return m_pChild[branch];  }
    Node    *&getChildRef(size_t branch){ return m_pChild[branch];  }
    Node    * getParent() { return m_pParent;   }
};


/* ----- Iterador del árbol binario ----- */
template <typename Container>
class binary_tree_iterator
{  
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type        = typename Container::value_type;
    using difference_type   = std::ptrdiff_t;
    using pointer           = value_type*;
    using reference         = value_type&;

private:
    using Node = typename Container::Node;
    using iterator = binary_tree_iterator<Container>;

    Container *m_pTree;
    Node* m_pNode;
  
public:
    binary_tree_iterator(Container *pTree = nullptr, Node *pNode = nullptr) : m_pTree(pTree), m_pNode(pNode) {}
    binary_tree_iterator(iterator &other)  : m_pTree(other.pTree),  m_pNode(other.m_pNode) {}


public:
    // Operadores de acceso
    reference operator*() { return m_pNode->getDataRef(); }
    pointer   operator->(){ return &(m_pNode->getDataRef()); }

    // (DONE) Iteradores
    // Pre-Incremento (++it)
    binary_tree_iterator operator++() {
        if (!m_pNode) return *this; // Ya terminó
        
        Node* nodoIzquierdo;
        Node* nodoDerecho;

        if (nodoDerecho = m_pNode->getChild(1)) {
            m_pNode = nodoDerecho;
            while (nodoIzquierdo = m_pNode->getChild(0)) {
                m_pNode = nodoIzquierdo;
            }
        } else {
            Node* nodoPadre = m_pNode->getParent();
            while (nodoPadre && m_pNode == nodoPadre->getChild(1)) {
                m_pNode = nodoPadre;
                nodoPadre = padre->getParent();
            }
            m_pNode = nodoPadre;
        }
        return *this;
    }

    // Post-Incremento (it++)
    binary_tree_iterator operator++(int) {
        binary_tree_iterator temp = *this;
        ++(*this);
        return temp;
    }

    // Pre-Decremento (--it)
    binary_tree_iterator operator--() {
        if (!m_pNode) return *this; 

        Node* nodoIzquierdo;
        Node* nodoDerecho;

        if (nodoIzquierdo = m_pNode->getChild(0)) {
            m_pNode = nodoIzquierdo;
            while (nodoDerecho = m_pNode->getChild(1)) {
                m_pNode = nodoDerecho;
            }
        } else {
            Node* nodoPadre = m_pNode->getParent();
            while (nodoPadre && m_pNode == nodoPadre->getChild(0)) {
                m_pNode = nodoPadre;
                nodoPadre = padre->getParent();
            }
            m_pNode = nodoPadre;
        }
        return *this;
    }

    // Post-Decremento (it--)
    binary_tree_iterator operator--(int) {
        binary_tree_iterator temp = *this;
        --(*this);
        return temp;
    }

    // Operadores de Comparación
    bool operator==(binary_tree_iterator other){ 
        return m_pNode == other.m_pNode;
    }

    bool operator!=(binary_tree_iterator other){ 
        return !(*this == other);    
    }
};

/* ----- Traits ----- */
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

/* ----- Arbol Binario ----- */
template <typename Traits>
class CBinaryTree{
public:
    using value_type        = typename Traits::T;
    using Node              = typename Traits::Node;
    
    using CompareFn         = typename Traits::CompareFn;
    using Container         = CBinaryTree<Traits>;
    using iterator          = binary_tree_iterator<Container>;
    using reverse_iterator  = std::reverse_iterator<iterator>;

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
private:
    Node *copy_tree(Node *pNode, Node *pParent) {
        if (!pNode) return nullptr;

        Node* newNode = CreateNode(pParent, pNode->getData(), pNode->getRef());
        newNode->getChildRef(0) = copy_tree(pNode->getChild(0), newNode);
        newNode->getChildRef(1) = copy_tree(pNode->getChild(1), newNode);
        return newNode;
    }

public:
    CBinaryTree(){} // Empty tree
    
    // (DONE) Copy Constructor
    CBinaryTree(Binary &other){
        if (other.m_pRoot) {
            m_pRoot = copy_tree(other.m_pRoot, nullptr);
            m_size  = other.m_size;
            Compfn  = other.Compfn;
        }
    }
    
    // (DONE) Move Constructor
    CBinaryTree(Binary &&other)
        : m_pRoot(std::exchange(other.m_pRoot, nullptr)), 
          m_size (std::exchange(other.m_size, 0)), 
          Compfn (std::exchange(other.Compfn, nullptr))
    { }

    // (DONE) Destructor recursivo y seguro.
    virtual ~CBinaryTree(){
        delete m_pRoot;     // Inicia la destrucción recursiva. Cada Nodo elimina a sus hijos.  
        m_pRoot = nullptr; 
        m_size  = 0;
    } 
    
    // (DONE) Begin, End, RBegin y REnd.
    iterator begin() { 
        if (!m_pRoot) return end();
        return iterator(this, getExtremeNode(m_pRoot, 0));
    }
    
    iterator end(){ 
        return iterator(this, nullptr); 
    }

    reverse_iterator rbegin(){ 
        return reverse_iterator(end());    
    }

    reverse_iterator rend()  { 
        return reverse_iterator(begin());  
    }

    // (DONE) Generalización de recorridos para cualquier funcion usando variadic templates
    // (DONE) Inorder generalizado
    template <typename Function, typename... Args>
    void inorder(Function func , Args&&... args){
        inorder_recursivo(m_pRoot, func, std::forward<Args>(args)...);  
    }

    template <typename Function, typename... Args> 
    void inorder_recursivo(Node  *pNode, Function func, Args&&... args){
        if( pNode ){
            inorder_recursivo(pNode->getChild(0), func, std::forward<Args>(args)...);
            func(pNode, std::forward<Args>(args)...);
            inorder_recursivo(pNode->getChild(1), func, std::forward<Args>(args)...);
        }
    }

    // (DONE) Posorder generalizado
    template <typename Function, typename... Args>
    void postorder(Function func, Args&&... args){
        postorder_recursivo(m_pRoot, func, args...);
    }

    template <typename Function, typename... Args>
    void postorder_recursivo(Node* pNode, Function func, Args&&... args) {
        if (pNode) {
            postorder_recursivo(pNode->getChild(0), func, std::forward<Args>(args)...);
            postorder_recursivo(pNode->getChild(1), func, std::forward<Args>(args)...);
            func(pNode, std::forward<Args>(args)...); 
        }
    }

    // (DONE) Preorden generalizado
    template <typename Function, typename... Args>
    void preorder(Function func, Args&&... args){   
        preorder_recursivo(m_pRoot, func, args...);  
    }
    template <typename Function, typename... Args>
    void preorder_recursivo(Node  *pNode, Function func, Args&&... args){
        if( pNode ){   
            func(pNode, std::forward<Args>(args)...);
            preorder_recursivo(pNode->getChild(0), func, std::forward<Args>(args)...);
            preorder_recursivo(pNode->getChild(1), func, std::forward<Args>(args)...);            
        }
    }

    // (DONE) Imprime la estructura del árbol
    void print(ostream &os){ 
        os << "\nEstructura del árbol binario:\n";
        os << endl;  
        print_recursivo(m_pRoot, 0, os);  
    }

    void print_recursivo(Node  *pNode, size_t level, ostream &os){
        if(!pNode) return;
        pParent = pNode->getParent();

        print_recursivo(pNode->getChild(1), level+1, os);
        
        for (size_t i = 0; i < level; i++){
            os << "    ";    
        }
        
        os << "--->" << pNode->getData() << "(" << (pParent?to_string(pParent->getData()):"Root") << ")" <<endl;

        print_recursivo(pNode->getChild(0), level+1, os);
    }

    // (DONE) Write en preorden
    void Write(ostream &os) {
        write_recursivo(m_pRoot, os);
    }

    void write_recursivo(Node* pNode, ostream &os) {
        if (!pNode){
            os << "# "; // Indica un nodo nulo
            return;
        }
        
        os << pNode->getData() << " " << pNode->getRef() << " ";
        write_recursivo(pNode->getChild(0), os);
        write_recursivo(pNode->getChild(1), os);
    }

    // (DONE) Read en preorden
    void Read(istream &is)  {
        m_pRoot = read_recursivo(is, nullptr);
    }

    Node* read_recursivo(istream &is, Node* pParent) {
        std::string data_string;
        is >> data_string;
        if(!is || data_string == "#") {
            return nullptr;
        }

        Ref ref_val;
        is >> ref_val;

        value_type data_val;
        std::stringstream converter(data_string);
        converter >> data_val;
        
        Node* newNode = CreateNode(pParent, data_val, ref_val);
        m_size++;
        
        newNode->setpChild(read_recursivo(is, newNode), 0);
        newNode->setpChild(read_recursivo(is, newNode), 1);
        
        return newNode;
    }
};

// (DONE) Operador << sigue fuera. Modificado para ver el arbol en inorder usando la funcion anteriormente definida.
template <typename Traits>
ostream & operator<<(std::ostream &os, CBinaryTree<Traits> &obj){
    os << "CBinaryTree with " << obj.size() << " elements.";
    
    // Pasamos a inorder una lambda que imprime el dato dentro del nodo
    obj.inorder([&os](typename CBinaryTree<Traits>::Node* pNode){
        os << pNode->getData() << " ";
    });

    return os;
}

// (DONE) Operador >> para leer el árbol como input.
template <typename Traits>
istream & operator>>(istream &is, CBinaryTree<Traits> &obj){
    obj = CBinaryTree<Traits>(); // Limpiar el árbol actual

    typename CBinaryTree<Traits>::value_type data;
    Ref ref;

    while(is >> data && is >> ref) {
        obj.insert(data, ref);
    }

    is.clear();
    return is;
}

void DemoBinaryTree();

#endif // __BINARY_TREE_H__