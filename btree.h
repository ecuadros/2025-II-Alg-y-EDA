#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <utility>  // Para std::move
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

template <typename _keyType, typename _ObjIDType, typename _Compare = std::less<_keyType>>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       using Compare = _Compare;
       // TODO: agregar funcion de comparacion
};

template <typename Trait>
class BTree // this is the full version of the BTree
{ 
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;
       typedef typename Trait::Compare    Compare;
       
       typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
       // Typedefs de iteradores (siguiendo el patrón de doublelinkedlist.h)
       typedef forward_btree_iterator<Trait>    iterator;
       typedef backward_btree_iterator<Trait>   reverse_iterator;

       typedef typename BTNode::lpfnForEach2    lpfnForEach2;
       typedef typename BTNode::lpfnForEach3    lpfnForEach3;
       typedef typename BTNode::lpfnFirstThat2  lpfnFirstThat2;
       typedef typename BTNode::lpfnFirstThat3  lpfnFirstThat3;
       typedef typename BTNode::ObjectInfo      ObjectInfo;

       // Friend declarations para los iteradores
       friend class forward_btree_iterator<Trait>;
       friend class backward_btree_iterator<Trait>;

public:
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
              : m_Order(order),
                m_Root(2 * order  + 1, unique),
                m_Unique(unique),
                m_NumKeys(0)
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Root.SetParent(nullptr);  // Root no tiene padre
              m_Height = 1;
       }

       // Delete copy constructor and copy assignment (evitar copias accidentales)
       BTree(const BTree&) = delete;
       BTree& operator=(const BTree&) = delete;

       // Move Constructor
       BTree(BTree&& other) noexcept
              : m_Order(other.m_Order),
                m_Root(std::move(other.m_Root)),
                m_Height(other.m_Height),
                m_Unique(other.m_Unique),
                m_NumKeys(other.m_NumKeys)
       {
              // El root no debe tener padre
              m_Root.SetParent(nullptr);
              
              // Reset other to a valid but empty state
              other.m_Height = 1;
              other.m_NumKeys = 0;
       }

       // Move Assignment Operator
       BTree& operator=(BTree&& other) noexcept
       {
              if (this != &other) {
                     // Move data from other
                     m_Order = other.m_Order;
                     m_Root = std::move(other.m_Root);
                     m_Height = other.m_Height;
                     m_Unique = other.m_Unique;
                     m_NumKeys = other.m_NumKeys;

                     // El root no debe tener padre
                     m_Root.SetParent(nullptr);

                     // Reset other to a valid but empty state
                     other.m_Height = 1;
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

       void            Print (ostream &os)
       {               m_Root.Print(os);                              }
       void            ForEach( lpfnForEach2 lpfn, void *pExtra1 )
       {               m_Root.ForEach(lpfn, 0, pExtra1);              }
       void            ForEach( lpfnForEach3 lpfn, void *pExtra1, void *pExtra2)
       {               m_Root.ForEach(lpfn, 0, pExtra1, pExtra2);     }
       ObjectInfo*     FirstThat( lpfnFirstThat2 lpfn, void *pExtra1 )
       {               return m_Root.FirstThat(lpfn, 0, pExtra1);     }
       ObjectInfo*     FirstThat( lpfnFirstThat3 lpfn, void *pExtra1, void *pExtra2)
       {               return m_Root.FirstThat(lpfn, 0, pExtra1, pExtra2);   }

       // Iteradores forward (in-order traversal: orden ascendente)
       iterator begin()
       {
               if (m_NumKeys == 0)
                       return end();
               return iterator(this, &m_Root, 0);
       }

       iterator end()
       {
               return iterator(this, nullptr, 0);
       }

       // Iteradores reverse (reverse in-order: orden descendente)
       reverse_iterator rbegin()
       {
               if (m_NumKeys == 0)
                       return rend();

               // Ir al último elemento (nodo más a la derecha)
               BTNode* node = &m_Root;
               while (node->m_SubPages[node->GetNumberOfKeys()])
                       node = node->m_SubPages[node->GetNumberOfKeys()];

               return reverse_iterator(this, node, node->GetNumberOfKeys() - 1);
       }

       reverse_iterator rend()
       {
               return reverse_iterator(this, nullptr, 0);
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

#endif