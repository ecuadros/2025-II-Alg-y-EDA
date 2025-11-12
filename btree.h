/**
 * @file btree.h
 * @brief Implementación de un B-Tree en C++ con soporte para lectura/escritura concurrente.

 */

#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <shared_mutex>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

template <typename _keyType, typename _ObjIDType>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       // TODO: agregar funcion de comparacion
};

/**
 * @brief Trait para BTree ascendente.
 *
 * @tparam _keyType Tipo de la clave.
 * @tparam _ObjIDType Tipo del identificador de objeto.
 */
template <typename _keyType, typename _ObjIDType>
struct BTreeAscTraits{
    using  keyType            = _keyType;
    using  ObjIDType          = _ObjIDType;
    using  CompareFn          = less<_keyType>;
};

/**
 * @brief Trait para BTree descendente.
 *
 * @tparam _keyType Tipo de la clave.
 * @tparam _ObjIDType Tipo del identificador de objeto.
 */
template <typename _keyType, typename _ObjIDType>
struct BTreeDescTraits
{
    using  keyType           = _keyType;
    using  ObjIDType         = _ObjIDType;
    using  CompareFn         = greater<_keyType>;
};

template <typename Container, typename Iterator>
class general_iterator{
protected:
    using Node       = typename Container::BTNode;
    using keyType    = typename Container::keyType;
    using ObjIDType  = typename Container::ObjIDType;

    Container* m_pContainer;
    Node* m_pNode;
    size_t m_Indx;

public:
    general_iterator(Container* pContainer = nullptr, Node* pNode = nullptr, size_t indx = 0)
        : m_pContainer(pContainer), m_pNode(pNode), m_Indx(indx) {}
       value_type& operator*() const { return m_pNode->getDataRef(); }

    bool operator==(const Iterator& other) const { return m_pNode == other.m_pNode; }
    bool operator!=(const Iterator& other) const { return !(*this == other); }

    ObjectInfo& operator*(){ return m_node->m_Keys[m_Indx];}
    ObjectInfo* operator->(){return &(m_node->m_Keys[m_Indx]);}

    bool operator==(const iterator& other) {
        return m_Node == other.m_Node && 
            (m_Node == nullptr || m_Indx == other.m_Indx);
    }
    
    bool operator!=(const iterator& other) {
        return !(*this == other);
    }
};

template <typename Container>
class btree_forward_iterator 
    : public general_iterator<Container, btree_forward_iterator<Container>> 
{
public:
    using Parent    = general_iterator<Container, btree_forward_iterator<Container>>;
    using Node      = typename Container::BTNode;
    using iterator  = btree_forward_iterator<Container>;

public:
    btree_forward_iterator(Container *pContainer, Node *pNode) 
        : Parent(pContainer, pNode) {}

    btree_forward_iterator(const iterator& other)
        : Parent(other.m_pContainer, other.m_pNode) {}

    iterator& operator++() {
        this->m_pNode = this->m_pNode ? (Node*)this->m_pNode->getNext(this->m_Indx, true) : nullptr;
        return *this;
    }
};

template <typename Container>
class btree_backward_iterator 
    : public general_iterator<Container, btree_backward_iterator<Container>> 
{
public:
    using Parent    = general_iterator<Container, btree_backward_iterator<Container>>;
    using Node      = typename Container::BTNode;
    using iterator  = btree_backward_iterator<Container>;

public:
    btree_backward_iterator(Container *pContainer, Node *pNode) 
        : Parent(pContainer, pNode) {}

    btree_backward_iterator(const iterator& other)
        : Parent(other.m_pContainer, other.m_pNode) {}

    iterator& operator++() {
        this->m_pNode = this->m_pNode ? (Node*)this->m_pNode->getNext(this->m_Indx, false) : nullptr;
        return *this;
    }
};

/**
 * @brief Implementación de un BTree genérico.
 * 
 * Soporta inserción, eliminación, búsqueda y recorridos. 
 * 
 * @tparam Trait Traits del BTree.
 */
template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType      keyType;
       typedef typename Trait::ObjIDType    ObjIDType; 
       typedef typename Trait::CompareFn    CompareFn;
       
       typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
        typedef typename BTNode::ObjectInfo      ObjectInfo;

        using forward_iterator = btree_forward_iterator<BTree>;
        using backward_iterator = btree_backward_iterator<BTree>;

public:

    backward_iterator rbegin() { 
        if (!m_pRoot) return rend();
        return backward_iterator(this, getExtremeNode(m_pRoot, 1));
    }
    backward_iterator rend()   { return backward_iterator(this, nullptr); }

    forward_iterator fbegin() {
        if (!m_pRoot) return fend();
        return forward_iterator(this, getExtremeNode(m_pRoot, 0));
    }

    forward_iterator fend() {
        return forward_iterator(this, nullptr);
    }

    /**
     * @brief Constructor del BTree.
     * @param order Orden máximo del árbol.
     * @param unique Determina si los elementos deben ser únicos.
     */
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
              : m_Order(order),
                m_Root(2 * order  + 1, unique),
                m_Unique(unique),
                m_NumKeys(0)
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Height = 1;
       }
       ~BTree() {}
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();

       /**
        * @brief Move constructor.
        * @param other Otro BTree a mover.
        */
        BTree(BTree&& other) noexcept
       : m_Mutex() 
       {
              std::unique_lock lock(other.m_Mutex);

              m_Order   = std::exchange(other.m_Order, DEFAULT_BTREE_ORDER);
              m_Root    = std::move(other.m_Root); 
              m_Height  = std::exchange(other.m_Height, 1);
              m_NumKeys = std::exchange(other.m_NumKeys, 0);
              m_Unique  = std::exchange(other.m_Unique, true);
       }

       BTree &operator=(BTree&& other) noexcept {
           if (this != &other) {
               std::scoped_lock lock(m_Mutex, other.m_Mutex);

               m_Order   = std::exchange(other.m_Order, DEFAULT_BTREE_ORDER);
               m_Root    = std::move(other.m_Root); 
               m_Height  = std::exchange(other.m_Height, 1);
               m_NumKeys = std::exchange(other.m_NumKeys, 0);
               m_Unique  = std::exchange(other.m_Unique, true);
           }
           return *this;
       }

    
       /**
       @brief Inserta un nuevo elemento en el BTree.
       @param key Clave del elemento a insertar.   
       @param ObjID Identificador del objeto a insertar.
       @return true si la inserción fue exitosa, false en caso contrario.
       */     
       bool            Insert (const keyType key, const ObjIDType ObjID);

       /**
       @brief Elimina un elemento del BTree.
       @param key Clave del elemento a eliminar.
       @param ObjID Identificador del objeto a eliminar.
       @return true si la eliminación fue exitosa, false en caso contrario.
       */
       bool            Remove (const keyType key, const ObjIDType ObjID);

       /**
        * @brief Busca un objeto por su clave.
        * @param key Clave a buscar.
        * @return Identificador del objeto o -1 si no se encuentra.
        */
       ObjIDType       Search (const keyType key)
       {      
              std::shared_lock lock(m_Mutex);
              ObjIDType ObjID = -1;
              m_Root.Search(key, ObjID);
              return ObjID;
       }
       /**
       @brief Devuelve el número de claves en el BTree.
       @return Número de claves.
       */
       size_t            size()  { return m_NumKeys; }

       /**
       @brief Devuelve la altura del BTree.
       @return Altura del árbol.
       */
       size_t            height() { return m_Height;      }

       /**
       @brief Devuelve el orden del BTree.
       @return Orden del árbol.
       */
       size_t            GetOrder() { return m_Order;     }

       void            Print (ostream &os)
       {               std::shared_lock lock(m_Mutex);   
                       m_Root.Print(os);                              }



       /**
       @brief Aplica una función a cada elemento del BTree.
       @tparam Function Tipo de la función a aplicar.
       @tparam Args Tipos de los argumentos adicionales.
       */
       template <typename Function, typename... Args>
       void ForEach(Function function, Args const&... args) {
              std::shared_lock lock(m_Mutex);
              m_Root.ForEach(function,0, args...);
       }

       ObjectInfo*     FirstThat( lpfnFirstThat2 lpfn, void *pExtra1 )
       {               return m_Root.FirstThat(lpfn, 0, pExtra1);     }
       ObjectInfo*     FirstThat( lpfnFirstThat3 lpfn, void *pExtra1, void *pExtra2)
       {               return m_Root.FirstThat(lpfn, 0, pExtra1, pExtra2);   }
       //typedef               ObjectInfo iterator;

       /**
       @brief Aplica una función hasta encontrar el primer elemento que cumpla una condición.
       @tparam Function Tipo de la función
       @tparam Args Tipos de los argumentos adicionales.       
       */
       template <typename Function, typename... Args>
       ObjectInfo* FirstThat(Function function, Args const&... args){
            std::shared_lock lock(m_Mutex); 
            return m_Root.FirstThat(function, 0, args...);
       }

       /**
       @brief Escribe el BTree en un stream.
       @param os Stream de salida.
       */
       void Write(std::ostream &os){
            std::shared_lock lock(m_Mutex);
            os << m_Order << m_Height << m_NumKeys << m_Unique;
            m_Root.Write(os);
       }

       /**
       @brief Lee el BTree desde un stream.
       @param is Stream de entrada.
       */
       void Read(std::istream &is){
            std::unique_lock lock(m_Mutex);
            is >> m_Order >> m_Height >> m_NumKeys >> m_Unique;
            //Cantidad de claves - valor
            size_t n;
            is >> n;

            clear();

            for (size_t i = 0; i < n; ++i) {
                keyType key;
                ObjIDType objID;
                is >> key >> objID;
                Insert(key, objID);  
            }
        }



protected:
       BTNode          m_Root;
       size_t          m_Height;  // height of tree
       size_t          m_Order;   // order of tree
       size_t          m_NumKeys; // number of keys
       bool            m_Unique;  // Accept the elements only once ?
       std::shared_mutex m_Mutex;

       void clear(){
           m_Root.Reset();
           m_Height = 1;
           m_NumKeys = 0;
       }

       BTNode* getExtremeNode(BTNode* startNode, int direction) const {
            if (!startNode) return nullptr;
            
            BTNode* pNode = startNode;
            while (pNode->m_SubPages[direction ? 0 : pNode->m_KeyCount]) {
                pNode = pNode->m_SubPages[direction ? 0 : pNode->m_KeyCount];
            }
            return pNode;
        }
};     

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const ObjIDType ObjID){
       std::unique_lock lock(m_Mutex);
       bt_ErrorCode error = m_Root.Insert(key, ObjID);
       if( error == bt_duplicate )
               return false;
       m_NumKeys++;
       if( error == bt_overflow ){
               m_Root.SplitRoot();
               m_Height++;
       }
       return true;
}

template <typename Trait>
bool BTree<Trait>::Remove (const keyType key, const ObjIDType ObjID)
{      std::unique_lock lock(m_Mutex);
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if( error == bt_duplicate || error == bt_nofound )
               return false;
       m_NumKeys--;

       if( error == bt_rootmerged )
               m_Height--;
       return true;
}

template <typename Trait>
std::ostream& operator<<(std::ostream &os, const CBTreePage<Trait> &btree) {
    btree.Write(os);
    return os;
}

template <typename Trait>
std::istream& operator>>(std::istream &is, CBTreePage<Trait> &btree) {
    btree.Read(is);
    return is;
}

#endif