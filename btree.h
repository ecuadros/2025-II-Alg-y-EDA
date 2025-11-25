#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

template <typename _keyType, typename _ObjIDType, typename _CompareFn = std::less<_keyType>>
struct BTreeTrait
{
	using keyType = _keyType;
	using ObjIDType = _ObjIDType;
	using CompareFn = _CompareFn;
};

template <typename Trait>
class BTree // this is the full version of the BTree
{
	typedef typename Trait::keyType    keyType;
	typedef typename Trait::ObjIDType    ObjIDType;
	typedef typename Trait::CompareFn    CompareFn;

	typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
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

       // ForEach generalizado
       template <typename Func, typename... Args>
       void ForEach(Func&& func, Args&&... args)
       {               m_Root.ForEach(std::forward<Func>(func), 0, std::forward<Args>(args)...); }

       // FirstThat generalizado
       template <typename Pred, typename... Args>
       ObjectInfo* FirstThat(Pred&& pred, Args&&... args)
       {               return m_Root.FirstThat(std::forward<Pred>(pred), 0, std::forward<Args>(args)...); }

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