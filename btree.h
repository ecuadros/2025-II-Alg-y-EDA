#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <utility>
#include <fstream>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

template <typename _keyType, typename _ObjIDType, typename _Compare = std::less<_keyType>>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       // TODO: agregar funcion de comparacion (DONE)
       using Compare = _Compare;
};

template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;
       
       typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
       typedef typename BTNode::ObjectInfo      ObjectInfo;

       class BTreeIterator {
       public:
              using iterator_category = std::bidirectional_iterator_tag;
              using value_type = ObjectInfo;
              using pointer = ObjectInfo*;
              using reference = ObjectInfo&;

              BTreeIterator(BTNode* pNode = nullptr, size_t keyIndex = 0)
                     : m_pNode(pNode), m_keyIndex(keyIndex) {}
              BTreeIterator(BTree* pTree = nullptr, BTNode* pNode = nullptr, size_t keyIndex = 0)
                     : m_pTree(pTree), m_pNode(pNode), m_keyIndex(keyIndex) {}

              reference operator*() const { return m_pNode->m_Keys[m_keyIndex]; }
              pointer operator->() const { return &m_pNode->m_Keys[m_keyIndex]; }

              BTreeIterator& operator++() { // Pre-incremento
                     // 1. Intentar avanzar al siguiente elemento en el mismo nodo
                     if (m_keyIndex + 1 < m_pNode->m_KeyCount) {
                            m_keyIndex++;
                            return *this;
                     }
                     if (!m_pNode) return *this; // Ya estamos en end()

                     // 2. Si no hay más claves, buscar el sucesor en orden
                     // El sucesor es el elemento más a la izquierda del subárbol derecho
                     BTNode* pNode = m_pNode->m_SubPages[m_keyIndex + 1];
                     if (pNode) {
                            while (pNode->m_SubPages[0]) {
                                   pNode = pNode->m_SubPages[0];
                     keyType currentKey = m_pNode->m_Keys[m_keyIndex].key;

                     // Usamos FirstThat para encontrar el siguiente elemento en orden
                     ObjectInfo* pNextInfo = m_pTree->FirstThat(0, 
                            [this, &currentKey](const ObjectInfo& info, size_t level) {
                                   return this->m_pTree->m_Root.m_Compare(currentKey, info.key); // currentKey < info.key
                            });

                     if (pNextInfo) {
                            // Encontramos el siguiente. Ahora necesitamos encontrar su nodo y su índice.
                            // Esta parte es compleja sin un mapa inverso. Por simplicidad,
                            // por ahora solo actualizamos el puntero. Una implementación más robusta
                            // necesitaría una forma de localizar el nodo del ObjectInfo.
                            // Para esta demo, el concepto es lo importante.
                            // La lógica manual con punteros al padre es más eficiente si se implementa correctamente.
                            // Pero para demostrar la reutilización, esta es la idea.
                            // Vamos a revertir a la lógica manual corregida, que es más performante.

                            // Lógica manual correcta:
                            // 1. Ir al subárbol derecho y encontrar el elemento más a la izquierda
                            BTNode* pCursor = m_pNode->m_SubPages[m_keyIndex + 1];
                            if (pCursor) {
                                   while (pCursor->m_SubPages[0]) {
                                          pCursor = pCursor->m_SubPages[0];
                                   }
                                   m_pNode = pCursor;
                                   m_keyIndex = 0;
                            } else {
                                   // 2. Si no hay subárbol derecho, subir hasta que ya no seamos un hijo derecho
                                   BTNode* pCurrent = m_pNode;
                                   m_pNode = m_pNode->m_pParent;
                                   while (m_pNode && m_pNode->m_SubPages[m_pNode->m_KeyCount] == pCurrent) {
                                          pCurrent = m_pNode;
                                          m_pNode = m_pNode->m_pParent;
                                   }
                                   if (m_pNode) {
                                          // Encontrar la clave en el padre que nos corresponde
                                          m_keyIndex = binary_search(m_pNode->m_Keys, 0, m_pNode->m_KeyCount, pCurrent->m_Keys[0].key, m_pNode->m_Compare);
                                   }
                            }
                            m_pNode = pNode;
                            m_keyIndex = 0; // El primer elemento del nodo más a la izquierda
                            return *this;
                     } else {
                            m_pNode = nullptr; // No hay más elementos, llegamos a end()
                     }

                     // 3. Si no hay subárbol derecho, subir hasta encontrar un ancestro
                     // que no sea un hijo derecho.
                     BTNode* pCurrent = m_pNode;
                     BTNode* pParent = pCurrent->m_pParent;
                     while (pParent && pParent->m_SubPages[pParent->m_KeyCount] == pCurrent) {
                            pCurrent = pParent;
                            pParent = pParent->m_pParent;
                     }
                     m_pNode = pParent; // Si pParent es null, hemos llegado al final (end())
                     return *this;
              }

              BTreeIterator& operator--() { // Pre-decremento
                     BTNode* prevNode = m_pNode->m_SubPages[m_keyIndex];
                     if (prevNode) {
                            while (prevNode->m_SubPages[prevNode->m_KeyCount]) {
                                   prevNode = prevNode->m_SubPages[prevNode->m_KeyCount];
                            }
                            m_pNode = prevNode;
                            m_keyIndex = prevNode->m_KeyCount - 1;
                     } else {
                            BTNode* pCurrent = m_pNode;
                            BTNode* pParent = pCurrent->m_pParent;
                            while (pParent && pParent->m_SubPages[0] == pCurrent) {
                                   pCurrent = pParent;
                                   pParent = pParent->m_pParent;
                            }
                            m_pNode = pParent; // Si es null, es el final (rend)
                     }
                     // Lógica para el decremento (similarmente compleja)
                     return *this;
              }

              bool operator==(const BTreeIterator& other) const { return m_pNode == other.m_pNode && m_keyIndex == other.m_keyIndex; }
              bool operator!=(const BTreeIterator& other) const { return !(*this == other); }

       private:
              BTree*  m_pTree;
              BTNode* m_pNode;
              size_t m_keyIndex;
       };

public:
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
              : m_Root(2 * order  + 1, unique),
                m_Order(order),
                m_NumKeys(0),
                m_Unique(unique)
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Height = 1;
       }
       BTree(BTree&& other) noexcept
              : m_Root(std::move(other.m_Root)),
                m_Height(std::exchange(other.m_Height, 1)),
                m_Order(std::exchange(other.m_Order, 0)),
                m_NumKeys(std::exchange(other.m_NumKeys, 0)),
                m_Unique(std::exchange(other.m_Unique, false))
       {

       }
       ~BTree() {}
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();
       void            Write(ostream& os);
       void            Read(istream& is);

       bool            Insert (const keyType key, const ObjIDType ObjID);
       bool            Remove (const keyType key, const ObjIDType ObjID);
       ObjIDType       Search (const keyType key)
       {      ObjIDType ObjID = -1;
              m_Root.Search(key, ObjID);
              return ObjID;
       }
       size_t            size()  { return m_NumKeys; }
       size_t            height() { return m_Height;      }
       size_t            GetOrder() { return m_Order;     }

       void            Print (ostream &os)
       {               m_Root.Print(os);                              }

       BTreeIterator begin() {
              BTNode* pNode = &m_Root;
              while (pNode && pNode->m_SubPages[0]) {
                     pNode = pNode->m_SubPages[0];
              }
              return BTreeIterator(pNode, 0);
       }
       BTreeIterator end() { return BTreeIterator(nullptr); }

       BTreeIterator rbegin() { /* ... implementación para el iterador reverso ... */ }
       BTreeIterator rend() { /* ... implementación para el iterador reverso ... */ }


       template<typename Func, typename... Args>
       void ForEach(size_t level, Func&& func, Args&&... args) {
              m_Root.ForEach(level, std::forward<Func>(func), std::forward<Args>(args)...);
       }

       template<typename Func, typename... Args>
       ObjectInfo* FirstThat(size_t level, Func&& func, Args&&... args) {
              return m_Root.FirstThat(level, std::forward<Func>(func), std::forward<Args>(args)...);
       }
protected:
       BTNode          m_Root;
       size_t          m_Height;  // height of tree
       size_t          m_Order;   // order of tree
       size_t          m_NumKeys; // number of keys
       bool            m_Unique;  // Accept the elements only once ?
};     

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const ObjIDType ObjID){
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
    os << m_Order << "\n";
    os << m_Unique << "\n";
    os << m_NumKeys << "\n";
    os << m_Height << "\n";
    m_Root.Write(os);
}

template <typename Trait>
void BTree<Trait>::Read(istream& is) {
    is >> m_Order >> m_Unique >> m_NumKeys >> m_Height;

    m_Root.m_MaxKeys = 2 * m_Order + 1;
    m_Root.m_Unique = m_Unique;
    m_Root.Create();

    m_Root.Read(is);
}

template <typename Trait>
bool BTree<Trait>::Remove (const keyType key, const ObjIDType ObjID)
{
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if( error == bt_duplicate || error == bt_nofound )
               return false;
       m_NumKeys--;

       if( error == bt_rootmerged )
               m_Height--;
       return true;
}

template <typename Trait>
std::ostream& operator<<(std::ostream& os, BTree<Trait>& tree) {
    tree.Print(os);
    return os;
}

#endif