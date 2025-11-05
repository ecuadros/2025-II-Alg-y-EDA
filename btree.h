#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

/**
 * @brief Trait base para definir tipos de clave, valor y función comparadora del B-Tree.
 * @tparam _keyType Tipo de la clave del árbol.
 * @tparam _ObjIDType Tipo del valor asociado a cada clave.
 * @tparam _Compare función comparador para mantener el orden
 */
template <typename _keyType, typename _ObjIDType, typename _Compare>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       using CompareFn = _Compare;
       // TODO (LISTO): agregar funcion de comparacion
};

/**
 * @brief Trait de B-Tree ordenado de forma descendente.
 * @tparam Key Tipo de clave.
 * @tparam Value Tipo de valor.
 */
template <typename Key, typename Value>
struct BTreeDescTrait {
    using keyType = Key;
    using ObjIDType = Value;
    using CompareFn = std::greater<Key>;
};

/**
 * @brief Trait de B-Tree ordenado de forma ascendente.
 * @tparam Key Tipo de clave.
 * @tparam Value Tipo de valor.
 */
template <typename Key, typename Value>
struct BTreeAscTrait {
    using keyType = Key;
    using ObjIDType = Value;
    using CompareFn = std::less<Key>;
};


/**
    *   @file btree.h
    *   @brief Implementación de un B-Tree
    *   @date 2025
*/

/**
 * @class BTree
 * @brief B-tree y template.
 * @tparam Trait que define clave, valor y comparador
 */
template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType    keyType;
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
        *   @brief Copy constructor
        *   @param order order Orden del árbol
        *   @param unique unique True si no se permiten claves duplicadas
        */
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
              : m_Root(2 * order + 1, unique),
                m_Height(1),
                m_Order(order),
                m_NumKeys(0),
                m_Unique(unique),
                m_Comp()
       {
              m_Root.SetMaxKeysForChilds(order);
       }

       /**
        * @brief Move Constructor
        */
       BTree(BTree &&other) {
              std::lock_guard<std::shared_mutex> lock(other.m_Mutex);
		m_Root = std::move(other.m_Root);
		m_Height = std::move(other.m_Height);
		m_Order = std::move(other.m_Order);
		m_NumKeys = std::move(other.m_NumKeys);
		m_Unique = std::move(other.m_Unique);
		m_Comp = std::move(other.m_Comp);
	}

       ~BTree() {}
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();

	/**
	 * @brief Inserta una clave y su referencia asociada
	 * 
	 * @param key Clave a insertar en el árbol
	 * @param ObjID Referencia (identificador o puntero lógico) asociada a la clave
	 * @return true Si la inserción fue exitosa
	 * @return false Si la clave ya existía (cuando unique es true)
	 */
       bool            Insert (const keyType key, const long ObjID);
	/**
	 * @brief Elimina una clave y su referencia asociada
	 * 
	 * @param key Clave que se desea eliminar
	 * @param ObjID Referencia asociada a la clave
	 * @return true Si la eliminación fue exitosa
	 * @return false Si la clave no se encontró o no pudo eliminarse
	 */
       bool            Remove (const keyType key, const long ObjID);

       /**
	 * @brief Busca una clave en el árbol
	 * 
	 * @param key Clave a buscar.
	 * @return La referencia (ObjID) asociada a la clave, o -1 si no se encontró
	 */
       ObjIDType       Search (const keyType key)
       {      
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              ObjIDType ObjID = -1;
              m_Root.Search(key, ObjID);
              return ObjID;
       }
       /** @brief Devuelve el número total de datos almacenados */
       size_t            size()  { std::shared_lock<std::shared_mutex> lock(m_Mutex); return m_NumKeys;      }
       /** @brief Devuelve la altura actual del árbol */
       size_t            height() { std::shared_lock<std::shared_mutex> lock(m_Mutex); return m_Height;      }
       /** @brief Devuelve el orden del árbol */
       size_t            GetOrder() { std::shared_lock<std::shared_mutex> lock(m_Mutex); return m_Order;     }

       /**
	 * @brief Imprime la estructura completa del árbol
	 * 
	 * @param os Flujo de salida donde se imprimirá la estructura
	 */
       void            Print (  ostream &os)
       {               std::shared_lock<std::shared_mutex> lock(m_Mutex); m_Root.Print(os);                              }
       	/**
	 * @brief Aplica una función a cada elemento del árbol
	 * 
	 * @param lpfn Puntero a función que se ejecutará por cada par (clave, referencia)
	 * @param pExtra1 Parámetro adicional que puede ser pasado a la función
	 */
       void            ForEach( lpfnForEach2 lpfn, void *pExtra1 )
       {               std::lock_guard<std::shared_mutex> lock(m_Mutex); m_Root.ForEach(lpfn, 0, pExtra1);              }
       /**
	 * @brief Aplica una función a cada elemento del árbol con dos parámetros adicionales
	 * 
	 * @param lpfn Puntero a función
	 * @param pExtra1 Parámetro adicional 1
	 * @param pExtra2 Parámetro adicional 2
	 */
       void            ForEach( lpfnForEach3 lpfn, void *pExtra1, void *pExtra2)
       {               std::lock_guard<std::shared_mutex> lock(m_Mutex); m_Root.ForEach(lpfn, 0, pExtra1, pExtra2);     }


       /**
        * @brief Aplica una función con parámetros variables a cada elemento
        * @tparam Function Tipo de función/lambda
        * @tparam Args Tipos de argumentos adicionales
        * @param func Función a aplicar
        * @param args Argumentos adicionales
        */
       template <typename Function, typename... Args>
       void ForEach_variadic(Function&& func, Args&&... args) {
              std::lock_guard<std::shared_mutex> lock(m_Mutex);
              m_Root.ForEach_variadic(std::forward<Function>(func), 0, std::forward<Args>(args)...);
       }

       
       /**
	 * @brief Busca el primer elemento que cumpla una condición dada
	 * 
	 * @param lpfn Función de condición que evalúa cada nodo
	 * @param pExtra1 Parámetro adicional
	 * @return Puntero a la información del objeto (ObjectInfo) si se encuentra
	 */
       ObjectInfo*     FirstThat( lpfnFirstThat2 lpfn, void *pExtra1 )
       {               std::shared_lock<std::shared_mutex> lock(m_Mutex); return m_Root.FirstThat(lpfn, 0, pExtra1);     }

       /**
	 * @brief Variante de búsqueda con dos parámetros adicionales
	 * 
	 * @param lpfn Función que define la condición
	 * @param pExtra1 Parámetro adicional 1
	 * @param pExtra2 Parámetro adicional 2
	 * @return Puntero a la información del objeto (ObjectInfo) si se encuentra
	 */
       ObjectInfo*     FirstThat( lpfnFirstThat3 lpfn, void *pExtra1, void *pExtra2)
       {               std::shared_lock<std::shared_mutex> lock(m_Mutex); return m_Root.FirstThat(lpfn, 0, pExtra1, pExtra2);   }
       //typedef               ObjectInfo iterator;

       /**
        * @brief Busca el primero que cumpla una función con variadic templates
        * @tparam Function Tipo de función/lambda
        * @tparam Args Tipos de argumentos adicionales
        * @param func Función a aplicar
        * @param args Argumentos adicionales
        */
       template <typename Function, typename... Args>
       ObjectInfo* FirstThat_variadic(Function&& func, Args&&... args) {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              return m_Root.FirstThat_variadic(std::forward<Function>(func), 0, std::forward<Args>(args)...);
       }

       /**
        * @brief Escribe el árbol por un flujo de salida (ostream)
        * 
        * @param os Flujo de salida donde se escribirán los metadatos y nodos del árbol
        * @return Referencia al flujo de salida
        */
       std::ostream&  Write(std::ostream &os) { 
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              os << m_Order << " " << m_Height << " " << m_NumKeys << " " << m_Unique << "\n";
              m_Root.Write(os); 
              return os;
       }

       /**
        * @brief Lee la estructura del árbol desde un flujo de entrada (istream)
        * 
        * @param is Flujo de entrada desde el cual se cargará la estructura
        * @return Referencia al flujo de entrada
        */
       std::istream&  Read(std::istream &is) { 
              std::lock_guard<std::shared_mutex> lock(m_Mutex);
              size_t order, height, numKeys;
              bool unique;
              is >> order >> height >> numKeys >> unique;

              m_Root.Reset();

              m_Order = order;
              m_Height = height;
              m_NumKeys = numKeys;
              m_Unique = unique;

              m_Root = BTNode(2 * order + 1, unique);
              m_Root.SetMaxKeysForChilds(order);
              m_Root.Read(is);
              return is;
       }

protected:
       BTNode          m_Root;
       size_t          m_Height;  // height of tree
       size_t          m_Order;   // order of tree
       size_t          m_NumKeys; // number of keys
       bool            m_Unique;  // Accept the elements only once ?
       CompareFn       m_Comp;

private:
       std::shared_mutex m_Mutex;
};     

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const long ObjID){
       std::lock_guard<std::shared_mutex> lock(m_Mutex);
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
bool BTree<Trait>::Remove (const keyType key, const long ObjID)
{
       std::lock_guard<std::shared_mutex> lock(m_Mutex);
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if( error == bt_duplicate || error == bt_nofound )
               return false;
       m_NumKeys--;

       if( error == bt_rootmerged )
               m_Height--;
       return true;
}

/**
 * @brief Sobrecarga del operador de salida
 * 
 * Imprime la estructura jerárquica del árbol en un flujo de salida
 * 
 * @param os Flujo de salida
 * @param tree Árbol a imprimir
 * @return Referencia al flujo de salida
 */
template <typename Trait>
ostream& operator<<(ostream& os, BTree<Trait>& tree)
{
    tree.Print(os);
    return os;
}


void DemoBTree();

#endif