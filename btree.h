#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <functional>
#include "btreepage.h"
#include "btree_iterator.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

template <typename _keyType, typename _ObjIDType>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       using CompareFn = std::less<_keyType>;
};

template <typename _keyType, typename _ObjIDType>
struct BTreeAscTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       using CompareFn = std::less<_keyType>;
};

template <typename _keyType, typename _ObjIDType>
struct BTreeDescTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       using CompareFn = std::greater<_keyType>;
};

template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;
       typedef typename Trait::CompareFn    CompareFn;
       
       typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
       typedef btree_iterator<Trait> iterator;
       typedef btree_reverse_iterator<Trait> reverse_iterator;

       //typedef ObjectInfo iterator;
       typedef typename BTNode::lpfnForEach2    lpfnForEach2;
       typedef typename BTNode::lpfnForEach3    lpfnForEach3;
       typedef typename BTNode::lpfnFirstThat2  lpfnFirstThat2;
       typedef typename BTNode::lpfnFirstThat3  lpfnFirstThat3;
       typedef typename BTNode::ObjectInfo      ObjectInfo;

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
       //typedef               ObjectInfo iterator;

       template<typename Func, typename... Args>
       void            ForEach(Func&& func, Args&&... args)
       {               m_Root.ForEach(std::forward<Func>(func), 0, std::forward<Args>(args)...);}

       template<typename Func, typename... Args>
       ObjectInfo*     FirstThat(Func&& func, Args&&... args)
       {               return m_Root.FirstThat(std::forward<Func>(func), 0, std::forward<Args>(args)...);}

       void            Write(ostream &os) { os << *this;  }

       iterator        begin() {
              if (m_NumKeys == 0)
                     return end();
              BTNode* leftmost = m_Root.GetLeftmostLeaf();
              return iterator(leftmost, 0);
       }

       iterator        end() { return iterator(nullptr, 0); }

       reverse_iterator rbegin() {
              if (m_NumKeys == 0)
                     return rend();
              BTNode* rightmost = m_Root.GetRightmostLeaf();
              return reverse_iterator(rightmost, rightmost->m_KeyCount - 1);
       }

       reverse_iterator rend() { return reverse_iterator(nullptr, 0); }

protected:
       BTNode          m_Root;
       size_t          m_Height;  // height of tree
       size_t          m_Order;   // order of tree
       size_t          m_NumKeys; // number of keys
       bool            m_Unique;  // Accept the elements only once ?
       CompareFn       m_Compare;
};     

template <typename Trait>
ostream & operator<<(std::ostream &os, BTree<Trait> &obj){
       os << "BTree with order=" << obj.GetOrder() << ", keys=" << obj.size() << ", height=" << obj.height() << std::endl;
       obj.Print(os);
       return os;
}

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