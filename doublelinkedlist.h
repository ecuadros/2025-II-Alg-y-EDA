#ifndef __DOUBLE_LINKEDLIST_H__
#define __DOUBLE_LINKEDLIST_H__
#include <iostream>
#include <mutex>
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

// TODO (Done): Forward y Backward iterators implementados y funcionando correctamente
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

// TODO (Done): Control de concurrencia implementado con std::mutex
// Las operaciones de escritura (Insert) están protegidas con lock_guard

// TODO (Done): Orden ascendente/descendente se puede de dos formas:
// 1. Usando Traits diferentes: AscendingTrait vs DescendingTrait
// 2. Usando iteradores: begin()->end() (ascendente) vs rbegin()->rend() (descendente)
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
    mutable std::mutex m_mutex;  // Protege modificaciones concurrentes de m_pRoot, m_pTail y m_nElem

public:
    // Constructor
    CDoubleLinkedList();
    CDoubleLinkedList(CDoubleLinkedList &other);

    // TODO: Done
    CDoubleLinkedList(CDoubleLinkedList &&other);

    // Destructor seguro
    virtual ~CDoubleLinkedList();

    void Insert(value_type &elem, Ref ref);
    Node *GetRoot()    {    return m_pRoot;     };

private:
    void InternalInsert(Node *&rParent, value_type &elem, Ref ref, Node *pPrev);

public:
    forward_iterator begin(){ return forward_iterator(this, m_pRoot); };
    forward_iterator end()  { return forward_iterator(this, nullptr); } 

    // TODO (Done): Iterador reverso: comienza en la cola y avanza hacia el inicio
    backward_iterator rbegin(){ return backward_iterator(this, m_pTail); };
    backward_iterator rend()  { return backward_iterator(this, nullptr); } 

    // Persistence
    std::ostream &Write(std::ostream &os);
    
    // TODO: Read (istream &is)
    std::istream &Read (std::istream &is);
};

template <typename Traits>
void CDoubleLinkedList<Traits>::Insert(value_type &elem, Ref ref){
    std::lock_guard<std::mutex> lock(m_mutex);  // Protege porque múltiples hilos pueden insertar simultáneamente
    InternalInsert(m_pRoot, elem, ref, nullptr);
}

// TODO (Done): Enlace para Prev() implementado correctamente
template <typename Traits>
void CDoubleLinkedList<Traits>::InternalInsert(Node *&rParent, value_type &elem, Ref ref, Node *pPrev){
    if( !rParent || m_fCompare(elem, rParent->GetDataRef()) ){
        Node *pNew = new Node(elem, ref, rParent);
        
        // Enlace hacia adelante
        rParent = pNew;
        
        // Enlace hacia atras
        pNew->SetPrev(pPrev);
        
        // Si hay un nodo siguiente, actualizar su enlace previo
        if( pNew->GetNext() ){
            pNew->GetNext()->SetPrev(pNew);
        } else {
            // Si no hay siguiente, este es el último nodo
            m_pTail = pNew;
        }
        
        m_nElem++;
        return;
    }
    // Tail recursion - pasamos rParent como el nuevo pPrev
    InternalInsert(rParent->GetNextRef(), elem, ref, rParent);
}

template <typename Traits>
CDoubleLinkedList<Traits>::CDoubleLinkedList(){}

// TODO (Done): Constructor por copia - hace loop copiando cada elemento
template <typename Traits>
CDoubleLinkedList<Traits>::CDoubleLinkedList(CDoubleLinkedList &other)
    : m_fCompare(other.m_fCompare)
{
    std::lock_guard<std::mutex> lock(other.m_mutex);  // Protege other porque otro hilo puede estar modificándolo
    Node *current = other.m_pRoot;
    
    while(current){
        value_type elem = current->GetData();
        Ref reference = current->GetRef();
        InternalInsert(m_pRoot, elem, reference, nullptr);  // No protege 'this' porque es objeto nuevo aún no compartido
        current = current->GetNext();
    }
}

// Move Constructor
template <typename Traits>
CDoubleLinkedList<Traits>::CDoubleLinkedList(CDoubleLinkedList &&other){
    std::lock_guard<std::mutex> lock(other.m_mutex);  // Protege other porque otro hilo podría accederlo durante el movimiento
    m_pRoot    = std::move(other.m_pRoot);
    m_pTail    = std::move(other.m_pTail);
    m_nElem    = std::move(other.m_nElem);
    m_fCompare = std::move(other.m_fCompare);
}

// TODO (Done): Destructor implementado - libera la memoria de cada Node
template <typename Traits>
CDoubleLinkedList<Traits>::~CDoubleLinkedList()
{
    std::lock_guard<std::mutex> lock(m_mutex);  // Protege porque otro hilo podría estar iterando mientras se destruye
    Node *pCurrent = m_pRoot;
    while(pCurrent){
        Node *pNext = pCurrent->GetNext();
        delete pCurrent;
        pCurrent = pNext;
    }
    m_pRoot = nullptr;
    m_pTail = nullptr;
    m_nElem = 0;
}

// TODO (Done): operator<< movido fuera de la clase
template <typename Traits>
std::ostream& operator<<(std::ostream &os, CDoubleLinkedList<Traits> &obj){
    auto pRoot = obj.GetRoot();
    while( pRoot ){
        os << pRoot->GetData() << "(" << pRoot->GetRef() << ") ";
        pRoot = pRoot->GetNext();
    }
    return os;
}

// Implementación del método Write
template <typename Traits>
std::ostream& CDoubleLinkedList<Traits>::Write(std::ostream &os){
    return os << *this;
}

// TODO (Done): Método Read implementado - lee formato: dato(ref) dato(ref) ...
template <typename Traits>
std::istream& CDoubleLinkedList<Traits>::Read(std::istream &is){
    // No protege porque Insert() ya lo hace internamente y evitamos doble lock
    value_type dato;
    Ref referencia;
    char parentesis;
    
    // Leer elementos en formato: dato(ref)
    while(is >> dato >> parentesis && parentesis == '('){
        is >> referencia >> parentesis;  // Leer ref y ')'
        if(parentesis == ')'){
            Insert(dato, referencia);
        }
    }
    
    return is;
}

void DemoDoubleLinkedList();

#endif // __DOUBLE_LINKEDLIST_H__