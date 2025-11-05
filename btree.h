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
       //typedef ObjectInfo iterator;
       typedef typename BTNode::lpfnForEach2    lpfnForEach2;
       typedef typename BTNode::lpfnForEach3    lpfnForEach3;
       typedef typename BTNode::lpfnFirstThat2  lpfnFirstThat2;
       typedef typename BTNode::lpfnFirstThat3  lpfnFirstThat3;
       typedef typename BTNode::ObjectInfo      ObjectInfo;

public:
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
              : m_Root(std::move(other.m_Root)),
                m_Height(other.m_Height),
                m_Order(other.m_Order),
                m_NumKeys(other.m_NumKeys),
                m_Unique(other.m_Unique)
            {
              other.m_Height = 1;
              other.m_NumKeys = 0;
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
       void            ForEach( lpfnForEach2 lpfn, void *pExtra1 )
       {               m_Root.ForEach(lpfn, 0, pExtra1);              }
       void            ForEach( lpfnForEach3 lpfn, void *pExtra1, void *pExtra2)
       {               m_Root.ForEach(lpfn, 0, pExtra1, pExtra2);     }


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