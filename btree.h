#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

// DONE #1: Función de comparación agregada y propagada.

const size_t MaxHeight = 5; 

template <typename _keyType, typename _ObjIDType, typename _Compare = std::less<_keyType> >
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       using Compare = _Compare; 
};

template <typename Trait>
class BTreeIterator
{
public:
       friend class BTree<Trait>;
       typedef typename Trait::keyType           keyType;
       typedef typename Trait::ObjIDType         ObjIDType;
       typedef typename Trait::Compare           Compare;
       typedef tagObjectInfo<keyType, ObjIDType> ObjectInfo;
       
       // Operadores de referencia
       ObjectInfo& operator*() { 
              return m_pCurrentPage->m_Keys[m_CurrentIndex];
       }
       ObjectInfo* operator->() {
              return &(m_pCurrentPage->m_Keys[m_CurrentIndex]);
       }

       // Operadores de comparacion
       bool operator==(const BTreeIterator &other) const {
              return (m_pCurrentPage == other.m_pCurrentPage) &&
                     (m_CurrentIndex == other.m_CurrentIndex);
       }
       
       bool operator!=(const BTreeIterator &other) const {
              return !(*this == other);
       }

       // Operadores de incremento
       BTreeIterator& operator++();

private:
       CBTreePage<Trait> *m_pCurrentPage;
       size_t            m_CurrentIndex;

       BTreeIterator(CBTreePage<Trait> *pStartPage, size_t startIndex)
              : m_pCurrentPage(pStartPage), m_CurrentIndex(startIndex)
       {
       }
};

template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType           keyType;
       typedef typename Trait::ObjIDType         ObjIDType;
       typedef typename Trait::Compare           Compare;
       
       typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
       //typedef ObjectInfo iterator;
       typedef typename BTNode::ObjectInfo      ObjectInfo;

       friend class BTreeIterator<Trait>;
       typedef BTreeIterator<Trait>       iterator;

       iterator begin() {
              CBTreePage<Trait> *pPage = &m_Root;
              while (pPage->m_SubPages[0]) {
                     pPage = pPage->m_SubPages[0];
              }
              return iterator(pPage, 0);
       }

       iterator end() {
              return iterator(nullptr, 0);
       }

       template <typename T_Func>
       void ForEach(T_Func fn)
       { 
           m_Root.ForEach(fn, 0); 
       }

       template <typename T_Func>
       ObjectInfo* FirstThat(T_Func fn)
       { 
           return m_Root.FirstThat(fn, 0); 
       }

public:
       BTree(size_t order = DEFAULT_BTREE_ORDER, 
              bool unique = true,
              const Compare &compare = Compare())
              : m_Order(order),
                m_compare(compare),
                m_Root(2 * order  + 1, unique, m_compare),
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
       bool            Insert (const keyType key, const long ObjID);
       bool            Remove (const keyType key, const long ObjID);
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
       //typedef               ObjectInfo iterator;

protected:
       size_t          m_Order;   // order of tree
       Compare         m_compare;
       BTNode          m_Root;
       bool            m_Unique;  // Accept the elements only once ?
       size_t          m_NumKeys; // number of keys
       size_t          m_Height;  // height of tree   
};     

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const long ObjID){
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
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if( error == bt_duplicate || error == bt_nofound )
               return false;
       m_NumKeys--;

       if( error == bt_rootmerged )
               m_Height--;
       return true;
}

template <typename Trait>
BTreeIterator<Trait>& BTreeIterator<Trait>::operator++()
{
       CBTreePage<Trait> *pPage = m_pCurrentPage;

       // Busca subarbol derecho
       CBTreePage<Trait> *rChild = pPage->m_SubPages[m_CurrentIndex + 1];
       if (rChild != nullptr) {
              CBTreePage<Trait> *pNext = rChild;
              // Itera hasta el nodo hoja más a la izquierda
              while (pNext->m_SubPages[0] != nullptr) {
                     pNext = pNext->m_SubPages[0];
              }
              m_pCurrentPage = pNext;
              m_CurrentIndex = 0;
              return *this;
       }

       // Si no tengo subarbol derecho, moverme a la derecha EN EL MISMO NODO
       if (m_CurrentIndex + 1 < pPage->m_KeyCount) {
              m_CurrentIndex++;
              return *this;
       }

       // Si no hay subarbol derecho ni más elementos en el mismo nodo,
       // subir al padre hasta encontrar un nodo donde pueda avanzar
       CBTreePage<Trait> *pParent = pPage->m_Parent;
       while (pParent != nullptr) {
              size_t parentIndex = 0;
              // Encontrar valor padre
              while (parentIndex < pParent->m_KeyCount &&
                     pParent->m_SubPages[parentIndex] != pPage) {
                     parentIndex++;
              }
              
              // Si el valor-padre tiene más elementos a la derecha, avanzar aala
              if (parentIndex < pParent->m_KeyCount) {
                     m_pCurrentPage = pParent;
                     m_CurrentIndex = parentIndex;
                     return *this;
              }

              // Si el valor-padre ta no tiene valores a la derecha, subir
              pPage = pParent;
              pParent = pParent->m_Parent;
       }

       // Salir del bucle = final del arbol
       m_pCurrentPage = nullptr;
       m_CurrentIndex = 0;
       return *this;
}

#endif