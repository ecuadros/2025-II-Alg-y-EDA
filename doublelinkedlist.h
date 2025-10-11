#ifndef __DOUBLE_LINKEDLIST_H__
#define __DOUBLE_LINKEDLIST_H__
#include <iostream>
#include "types.h"
#include "traits.h"
#include <mutex>  
using namespace std;

template <typename Traits>
class DLLNode{
private:
    using value_type = typename Traits::value_type;
    using Node       = DLLNode<Traits>;

    // Fields go here
    value_type  m_data;
    Ref         m_ref;
    Node       *m_pNext = nullptr;
    Node       *m_pPrev = nullptr;

public:
    DLLNode(value_type &elem, Ref ref, Node *pNext = nullptr)
        : m_data(elem), m_ref(ref), m_pNext(pNext){
    }

    value_type   GetData()    { return m_data;     }
    value_type  &GetDataRef() { return m_data;     }
    Ref          GetRef()     { return m_ref;      }
    Node        *GetNext()    { return m_pNext;    }
    Node        *&GetNextRef(){ return m_pNext;    }
    void         SetNext(Node *pNext){ m_pNext = pNext; }

    // Particular para la double LinkedList
    Node * GetPrev()    { return m_pPrev;    }
    Node *&GetPrevRef() { return m_pPrev;    }
    void   SetPrev(Node *pPrev){ m_pPrev = pPrev; }
};

//
// TODO Activar el forward_iterator
template <typename Container>
class forward_double_linkedlist_iterator{
 private:
     using value_type = typename Container::value_type;
     using Node       = typename Container::Node;
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

     iterator operator++(){ 
         if(m_pNode)
             m_pNode = m_pNode->GetNext();
         return *this;
     }
     value_type &operator*(){ return m_pNode->GetDataRef(); }
};

template <typename Container>
class backward_double_linkedlist_iterator{
 private:
     using value_type = typename Container::value_type;
     using Node       = typename Container::Node;
     using iterator   = backward_double_linkedlist_iterator<Container>;

     Container *m_pList = nullptr;
     Node      *m_pNode = nullptr;
 public:
     backward_double_linkedlist_iterator(Container *pList, Node *pNode)
             : m_pList(pList), m_pNode(pNode){}
     backward_double_linkedlist_iterator(iterator &other)
             : m_pList(other.m_pList), m_pNode(other.m_pNode){}   
     bool operator==(iterator other){ return m_pList == other.m_pList && m_pNode == other.m_pNode; }
     bool operator!=(iterator other){ return !(*this == other); }

     iterator operator++(){ 
         if(m_pNode)
             m_pNode = m_pNode->GetPrev();
         return *this;
     }
     value_type &operator*(){ return m_pNode->GetDataRef(); }
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
    size_t  m_nElem = 0;
    Func    m_fCompare;
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
    Node *GetRoot()    { return m_pRoot; };

public:
    forward_iterator begin(){ return forward_iterator(this, m_pRoot); };
    forward_iterator end()  { return forward_iterator(this, nullptr); } 

    // TODO: verificar donde debe comenzar apuntando el iterator reverso
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
};

template <typename Traits>
void CDoubleLinkedList<Traits>::Insert(value_type &elem, Ref ref){
    std::lock_guard<std::mutex> lock(m_mutex);
    InternalInsert(m_pRoot, elem, ref);

}

//  TODO: Agregar el enlace para el Prev()
template <typename Traits>
void CDoubleLinkedList<Traits>::InternalInsert(Node *&rParent, value_type &elem, Ref ref){
    // Si la lista está vacía o insertamos al principio
    if(!rParent){
        Node *pNew = new Node(elem, ref, nullptr);
        pNew->SetPrev(nullptr);
        
        if(!m_pRoot){
            // Lista completamente vacía
            m_pRoot = m_pTail = pNew;
        } else {
            // Insertar al final (caso recursivo llegó al final)
            if(m_pTail){
                m_pTail->SetNext(pNew);
                pNew->SetPrev(m_pTail);
            }
            m_pTail = pNew;
        }
        rParent = pNew;
        m_nElem++;
        return;
    }

    // Si el nuevo elemento debe ir antes del actual
    if(m_fCompare(elem, rParent->GetDataRef())){
        Node *pNew = new Node(elem, ref, rParent);
        Node *pPrev = rParent->GetPrev();
        
        pNew->SetPrev(pPrev);
        pNew->SetNext(rParent);
        rParent->SetPrev(pNew);
        
        if(pPrev){
            pPrev->SetNext(pNew);
        } else {
            // Insertando al inicio
            m_pRoot = pNew;
        }
        
        rParent = pNew;  // ✅ CRÍTICO: actualizar la referencia
        m_nElem++;
        return;
    }

    // Recursión al siguiente nodo
    InternalInsert(rParent->GetNextRef(), elem, ref);
}
template <typename Traits>
CDoubleLinkedList<Traits>::CDoubleLinkedList(){}

// TODO Constructor por copia
template <typename Traits>
CDoubleLinkedList<Traits>::CDoubleLinkedList(CDoubleLinkedList &other){
    Node *pOther = other.m_pRoot;
    Node *pPrev = nullptr;
    while(pOther){
        Node *pNew = new Node(pOther->GetDataRef(), pOther->GetRef());
        if(!m_pRoot)
            m_pRoot = pNew;
        if(pPrev)
            pPrev->SetNext(pNew);
        pNew->SetPrev(pPrev);
        pPrev = pNew;
        pOther = pOther->GetNext();
        m_nElem++;
    }
    m_pTail = pPrev;
    m_fCompare = other.m_fCompare;
}
// Move Constructor
template <typename Traits>
CDoubleLinkedList<Traits>::CDoubleLinkedList(CDoubleLinkedList &&other){
    m_pRoot    = std::move(other.m_pRoot);
    m_pTail    = std::move(other.m_pTail);
    m_nElem    = std::move(other.m_nElem);
    m_fCompare = std::move(other.m_fCompare);

    // Limpiamos el otro objeto
    other.m_pRoot = other.m_pTail = nullptr;
    other.m_nElem = 0;
}

//  TODO: Implementar y liberar la memoria de cada Node
template <typename Traits>
CDoubleLinkedList<Traits>::~CDoubleLinkedList(){
    Node *current = m_pRoot;
    while(current){
        Node *next = current->GetNext();
        delete current;
        current = next;
    }
    m_pRoot = m_pTail = nullptr;
    m_nElem = 0;
}


// TODO: Read (istream &is)
template <typename Traits>
std::istream &CDoubleLinkedList<Traits>::Read(std::istream &is){
    value_type val;
    Ref ref;
    while(is >> val >> ref){
        Insert(val, ref);
    }
    return is;
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
