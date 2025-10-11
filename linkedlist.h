#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__
#include <iostream>
#include "types.h"
#include "traits.h"

// Nodo base de la Lista Enlazada
template <typename Traits>
class LLNode{
private:
    using    value_type = typename Traits::value_type;
    using    Node       = LLNode<Traits>;

    // Fields go here
    value_type          m_data;
    Ref                 m_ref;
    Node               *m_pNext = nullptr;

public:
    LLNode(value_type &elem, Ref ref, Node *pNext = nullptr)
        : m_data(elem), m_ref(ref), m_pNext(pNext){
    }
    value_type   GetData()    { return m_data;     }
    value_type  &GetDataRef() { return m_data;     }
    Ref    GetRef()     { return m_ref;      }
    Node * GetNext()    { return m_pNext;    }
    Node *&GetNextRef() { return m_pNext;    }
};


// (DONE) Activar el forward_iterator
template <typename Container>
class forward_linkedlist_iterator{
private:
    using Node       = typename Container::Node;
    using forward_iterator   = forward_linkedlist_iterator<Container>;

    Container *m_pList = nullptr;
    Node      *m_pNode = nullptr;

public:
    // Iterator traits
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename Container::value_type;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    // CONSTRUCTORES
    forward_linkedlist_iterator(Container *pList, Node *pNode) 
        : m_pList(pList), m_pNode(pNode){}
     
    forward_linkedlist_iterator(forward_iterator &other)
        : m_pList(other.m_pList), m_pNode(other.m_pNode){}   
    
    // ACCESORES
    pointer operator->(){ return &(m_pNode->GetDataRef()); }

    reference operator*(){ return m_pNode->GetDataRef(); }
    
    // COMPARACIONES
    // Igual
    bool operator==(forward_iterator other){ 
        return m_pList == other.m_pList && m_pNode == other.m_pNode;
    }
    // Distinto
    bool operator!=(forward_iterator other){ return !(*this == other);    }

    // INCREMENTOS
    // Pre-Incremento 
    forward_iterator operator++(){ 
         if(m_pNode)
             m_pNode = m_pNode->GetNext();
         return *this;
    }
    // Post-Incremento
    forward_iterator operator++(int){ 
         forward_iterator temp = *this;
         ++(*this);
         return temp;
    }

};

// TODO Agregar control de concurrencia

// (DONE) Agregar que sea ascendente o descendente con el mismo codigo
template <typename Traits>
class CLinkedList{
public:
    using value_type         = typename Traits::value_type; 
    using Func               = typename Traits::Func;
    using Node               = LLNode<Traits>; 
    using Container          = CLinkedList<Traits>;
    using forward_iterator   = forward_linkedlist_iterator<Container>;

private:
    Node   *m_pRoot = nullptr;
    size_t m_nElem = 0;
    Func   m_fCompare;

private:
    void clear(){
        Node *pCurrent = m_pRoot;
        while( pCurrent ){
            Node *pNext = pCurrent->GetNext();
            delete pCurrent;
            pCurrent = pNext;
        }
        m_pRoot = nullptr;
        m_nElem = 0;
    }

public:
    // Constructor
    CLinkedList();
    CLinkedList(CLinkedList &other);

    // (DONE) Implementar el Move Constructor
    CLinkedList(CLinkedList &&other);

    // Destructor seguro
    virtual ~CLinkedList();

    void Insert(value_type &elem, Ref ref);

    private:
    void InternalInsert(Node *&rParent, value_type &elem, Ref ref);
    Node *GetRoot()    {    return m_pRoot;     };

public:
    forward_iterator begin(){ return forward_iterator(this, m_pRoot); };
    forward_iterator end()  { return forward_iterator(this, nullptr); } 


public:
    // Modificamos Write para que su uso no dependa de << 
    // Y sea facilmente legible para Read
    std::ostream &Write(std::ostream &os) {
        Node *pCurrent = m_pRoot;
        while (pCurrent != nullptr) {
            os << pCurrent->GetData() << " " << pCurrent->GetRef() << "\n";
            pCurrent = pCurrent->GetNext();
        }
        return os;
    }
    
    // (DONE): Read (istream &is)
    std::istream &Read (std::istream &is){
        this->clear();
        value_type elem;
        Ref ref;

        while (is >> elem >> ref) {
            this->Insert(elem, ref);
        }
        return is;
    }
};

template <typename Traits>
CLinkedList<Traits>::CLinkedList() {}

template <typename Traits>
void CLinkedList<Traits>::Insert(value_type &elem, Ref ref){
    InternalInsert(m_pRoot, elem, ref);
}

template <typename Traits>
void CLinkedList<Traits>::InternalInsert(Node *&rParent, value_type &elem, Ref ref){
    if( !rParent || m_fCompare(elem, rParent->GetDataRef()) ){
        rParent = new Node(elem, ref, rParent);
        m_nElem++;
        return;
    }
    // Tail recursion
    InternalInsert(rParent->GetNextRef(), elem, ref);
}

// (DONE) Constructor por copia
template <typename Traits>
CLinkedList<Traits>::CLinkedList(CLinkedList<Traits> &other)
    : m_pRoot(nullptr), m_nElem(0), m_fCompare(other.m_fCompare)
{
    for (Node *pOther = other.m_pRoot; pOther != nullptr; pOther = pOther->GetNext()){
        Insert( pOther->GetDataRef(), pOther->GetRef() );
    }
}

// Move Constructor
template <typename Traits>
CLinkedList<Traits>::CLinkedList(CLinkedList &&other){
    m_pRoot    = std::move(other.m_pRoot);
    m_nElem    = std::move(other.m_nElem);
    m_fCompare = std::move(other.m_fCompare);
}

// (DONE) Implementar y liberar la memoria de cada Node
template <typename Traits>
CLinkedList<Traits>::~CLinkedList()
{
    this->clear();
}

// (DONE) Este operador debe quedar fuera de la clase
template <typename Traits>
std::ostream &operator<<(std::ostream &os, CLinkedList<Traits> &obj){
    for(auto& elem : obj){
        os << elem << " "; // Asumiendo que elem (value_type) tiene operador << 
    }
    return os;
}


void DemoLinkedList();

#endif // __LINKEDLIST_H__