#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

// DONE: Funcion Compare agregada

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
       BTreeIterator& operator--();

private:
       CBTreePage<Trait> *m_pCurrentPage;
       size_t            m_CurrentIndex;

       BTreeIterator(CBTreePage<Trait> *pStartPage, size_t startIndex)
              : m_pCurrentPage(pStartPage), m_CurrentIndex(startIndex)
       {}
};

template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;
       typedef typename Trait::Compare           Compare;

       typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
       typedef typename BTNode::ObjectInfo      ObjectInfo;

       friend class BTreeIterator<Trait>;
       typedef BTreeIterator<Trait>       iterator;

       iterator begin() {
              BTNode *pPage = &m_Root;
              while (pPage->m_SubPages[0]) {
                     pPage = pPage->m_SubPages[0];
              }
              return iterator(pPage, 0);
       }

       iterator end() {
              return iterator(nullptr, 0);
       }

       iterator rbegin() {
              BTNode *pPage = &m_Root;
              while (pPage->m_SubPages[pPage->m_KeyCount]) {
                     pPage = pPage->m_SubPages[pPage->m_KeyCount];
              }
              return iterator(pPage, pPage->m_KeyCount - 1);
       }

       iterator rend() {
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

       bool Write(const std::string &filename) const;
       bool Read(const std::string &filename);

public:
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true, const Compare& compare = Compare())
              : m_Order(order),
                m_compare(compare),
                m_Root(2 * order  + 1, unique, m_compare),
                m_Unique(unique),
                m_NumKeys(0)
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Height = 1;
       }

       BTree(BTree &&other)
       : m_Order(other.m_Order),
         m_compare(std::move(other.m_compare)),
         m_Root(std::move(other.m_Root)),
         m_Unique(other.m_Unique),
         m_NumKeys(other.m_NumKeys),
         m_Height(other.m_Height)
       {
              other.m_NumKeys = 0;
              other.m_Height = 0;

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
       Compare       m_compare;
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
bool BTree<Trait>::Write(const std::string &filename) const
{
       std::ofstream file(filename);
       if (!file.is_open()) {
              std::cerr << "Error opening file for writing: " << filename << std::endl;
              return false;
       }

       file << m_Height << " " << m_NumKeys << "\n";
       
       // Write recursivo
       m_Root.Write(file);

       file.close();
       return !file.fail();
}

template <typename Trait>
bool BTree<Trait>::Read(const std::string &filename)
{
       std::ifstream file(filename);
       if (!file.is_open()) {
              std::cerr << "Error opening file for reading: " << filename << std::endl;
              return false;
       }

       m_Root.clear(); // limpiar arbol actual
       file >> m_Height >> m_NumKeys; // leer altura y numero de llaves
       m_Root.Read(file, nullptr); // lectura recursiva

       file.close();
       return !file.fail();
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
       CBTreePage<Trait> *pParent = pPage-> m_parent;
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
              pParent = pParent-> m_parent;
       }

       // Salir del bucle = final del arbol
       m_pCurrentPage = nullptr;
       m_CurrentIndex = 0;
       return *this;
}

template <typename Trait>
BTreeIterator<Trait>& BTreeIterator<Trait>::operator--()
{
       CBTreePage<Trait> *pPage = m_pCurrentPage;

       // Busca subarbol izquierdo
       CBTreePage<Trait> *lChild = pPage->m_SubPages[m_CurrentIndex];
       if (lChild != nullptr) {
              CBTreePage<Trait> *pNext = lChild;
              // Itera hasta el nodo hoja más a la derecha
              while (pNext->m_SubPages[pNext->m_KeyCount] != nullptr) {
                     pNext = pNext->m_SubPages[pNext->m_KeyCount];
              }
              m_pCurrentPage = pNext;
              m_CurrentIndex = pNext->m_KeyCount - 1;
              return *this;
       }

       // Si no tengo subarbol izquierdo, moverme a la izquierda EN EL MISMO NODO
       if (m_CurrentIndex > 0) {
              m_CurrentIndex--;
              return *this;
       }

       // Si no hay subarbol izquierdo ni más elementos en el mismo nodo,
       // subir al padre hasta encontrar un nodo donde pueda retroceder
       CBTreePage<Trait> *pParent = pPage-> m_parent;
       while (pParent != nullptr) {
              size_t parentIndex = 0;
              // Encontrar valor padre
              while (parentIndex < pParent->m_KeyCount &&
                     pParent->m_SubPages[parentIndex] != pPage) {
                     parentIndex++;
              }
              
              // Si el valor-padre tiene más elementos a la izquierda, retroceder aala
              if (parentIndex > 0) {
                     m_pCurrentPage = pParent;
                     m_CurrentIndex = parentIndex - 1;
                     return *this;
              }

              // Si el valor-padre ta no tiene valores a la izquierda, subir
              pPage = pParent;
              pParent = pParent-> m_parent;
       }

       // Salir del bucle = inicio del arbol
       m_pCurrentPage = nullptr;
       m_CurrentIndex = 0;
       return *this;
}

template <typename Trait>
ostream& operator<<(ostream &os, BTree<Trait> &btree)
{
       os << "[";
       bool first = true;

       for(auto& item : btree) {
              if (!first) {
                     os << ", ";
              }
              os << item.key << "->" << item.ObjID;
              first = false;
       }

       os << "]";
       return os;
}

#endif