#ifndef __DOUBLE_LINKEDLIST_H__
#define __DOUBLE_LINKEDLIST_H__
#include <iostream>
#include "types.h"
#include "traits.h"
#include <mutex>

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

// 
// TODO Activar el forward_iterator
template <typename Container>
class forward_double_linkedlist_iterator{
 private:
     using value_type = typename Container::value_type;
     using Node       = typename Container::Node;
     // Diff
     using iterator   = forward_double_linkedlist_iterator<Container>;

     Container *m_pList = nullptr;
     Node      *m_pNode = nullptr;
 public:
     forward_double_linkedlist_iterator(Container *pList, Node *pNode)
             : m_pList(pList), m_pNode(pNode){}
     forward_double_linkedlist_iterator(iterator &other)
             : m_pList(other.m_pList), m_pNode(other.m_pNode){}   
     bool operator==(iterator other){ return m_pList == other.m_pList && m_pNode == other.m_pNode; }
     bool operator!=(iterator other){ return !(*this == other);    }

     // Diff
     iterator operator++(){ 
         if(m_pNode)
             m_pNode = m_pNode->GetNext();
         return *this;
     }
     value_type &operator*(){    return m_pNode->GetDataRef();   }
};

template <typename Container>
class backward_double_linkedlist_iterator{
 private:
     using value_type = typename Container::value_type;
     using Node       = typename Container::Node;
     // Diff
     using iterator   = backward_double_linkedlist_iterator<Container>;

     Container *m_pList = nullptr;
     Node      *m_pNode = nullptr;
 public:
     backward_double_linkedlist_iterator(Container *pList, Node *pNode)
             : m_pList(pList), m_pNode(pNode){}
     backward_double_linkedlist_iterator(iterator &other)
             : m_pList(other.m_pList), m_pNode(other.m_pNode){}   
     bool operator==(iterator other){ return m_pList == other.m_pList && 
                                             m_pNode == other.m_pNode;
                                    }
     bool operator!=(iterator other){ return !(*this == other);    }

     // Diff
     iterator operator++(){ 
         if(m_pNode)
             m_pNode = m_pNode->GetPrev();
         return *this;
     }
     value_type &operator*(){    return m_pNode->GetDataRef();   }
};

// TODO Agregar control de concurrencia

// TODO Agregar que sea ascendente o descendente con el mismo codigo
template <typename Traits>
class CDoubleLinkedList{
public:
    using value_type = typename Traits::value_type; 
    using Func       = typename Traits::Func;
    using Node       = DLLNode<Traits>; 
    using Container  = CDoubleLinkedList<Traits>;
    using forward_iterator   = forward_double_linkedlist_iterator<Container>;
    using backward_iterator  = backward_double_linkedlist_iterator<Container>;
    
private:
    Node   *m_pRoot = nullptr;
    Node   *m_pTail = nullptr;
    size_t m_nElem = 0;
    Func   m_fCompare;
    mutable std::mutex m_mutex; 

public:
    // Constructor
    CDoubleLinkedList();
    CDoubleLinkedList(CDoubleLinkedList &other);

    // TODO: Done
    CDoubleLinkedList(CDoubleLinkedList &&other);

    // Destructor seguro
    virtual ~CDoubleLinkedList();

    void Insert(value_type &elem, Ref ref);
private:
    void InternalInsert(Node *&rParent, value_type &elem, Ref ref);
    Node *GetRoot()    {    return m_pRoot;     };

public:
    forward_iterator begin(){ 
        std::lock_guard<std::mutex> lock(m_mutex); 
        return forward_iterator(this, m_pRoot); };
    forward_iterator end()  { 
        std::lock_guard<std::mutex> lock(m_mutex); 
        return forward_iterator(this, nullptr); } 

    // TODO: verificar donde debe comenzar apuntando el iterator reverso
    backward_iterator rbegin(){ 
        std::lock_guard<std::mutex> lock(m_mutex); 
        return backward_iterator(this, m_pTail); };
    backward_iterator rend()  { 
        std::lock_guard<std::mutex> lock(m_mutex); 
        return backward_iterator(this, nullptr); } 
    
    std::ostream& PrintTo(std::ostream &os) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto pRoot = GetRoot();
        while( pRoot ){
            os << pRoot->GetData() << "(" << pRoot->GetRef() << ") ";
            pRoot = pRoot->GetNext();
        }
        return os;
    }
    /*
    friend std::ostream& operator<<(std::ostream &os, CDoubleLinkedList<Traits> &obj){
        auto pRoot = obj.GetRoot();
        while( pRoot ){
            os << pRoot->GetData() << "(" << pRoot->GetRef() << ") ";
            pRoot = pRoot->GetNext();
        }
        return os;
    }
    */
public:
    // Persistence
    std::ostream &Write(std::ostream &os) { 
        os << m_nElem << std::endl;

        auto pRoot = GetRoot();
        while( pRoot ){
            os << pRoot->GetData() << " " << pRoot->GetRef() << std::endl;
            pRoot = pRoot->GetNext();
        }
        
        return os << *this; }
    
    // TODO: Read (istream &is)
    std::istream &Read (std::istream &is);
private:
    void DestroyNode(Node *pNode){
        if( pNode ){ // Recursion until the final node
            DestroyNode(pNode->GetNext()); 
            delete pNode;
        }
    }
};

template <typename Traits>
void CDoubleLinkedList<Traits>::Insert(value_type &elem, Ref ref){
    std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex for thread safety
    InternalInsert(m_pRoot, elem, ref);
}

// TODO: Agregar el enlace para el Prev()
template <typename Traits>
void CDoubleLinkedList<Traits>::InternalInsert(Node *&rParent, value_type &elem, Ref ref){
    if( !rParent || m_fCompare(elem, rParent->GetDataRef()) ){

        Node *pNew = new Node(elem, ref, rParent);
        
        if( rParent){ 
            pNew->SetPrev( rParent->GetPrev() ); // Nuevo hereda el anterior del parent
            rParent->SetPrev( pNew );            // Parent apunta al nuevo como anterior

            if ( pNew->GetPrev() ){ 
                pNew->GetPrev()->SetNext( pNew ); // El anterior apunta al nuevo
            } else {
                m_pRoot = pNew; // Si no hay anterior, nuevo es la raíz
            }
        } else {
            pNew->SetPrev( m_pTail ); // Nuevo apunta al tail actual
            if (m_pTail) {
                m_pTail->SetNext(pNew); // Tail actual apunta al nuevo
            } else {
                m_pRoot = pNew; // Lista vacía - nuevo es raíz
            }
            m_pTail = pNew; // Nuevo se convierte en tail

        }
        rParent = pNew;
        m_nElem++;
        return;
    }
    // Tail recursion
    InternalInsert(rParent->GetNextRef(), elem, ref);
}

template <typename Traits>
CDoubleLinkedList<Traits>::CDoubleLinkedList(){}

// TODO Constructor por copia
//      Hacer loop copiando cada elemento
template <typename Traits>
CDoubleLinkedList<Traits>::CDoubleLinkedList(CDoubleLinkedList &other){
    Node *pNode = other.m_pRoot; 
    while(pNode){
        value_type data = pNode->GetData();
        Ref ref = pNode->GetRef();
        Insert(data, ref);
        pNode = pNode->GetNext();
    }
}

// Move Constructor
template <typename Traits>
CDoubleLinkedList<Traits>::CDoubleLinkedList(CDoubleLinkedList &&other){
    m_pRoot    = std::move(other.m_pRoot);
    m_nElem    = std::move(other.m_nElem);
    m_fCompare = std::move(other.m_fCompare);
    other.m_pRoot = nullptr;
    other.m_pTail = nullptr;
    other.m_nElem = 0;
}

// TODO: Implementar y liberar la memoria de cada Node
template <typename Traits>
CDoubleLinkedList<Traits>::~CDoubleLinkedList()
{
    DestroyNode(m_pRoot);
    m_pRoot = nullptr;
    m_pTail = nullptr;
    m_nElem = 0;
}

// TODO: Este operador debe quedar fuera de la clase
// template <typename Traits>
// std::ostream &operator<<(std::ostream &os, CDoubleLinkedList<Traits> &obj){
//     auto pRoot = obj.GetRoot();
//     while( pRoot )
//         os << pRoot->GetData() << " ";
//     return os;
// }

template <typename Traits>
std::ostream &operator<<(std::ostream &os, CDoubleLinkedList<Traits> &obj){
    return obj.PrintTo(os);
}


// TODO: Implementar Read
template <typename Traits>
std::istream &CDoubleLinkedList<Traits>::Read(std::istream &is)
{
    DestroyNode(m_pRoot);
    m_pRoot = nullptr;
    m_pTail = nullptr;
    m_nElem = 0;

    size_t count;
    is >> count;
    if(!is) return is;

    for(size_t i = 0 ; i < count; ++i){
        value_type data;
        Ref ref;
        is >> data >> ref;
        if(!is) break;
        Insert(data, ref);
    }
    return is;
}

void DemoDoubleLinkedList();

#endif // __DOUBLE_LINKEDLIST_H__