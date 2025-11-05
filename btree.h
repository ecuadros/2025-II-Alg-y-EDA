#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <utility>
#include <iterator> // Necesario para std::reverse_iterator
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

              BTreeIterator(BTree* pTree, BTNode* pNode = nullptr, size_t keyIndex = 0)
                     : m_pTree(pTree), m_pNode(pNode), m_keyIndex(keyIndex) {}

              reference operator*() const { return m_pNode->m_Keys[m_keyIndex]; }
              pointer operator->() const { return &m_pNode->m_Keys[m_keyIndex]; }

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

              bool operator==(const BTreeIterator& other) const { return m_pNode == other.m_pNode && m_keyIndex == other.m_keyIndex; }
              bool operator!=(const BTreeIterator& other) const { return !(*this == other); }

       private:
              BTree*  m_pTree;
              BTNode* m_pNode;
              size_t m_keyIndex;
       };

       using iterator = BTreeIterator;
       using reverse_iterator = std::reverse_iterator<iterator>;

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

       iterator begin() {
              BTNode* pNode = &m_Root;
              if (pNode->m_KeyCount == 0) { // el arbol esta vacio
                  return iterator(this, nullptr, 0); // Retorna end()
              }
              while (pNode->m_SubPages[0] != nullptr) { // Bajo al nodo mas a la izquierda
                     pNode = pNode->m_SubPages[0];
              }
              return iterator(this, pNode, 0);
       }
       iterator end() { return iterator(this, nullptr, 0); }

       reverse_iterator rbegin() { return reverse_iterator(end()); }
       reverse_iterator rend() { return reverse_iterator(begin()); }


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