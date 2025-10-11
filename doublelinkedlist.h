#ifndef __DOUBLE_LINKEDLIST_H__
#define __DOUBLE_LINKEDLIST_H__
#include <iostream>
#include <utility>
#include "types.h"
#include "traits.h"
#include "foreach.h"

template <typename Traits>
class DLLNode{
private:
    using    value_type = typename Traits::value_type;
    using    Node       = DLLNode<Traits>;

    // Fields go here
    value_type          m_data;
    Ref                 m_ref;
    Node               *m_pNext = nullptr;
    Node               *m_pPrev = nullptr;

public:
    DLLNode(value_type &elem, Ref ref, Node *pNext = nullptr)
        : m_data(elem), m_ref(ref), m_pNext(pNext){
    }
    value_type   GetData()    { return m_data;     }
    value_type  &GetDataRef() { return m_data;     }
    Ref    GetRef()     { return m_ref;      }
    Node * GetNext()    { return m_pNext;    }
    Node *&GetNextRef() { return m_pNext;    }
    // Diff
    void   SetNext(Node *pNext){    m_pNext = pNext; }

    // Particular para la double LinkedList
    Node * GetPrev()    { return m_pPrev;    }
    Node *&GetPrevRef() { return m_pPrev;    }
    // Diff
    void   SetPrev(Node *pPrev){    m_pPrev = pPrev; }
};

// (DONE) Forward Iterator
template <typename Container>
class forward_double_linkedlist_iterator{
private:
    using Node       = typename Container::Node;
    using iterator   = forward_double_linkedlist_iterator<Container>;

    Container *m_pList = nullptr;
    Node      *m_pNode = nullptr;

public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename Container::value_type;    
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

public:
    forward_double_linkedlist_iterator(Container *pList, Node *pNode)
        : m_pList(pList), m_pNode(pNode){}
    
    forward_double_linkedlist_iterator(iterator &other)
        : m_pList(other.m_pList), m_pNode(other.m_pNode){}   
    
    // Operadores de Acceso
    pointer operator->(){ return &(m_pNode->GetDataRef()); }
    reference operator*(){ return m_pNode->GetDataRef(); }

    // Operadores de Comparacion
    bool operator==(iterator other){ return m_pList == other.m_pList && m_pNode == other.m_pNode; }
    bool operator!=(iterator other){ return !(*this == other);    }

    // Operadores de Incremento
    iterator operator++(){ 
        if(m_pNode) m_pNode = m_pNode->GetNext();
        return *this;
    }
    
    iterator operator++(int){ 
        iterator temp = *this;
        ++(*this);
        return temp;
    }
}; 

// (DONE) Backward Iterator
template <typename Container>
class backward_double_linkedlist_iterator{
private:
    using Node       = typename Container::Node;
    using iterator   = backward_double_linkedlist_iterator<Container>;

    Container *m_pList = nullptr;
    Node      *m_pNode = nullptr;

public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename Container::value_type;    
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;
public:
    backward_double_linkedlist_iterator(Container *pList, Node *pNode)
        : m_pList(pList), m_pNode(pNode){}
    backward_double_linkedlist_iterator(iterator &other)
        : m_pList(other.m_pList), m_pNode(other.m_pNode){}   
    
    // Operadores de Acceso
    pointer operator->(){ return &(m_pNode->GetDataRef()); }
    reference operator*(){ return m_pNode->GetDataRef(); }

    // Operadores de Comparacion
    bool operator==(iterator other){ return m_pList == other.m_pList && m_pNode == other.m_pNode;}
    bool operator!=(iterator other){ return !(*this == other);    }

    // Operadores de Incremento
    iterator operator++(){ 
        if(m_pNode) m_pNode = m_pNode->GetPrev();
        return *this;
    }

    iterator operator++(int){ 
        iterator temp = *this;
        ++(*this);
        return temp;
    }
}; 

// TODO Agregar control de concurrencia

template <typename Traits>
class CDoubleLinkedList{
public:
    using value_type = typename Traits::value_type; 
    using Func       = typename Traits::Func;
    using Node       = DLLNode<Traits>; 
    using Container  = CDoubleLinkedList<Traits>;
    using forward_iterator = forward_double_linkedlist_iterator<Container>;
    using backward_iterator  = backward_double_linkedlist_iterator<Container>;
    
private:
    Node   *m_pRoot = nullptr;
    Node   *m_pTail = nullptr;
    size_t m_nElem = 0;
    Func   m_fCompare;

public:
    // Constructor
    CDoubleLinkedList();
    CDoubleLinkedList(CDoubleLinkedList &other);

    // Move Constructor
    CDoubleLinkedList(CDoubleLinkedList &&other);

    // Destructor seguro
    virtual ~CDoubleLinkedList();

    void Insert(value_type &elem, Ref ref);

private:
    Node *GetRoot()    {    return m_pRoot;     };
    void clear(){
        Node* pCurrent = m_pRoot;
        while (pCurrent) {
            Node* pNext = pCurrent->GetNext();
            delete pCurrent;
            pCurrent = pNext;
        }
        m_pRoot = m_pTail = nullptr;    
        m_nElem = 0;
    }

public:
    forward_iterator begin(){ return forward_iterator(this, m_pRoot); };
    forward_iterator end()  { return forward_iterator(this, nullptr); } 

    backward_iterator rbegin(){ return backward_iterator(this, m_pTail); };
    backward_iterator rend()  { return backward_iterator(this, nullptr); } 

public:
    // (DONE) Write and Read
    std::ostream &Write(std::ostream &os);
    std::istream &Read (std::istream &is);

    // (DONE) Foreach generico en foreach.h
    template <typename Function, typename... Args>
    void foreach(Function func, Args &&... args){
        ::foreach(this->begin(), this->end(), func, std::forward<Args>(args)...);
    }
};

template <typename Traits>
void CDoubleLinkedList<Traits>::Insert(value_type &elem, Ref ref){
    Node* pCurrent = m_pRoot;
    while (pCurrent != nullptr && m_fCompare(pCurrent->GetDataRef(), elem)) {
        pCurrent = pCurrent->GetNext();
    }

    Node* pNew = new Node(elem, ref);
    m_nElem++;

    if (pCurrent == nullptr){
        if (m_pTail) {
            m_pTail->SetNext(pNew);
            pNew->SetPrev(m_pTail);
            m_pTail = pNew;
        } else {
            m_pRoot = m_pTail = pNew; // Lista vacía
        }
    }

    else {
        pNew->SetNext(pCurrent);
        pNew->SetPrev(pCurrent->GetPrev());

        if (pCurrent->GetPrev() == nullptr) { // Es el nuevo inicio de la lista
            m_pRoot = pNew;
        } else { // Está en el medio de la lista
            pCurrent->GetPrev()->SetNext(pNew);
        }
        pCurrent->SetPrev(pNew);
    }
}

template <typename Traits>
CDoubleLinkedList<Traits>::CDoubleLinkedList(){}

// (DONE) Constructor por Copia
template <typename Traits>
CDoubleLinkedList<Traits>::CDoubleLinkedList(CDoubleLinkedList &other)
    : m_pRoot(nullptr), m_pTail(nullptr), m_nElem(0), m_fCompare(other.m_fCompare){
    for (Node *pOther = other.m_pRoot; pOther != nullptr; pOther = pOther->GetNext()){
        this->Insert( pOther->GetDataRef(), pOther->GetRef() );
    }
}

// Move Constructor
template <typename Traits>
CDoubleLinkedList<Traits>::CDoubleLinkedList(CDoubleLinkedList &&other){
    m_pRoot    = std::move(other.m_pRoot);
    m_nElem    = std::move(other.m_nElem);
    m_fCompare = std::move(other.m_fCompare);
}

// (DONE) Implementar destructor seguro
template <typename Traits>
CDoubleLinkedList<Traits>::~CDoubleLinkedList(){
    this->clear();
}

// (DONE) Operador << fuera de la clase.
template <typename Traits>
std::ostream &operator<<(std::ostream &os, CDoubleLinkedList<Traits> &obj){
    for(auto &elem : obj){
        os << elem << "-"; // Asumiendo que elem (value_type) tiene operador << 
    }
    return os;
}

template <typename Traits>
std::ostream& CDoubleLinkedList<Traits>::Write(std::ostream& os) { 
    Node* pCurrent = m_pRoot;
    while (pCurrent) {
        os << pCurrent->GetData() << " " << pCurrent->GetRef() << "\n";
        pCurrent = pCurrent->GetNext();
    }
    return os;
}

template <typename Traits>
std::istream& CDoubleLinkedList<Traits>::Read(std::istream &is){
        this->clear();
        value_type elem;
        Ref ref;

        while (is >> elem >> ref) {
            this->Insert(elem, ref);
        }
        return is;
}

void DemoDoubleLinkedList();

#endif // __DOUBLE_LINKEDLIST_H__