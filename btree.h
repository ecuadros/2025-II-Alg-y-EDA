#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <functional>
#include <utility>
#include <fstream>
#include <string>
#include <iterator>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

template <typename _keyType, typename _ObjIDType, typename _Compare>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       // TODO (completado): agregar funcion de comparacion       
       using Compare = _Compare;
};

// Convenience aliases for common use cases
template <typename Key, typename Value>
using BTreeAscTrait = BTreeTrait<Key, Value, std::less<Key>>;

template <typename Key, typename Value>
using BTreeDescTrait = BTreeTrait<Key, Value, std::greater<Key>>;

// Declaración anticipada
template <typename Trait>
class BTree;

// Iterador bidireccional para BTree
// Permite navegar el árbol en orden (in-order traversal)
template <typename Trait>
class BTreeIterator
{
       friend class BTree<Trait>;
       typedef CBTreePage<Trait> BTPage;
       typedef typename BTPage::ObjectInfo ObjectInfo;
       
private:
       BTPage* m_CurrentPage;    // Página actual
       size_t m_CurrentIndex;    // Índice en la página actual
       
       BTreeIterator(BTPage* page, size_t index) 
              : m_CurrentPage(page), m_CurrentIndex(index) {}
       
public:
       // Tipos para compatibilidad con STL
       using iterator_category = std::bidirectional_iterator_tag;
       using value_type = ObjectInfo;
       using difference_type = std::ptrdiff_t;
       using pointer = ObjectInfo*;
       using reference = ObjectInfo&;
       
       BTreeIterator() : m_CurrentPage(nullptr), m_CurrentIndex(0) {}
       
       // Operadores de desreferencia
       ObjectInfo& operator*() { return m_CurrentPage->m_Keys[m_CurrentIndex]; }
       ObjectInfo* operator->() { return &(m_CurrentPage->m_Keys[m_CurrentIndex]); }
       
       // Pre-incremento: avanza al siguiente elemento en orden ascendente
       BTreeIterator& operator++();
       
       // Post-incremento
       BTreeIterator operator++(int)
       {
               BTreeIterator temp = *this;
               ++(*this);
               return temp;
       }
       
       // Pre-decremento: retrocede al elemento anterior en orden ascendente
       BTreeIterator& operator--();
       
       // Post-decremento
       BTreeIterator operator--(int)
       {
               BTreeIterator temp = *this;
               --(*this);
               return temp;
       }
       
       // Operadores de comparación
       bool operator==(const BTreeIterator& other) const
       {
               if (m_CurrentPage == nullptr && other.m_CurrentPage == nullptr)
                       return true;
               return m_CurrentPage == other.m_CurrentPage && m_CurrentIndex == other.m_CurrentIndex;
       }
       
       bool operator!=(const BTreeIterator& other) const
       {
               return !(*this == other);
       }
};

template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;
       
       typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
       typedef typename BTNode::ObjectInfo      ObjectInfo;
       
       // Soporte para iterador bidireccional
       friend class BTreeIterator<Trait>;
       typedef BTreeIterator<Trait> iterator;

public:
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
              : m_Order(order),
                m_Root(2 * order  + 1, unique),
                m_Unique(unique),
                m_NumKeys(0)
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Height = 1;
       }

       // Move constructor: transfers ownership efficiently
       BTree(BTree&& other) noexcept 
              : m_Order(other.m_Order),
                m_Root(std::move(other.m_Root)),
                m_Height(other.m_Height),
                m_Unique(other.m_Unique),
                m_NumKeys(other.m_NumKeys)
       {
              other.m_Height = 1;  // Leave source in valid state
              other.m_NumKeys = 0;
       }
       
       // Move assignment: transfers ownership to existing object
       BTree& operator=(BTree&& other) noexcept 
       {
              if (this != &other)
              {
                     m_Order = other.m_Order;
                     m_Root = std::move(other.m_Root);
                     m_Height = other.m_Height;
                     m_Unique = other.m_Unique;
                     m_NumKeys = other.m_NumKeys;
                     
                     other.m_Height = 1;  // Leave source in valid state
                     other.m_NumKeys = 0;
              }
              return *this;
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

       void            Print (ostream &os) const
       {               m_Root.Print(os);                              }
       
       std::ostream&   Write(std::ostream& os) const;   // Guardar árbol en stream
       std::istream&   Read(std::istream& is);          // Cargar árbol desde stream
       
       // ForEach generalizado con variadic templates
       template <typename Func, typename... Args>
       void            ForEach(Func&& func, Args&&... args)
       {               m_Root.ForEach(std::forward<Func>(func), 0, std::forward<Args>(args)...);              }
       
       // FirstThat generalizado con variadic templates
       template <typename Pred, typename... Args>
       ObjectInfo*     FirstThat(Pred&& predicate, Args&&... args)
       {               return m_Root.FirstThat(std::forward<Pred>(predicate), 0, std::forward<Args>(args)...);        }
       
       // operator<<
       friend std::ostream& operator<<(std::ostream& os, const BTree<Trait>& tree)
       {               tree.Print(os);  return os;              }
       
       // Métodos del iterador bidireccional
       // begin(): Retorna iterador apuntando al elemento más pequeño (leftmost)
       // end(): Retorna iterador centinela (nullptr) que marca el final
       // rbegin(): Retorna iterador apuntando al elemento más grande (rightmost)
       // rend(): Retorna iterador centinela (nullptr) que marca el inicio al retroceder
       // Nota: Todos retornan el mismo tipo 'iterator', la dirección la controla
       //       el usuario usando operator++ (avanzar) u operator-- (retroceder)
       
       iterator begin()
       {
               if (m_NumKeys == 0)
                       return end();
               
               // Encontrar el elemento más a la izquierda (clave más pequeña)
               BTNode* page = &m_Root;
               while (page->m_SubPages[0])
                       page = page->m_SubPages[0];
               
               return iterator(page, 0);
       }
       
       iterator end()
       {
               return iterator(nullptr, 0);
       }
       
       iterator rbegin()
       {
               if (m_NumKeys == 0)
                       return rend();
               
               // Encontrar el elemento más a la derecha (clave más grande)
               BTNode* page = &m_Root;
               while (page->m_SubPages[page->m_KeyCount])
                       page = page->m_SubPages[page->m_KeyCount];
               
               return iterator(page, page->m_KeyCount - 1);
       }
       
       iterator rend()
       {
               return iterator(nullptr, 0);
       }

protected:
       BTNode          m_Root;
       size_t          m_Height;  // height of tree
       size_t          m_Order;   // order of tree
       size_t          m_NumKeys; // number of keys
       bool            m_Unique;  // Accept the elements only once ?
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

// Write: Save BTree with complete structure to stream
template <typename Trait>
std::ostream& BTree<Trait>::Write(std::ostream& os) const
{
       // Write metadata
       os << m_Order << " " << m_Height << " " << m_NumKeys << " " << m_Unique << "\n";
       
       // Write tree structure recursively
       const_cast<BTNode&>(m_Root).WriteStructure(os);
       
       return os;
}

// Read: Load BTree with complete structure from stream
template <typename Trait>
std::istream& BTree<Trait>::Read(std::istream& is)
{
       // Read metadata
       is >> m_Order >> m_Height >> m_NumKeys >> m_Unique;
       
       // Reinitialize tree
       m_Root.Reset();
       m_Root = BTNode(2 * m_Order + 1, m_Unique);
       m_Root.SetMaxKeysForChilds(m_Order);
       
       // Read tree structure recursively
       m_Root.ReadStructure(is);
       
       return is;
}

// Implementación de operator++ (avanza en orden ascendente)
template <typename Trait>
BTreeIterator<Trait>& BTreeIterator<Trait>::operator++()
{
       if (!m_CurrentPage) return *this;
       
       // 1. Si hay hijo derecho, descender al hijo más a la izquierda
       BTPage* rightChild = m_CurrentPage->m_SubPages[m_CurrentIndex + 1];
       if (rightChild != nullptr) {
               BTPage* next = rightChild;
               while (next->m_SubPages[0] != nullptr) {
                       next = next->m_SubPages[0];
               }
               m_CurrentPage = next;
               m_CurrentIndex = 0;
               return *this;
       }
       
       // 2. Si hay más keys en el nodo actual, avanzar a la derecha
       if (m_CurrentIndex + 1 < m_CurrentPage->m_KeyCount) {
               m_CurrentIndex++;
               return *this;
       }
       
       // 3. Subir al padre hasta encontrar una key no visitada
       BTPage* child = m_CurrentPage;
       BTPage* parent = m_CurrentPage->m_Parent;
       
       while (parent != nullptr) {
               // Encontrar la posición del hijo en el padre
               size_t childPos = 0;
               while (childPos <= parent->m_KeyCount && parent->m_SubPages[childPos] != child) {
                       childPos++;
               }
               
               // Si child está en SubPages[i], la siguiente key es Keys[i]
               if (childPos < parent->m_KeyCount) {
                       m_CurrentPage = parent;
                       m_CurrentIndex = childPos;
                       return *this;
               }
               
               // Seguir subiendo
               child = parent;
               parent = parent->m_Parent;
       }
       
       // No hay más elementos - alcanzamos el final
       m_CurrentPage = nullptr;
       m_CurrentIndex = 0;
       return *this;
}

// Implementación de operator-- (retrocede en orden ascendente)
template <typename Trait>
BTreeIterator<Trait>& BTreeIterator<Trait>::operator--()
{
       if (!m_CurrentPage) return *this;
       
       // 1. Si hay hijo izquierdo, descender al hijo más a la derecha
       BTPage* leftChild = m_CurrentPage->m_SubPages[m_CurrentIndex];
       if (leftChild != nullptr) {
               BTPage* next = leftChild;
               while (next->m_SubPages[next->m_KeyCount] != nullptr) {
                       next = next->m_SubPages[next->m_KeyCount];
               }
               m_CurrentPage = next;
               m_CurrentIndex = next->m_KeyCount - 1;
               return *this;
       }
       
       // 2. Si hay más keys a la izquierda en el nodo actual, retroceder
       if (m_CurrentIndex > 0) {
               m_CurrentIndex--;
               return *this;
       }
       
       // 3. Subir al padre hasta encontrar una key no visitada
       BTPage* child = m_CurrentPage;
       BTPage* parent = m_CurrentPage->m_Parent;
       
       while (parent != nullptr) {
               // Encontrar la posición del hijo en el padre
               size_t childPos = 0;
               while (childPos <= parent->m_KeyCount && parent->m_SubPages[childPos] != child) {
                       childPos++;
               }
               
               // Si child está en SubPages[i] con i > 0, la key anterior es Keys[i-1]
               if (childPos > 0) {
                       m_CurrentPage = parent;
                       m_CurrentIndex = childPos - 1;
                       return *this;
               }
               
               // Seguir subiendo
               child = parent;
               parent = parent->m_Parent;
       }
       
       // No hay más elementos - alcanzamos el inicio
       m_CurrentPage = nullptr;
       m_CurrentIndex = 0;
       return *this;
}

#endif