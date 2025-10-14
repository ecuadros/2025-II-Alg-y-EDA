#ifndef __DOUBLE_LINKEDLIST_H__
#define __DOUBLE_LINKEDLIST_H__
#include <iostream>
#include "types.h"
#include "traits.h"

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

public:
    // Constructor
    CDoubleLinkedList();
    CDoubleLinkedList(CDoubleLinkedList &other);

    // TODO: Done Move Constructor
    CDoubleLinkedList(CDoubleLinkedList &&other);

    // Destructor seguro
    virtual ~CDoubleLinkedList();

    void Insert(value_type &elem, Ref ref);
private:
    void InternalInsert(Node *&rParent, value_type &elem, Ref ref);    
    Node *GetRoot()    {    return m_pRoot;     };
    void InsertTail(value_type &elem, Ref ref);

public:
    forward_iterator begin(){ return forward_iterator(this, m_pRoot); };
    forward_iterator end()  { return forward_iterator(this, nullptr); } 

    // TODO: Done verifricar donde debe comenzar apuntando el iterator reverso
    backward_iterator rbegin(){ return backward_iterator(this, m_pTail); };
    backward_iterator rend()  { return backward_iterator(this, nullptr); } 

    friend std::ostream& operator<<(std::ostream &os, CDoubleLinkedList<Traits> &obj){
        auto pRoot = obj.GetRoot();
        while( pRoot ){
            os << pRoot->GetData() << "(" << pRoot->GetRef() << ") ";
            pRoot = pRoot->GetNext();
        }
        return os;
    }
public:
    // Persistence
    std::ostream &Write(std::ostream &os) { return os << *this; }
    
    // TODO: Read (istream &is)
    std::istream &Read (std::istream &is);

    // TODO: crear foreach generico aplicando una funcion a cada elemento
    template <typename Function, typename... Args>
    void foreach(Function func, Args const&... args){
        ::foreach(begin(), end(), func, args...);
        // auto iter = begin();
        // for(; iter != end() ; ++iter )
        //     std::invoke(func, *iter, args...);
    }
};

template <typename Traits>
void CDoubleLinkedList<Traits>::Insert(value_type &elem, Ref ref){
    std::lock_guard<std::mutex> lock(m_mutex);
    InternalInsert(m_pRoot, elem, ref);
}

// TODO: Done Agregar el enlace para el Prev()
template <typename Traits>
void CDoubleLinkedList<Traits>::InternalInsert(Node *&rParent, value_type &elem, Ref ref){
    if( !rParent || m_fCompare(elem, rParent->GetDataRef()) ){
        Node *pNew = rParent = new Node(elem, ref, rParent);
        if( !pNew->GetNext() ) // Final de la lista
            m_pTail = pNew;

        // Puente hacia atras
        Node *pNext = pNew->GetNext();
        if( pNext ){ // Hay algo a continuacion
            pNew ->SetPrev( pNext->GetPrev() );
            pNext->SetPrev( pNew ); 
        }
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
    m_fCompare = other.m_fCompare;

    Node *node = other.m_pRoot;
    while(node) {
        InsertTail(node->GetDataRef(), node->GetRef());
        node = node->GetNext();
    }
}

template <typename Traits>
void CDoubleLinkedList<Traits>::InsertTail(value_type &elem, Ref ref) {
    Node *newNode = new Node(elem, ref, nullptr);
    
    if (!m_pTail) {
        m_pRoot = newNode;
        m_pTail = newNode;
    } else {
        m_pTail->SetNext(newNode);
        newNode->SetPrev(m_pTail);
        m_pTail = newNode;
    }
    m_nElem++;
}

// Move Constructor
template <typename Traits>
CDoubleLinkedList<Traits>::CDoubleLinkedList(CDoubleLinkedList &&other){
    m_pRoot    = std::move(other.m_pRoot);
    m_nElem    = std::move(other.m_nElem);
    m_fCompare = std::move(other.m_fCompare);
}

// TODO: Implementar y liberar la memoria de cada Node
template <typename Traits>
CDoubleLinkedList<Traits>::~CDoubleLinkedList()
{
}

// TODO: Este operador debe quedar fuera de la clase
// template <typename Traits>
// std::ostream &operator<<(std::ostream &os, CDoubleLinkedList<Traits> &obj){
//     auto pRoot = obj.GetRoot();
//     while( pRoot )
//         os << pRoot->GetData() << " ";
//     return os;
// }

void DemoDoubleLinkedList();

#endif // __DOUBLE_LINKEDLIST_H__