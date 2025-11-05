#ifndef __BTREE_H__
#define __BTREE_H__

/**
 * @file btree.h
 * @brief Define la clase BTree, una estructura de datos de árbol balanceado.
 */

#include <shared_mutex>
#include <iostream>
#include <utility>
#include <mutex> 
#include <iterator>
#include <fstream>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

/**
 * @struct BTreeTrait
 * @brief Define los tipos y la función de comparación para el B-Tree.
 * @tparam _keyType El tipo de dato para las claves.
 * @tparam _ObjIDType El tipo de dato para los IDs de objeto (valores).
 * @tparam _Compare La función de comparación (ej. std::less o std::greater).
 */
template <typename _keyType, typename _ObjIDType, typename _Compare = std::less<_keyType>>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       // TODO: agregar funcion de comparacion (DONE)
       using Compare = _Compare;
};

/**
 * @struct BTreeDescTrait
 * @brief Trait de ejemplo para un B-Tree descendente.
 * @tparam _keyType El tipo de dato para las claves.
 * @tparam _ObjIDType El tipo de dato para los IDs de objeto.
 */
template <typename _keyType, typename _ObjIDType>
struct BTreeDescTrait : public BTreeTrait<_keyType, _ObjIDType, std::greater<_keyType>> {};

/**
 * @class BTree
 * @brief Implementa una estructura de datos B-Tree.
 * @tparam Trait Un struct que define los tipos usados por el B-Tree.
 */
template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;
       
       typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
       typedef typename BTNode::ObjectInfo      ObjectInfo;

       /**
        * @class BTreeIterator
        * @brief Un iterador bidireccional para el B-Tree.
        */
       class BTreeIterator {
       public:
              using iterator_category = std::bidirectional_iterator_tag;
              using value_type = ObjectInfo;
              using pointer = ObjectInfo*;
              using reference = ObjectInfo&;

              /**
               * @brief Construye un iterador de B-Tree.
               * @param pTree Puntero al B-Tree padre.
               * @param pNode Puntero al nodo actual.
               * @param keyIndex Índice de la clave dentro del nodo.
               */
              BTreeIterator(BTree* pTree, BTNode* pNode = nullptr, size_t keyIndex = 0)
                     : m_pTree(pTree), m_pNode(pNode), m_keyIndex(keyIndex) {}

              /// Desreferencia el iterador para obtener el elemento.
              reference operator*() const { return m_pNode->m_Keys[m_keyIndex]; }
              /// Desreferencia el iterador para acceder a un miembro del elemento.
              pointer operator->() const { return &m_pNode->m_Keys[m_keyIndex]; }

              /// Operador de pre-incremento. Avanza el iterador al siguiente elemento.
              BTreeIterator& operator++() {
                     if (!m_pNode) {
                         return *this;
                     }
 
                     if (m_pNode->m_SubPages[0] != nullptr) {
                         BTNode* pCursor = m_pNode->m_SubPages[m_keyIndex + 1];
                         while (pCursor->m_SubPages[0] != nullptr) {
                             pCursor = pCursor->m_SubPages[0];
                         }
                         m_pNode = pCursor;
                         m_keyIndex = 0;
                         return *this;
                     }

                     if (m_pNode->m_SubPages[0] == nullptr) { // Es un nodo hoja
                         m_keyIndex++;
                         if (m_keyIndex < m_pNode->m_KeyCount) {
                             return *this;
                         }
                         BTNode* pCurrent = m_pNode;
                         BTNode* pParent = pCurrent->m_pParent;
                         while (pParent != nullptr && pParent->m_SubPages[pParent->m_KeyCount] == pCurrent) {
                             pCurrent = pParent;
                             pParent = pParent->m_pParent;
                         }

                         if (pParent == nullptr) {
                             m_pNode = nullptr;
                         } else {
                             size_t pos = 0;
                             while(pParent->m_SubPages[pos] != pCurrent) pos++;
                             m_pNode = pParent;
                             m_keyIndex = pos;
                         }
                     }
                     return *this;
              }

              /// Operador de pre-decremento. Mueve el iterador al elemento anterior.
              BTreeIterator& operator--() {
                    if (!m_pNode) {
                        m_pNode = &m_pTree->m_Root;
                        while (m_pNode->m_SubPages[m_pNode->m_KeyCount]) {
                            m_pNode = m_pNode->m_SubPages[m_pNode->m_KeyCount];
                        }
                        m_keyIndex = m_pNode->m_KeyCount - 1;
                        return *this;
                    }

                    BTNode* pCursor = m_pNode->m_SubPages[m_keyIndex];
                    if (pCursor) {
                        while (pCursor->m_SubPages[pCursor->m_KeyCount]) {
                            pCursor = pCursor->m_SubPages[pCursor->m_KeyCount];
                        }
                        m_pNode = pCursor;
                        m_keyIndex = pCursor->m_KeyCount - 1;
                    } else {
                        BTNode* pChild = m_pNode;
                        BTNode* pParent = m_pNode->m_pParent;
                        while (pParent && pParent->m_SubPages[0] == pChild) {
                            pChild = pParent;
                            pParent = pParent->m_pParent;
                        }
                        if (!pParent) {
                            m_pNode = nullptr;
                        } else {
                            size_t child_pos = 0;
                            while(child_pos <= pParent->m_KeyCount && pParent->m_SubPages[child_pos] != pChild) {
                                child_pos++;
                            }
                            m_pNode = pParent;
                            m_keyIndex = child_pos - 1;
                        }
                    }
                     return *this;
              }

              /// Operador de comparación de igualdad.
              bool operator==(const BTreeIterator& other) const { return m_pNode == other.m_pNode && m_keyIndex == other.m_keyIndex; }
              /// Operador de comparación de desigualdad.
              bool operator!=(const BTreeIterator& other) const { return !(*this == other); }

       private:
              BTree*  m_pTree;    ///< Puntero al B-Tree al que pertenece este iterador.
              BTNode* m_pNode;    ///< Puntero al nodo actual en el árbol.
              size_t m_keyIndex;  ///< Índice de la clave actual en el nodo.
       };

       using iterator = BTreeIterator;
       using reverse_iterator = std::reverse_iterator<iterator>;

public:
       /**
        * @brief Constructor del B-Tree.
        * @param order El orden del árbol.
        * @param unique Verdadero si las claves deben ser únicas.
        */
public:
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
              : m_Order(order),
                m_Root(2 * order  + 1, unique),
                m_NumKeys(0),
                m_Unique(unique)
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Height = 1;
       }
       /**
        * @brief Constructor por movimiento.
        * @param other El B-Tree a mover.
        */
       BTree(BTree&& other) noexcept
              : m_Root(other.m_Root),
                m_Height(std::exchange(other.m_Height, 1)),
                m_Order(std::exchange(other.m_Order, 0)),
                m_NumKeys(std::exchange(other.m_NumKeys, 0)),
                m_Unique(std::exchange(other.m_Unique, false))
       {

       }
       /// @brief Destructor.
       ~BTree() {}
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();
       /// @brief Escribe el árbol a un flujo de salida para serialización.
       /// @param os El flujo de salida.
       void            Write(ostream& os);
       /// @brief Lee el árbol desde un flujo de entrada para deserialización.
       /// @param is El flujo de entrada.
       void            Read(istream& is);

       /**
        * @brief Inserta un par clave-valor en el árbol.
        * @param key La clave a insertar.
        * @param ObjID El valor (ID de objeto) a asociar con la clave.
        * @return Verdadero si la inserción fue exitosa, falso si la clave era un duplicado.
        */
       bool            Insert (const keyType key, const ObjIDType ObjID);
       /**
        * @brief Elimina una clave del árbol.
        * @param key La clave a eliminar.
        * @param ObjID El ID de objeto asociado (actualmente no se usa en la lógica de eliminación).
        * @return Verdadero si la eliminación fue exitosa.
        */
       bool            Remove (const keyType key, const ObjIDType ObjID);
       /**
        * @brief Busca una clave en el árbol.
        * @param key La clave a buscar.
        * @return El ID del objeto si se encuentra, de lo contrario -1.
        */
       ObjIDType       Search (const keyType key)
       {      
              std::lock_guard<std::shared_mutex> lock(m_Mutex);
              ObjIDType ObjID = -1;
              m_Root.Search(key, ObjID);
              return ObjID;
       }
       /// Devuelve el número total de claves en el árbol.
       size_t            size()  const { std::shared_lock<std::shared_mutex> lock(m_Mutex); return m_NumKeys; }
       /// Devuelve la altura del árbol.
       size_t            height() const { std::shared_lock<std::shared_mutex> lock(m_Mutex); return m_Height;      }
       /// Devuelve el orden del árbol.
       size_t            GetOrder() const { std::shared_lock<std::shared_mutex> lock(m_Mutex); return m_Order;     }

       /// Imprime la estructura del árbol en un flujo de salida.
       void            Print (ostream &os) const
       {               
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              m_Root.Print(os);
       }

       /// Devuelve un iterador al primer elemento del árbol.
       iterator begin() {
              std::lock_guard<std::shared_mutex> lock(m_Mutex);
              BTNode* pNode = &m_Root;
              if (pNode->m_KeyCount == 0) { 
                  return iterator(this, nullptr, 0); // Retorna end()
              }
              while (pNode->m_SubPages[0] != nullptr) { 
                     pNode = pNode->m_SubPages[0];
              }
              return iterator(this, pNode, 0);
       }
       /// Devuelve un iterador al elemento siguiente al último.
       iterator end() { 
              return iterator(this, nullptr, 0); 
       }

       /// Devuelve un iterador inverso al último elemento.
       reverse_iterator rbegin() { 
              std::lock_guard<std::shared_mutex> lock(m_Mutex);
              return reverse_iterator(end()); 
       }
       /// Devuelve un iterador inverso al elemento anterior al primero.
       reverse_iterator rend() { return reverse_iterator(begin()); }


       /**
        * @brief Aplica una función a cada elemento del árbol en orden.
        * @tparam Func El tipo de la función.
        * @tparam Args Los tipos de los argumentos adicionales para la función.
        * @param level Nivel inicial para el recorrido (usualmente 0).
        * @param func La función a aplicar.
        * @param args Argumentos adicionales para la función.
        */
       template<typename Func, typename... Args>
       void ForEach(size_t level, Func&& func, Args&&... args) {
              m_Root.ForEach(level, std::forward<Func>(func), std::forward<Args>(args)...);
       }

       /**
        * @brief Busca el primer elemento que satisface una condición.
        * @return Un puntero al ObjectInfo si se encuentra, de lo contrario nullptr.
        */
       template<typename Func, typename... Args>
       ObjectInfo* FirstThat(size_t level, Func&& func, Args&&... args) {
              return m_Root.FirstThat(level, std::forward<Func>(func), std::forward<Args>(args)...);
       }
protected:
       BTNode          m_Root;    ///< El nodo raíz del B-Tree.
       size_t          m_Height;  ///< Altura del árbol.
       size_t          m_Order;   ///< Orden del árbol.
       size_t          m_NumKeys; ///< Número total de claves en el árbol.
       bool            m_Unique;  ///< Verdadero si las claves deben ser únicas.
       mutable std::shared_mutex m_Mutex; ///< Mutex de lectura-escritura para seguridad en hilos.
};     

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const ObjIDType ObjID){
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
void BTree<Trait>::Write(ostream& os) {
    std::lock_guard<std::shared_mutex> lock(m_Mutex);
    os << m_Order << "\n";
    os << m_Unique << "\n";
    os << m_NumKeys << "\n";
    os << m_Height << "\n";
    m_Root.Write(os);
}

template <typename Trait>
void BTree<Trait>::Read(istream& is) {
    std::lock_guard<std::shared_mutex> lock(m_Mutex);
    is >> m_Order >> m_Unique >> m_NumKeys >> m_Height;

    m_Root.m_MaxKeys = 2 * m_Order + 1;
    m_Root.m_Unique = m_Unique;
    m_Root.Create();

    m_Root.Read(is);
}

template <typename Trait>
bool BTree<Trait>::Remove (const keyType key, const ObjIDType ObjID)
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

/// Sobrecarga del operador << para imprimir el B-Tree.
template <typename Trait>
std::ostream& operator<<(std::ostream& os, const BTree<Trait>& tree) {
    tree.Print(os);
    return os;
}

#endif