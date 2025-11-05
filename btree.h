#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

template <typename _keyType, typename _ObjIDType>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;

       // Comparison function: returns true if a == b
       static bool isEqual(const keyType& a, const keyType& b) {
              return a == b;
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

       class Iterator {
       private:
              std::vector<ObjectInfo*> items;
              size_t index;
              bool reverse;

       public:
              Iterator(const std::vector<ObjectInfo*>& vec, size_t idx, bool rev = false)
                     : items(vec), index(idx), reverse(rev) {}

              ObjectInfo& operator*() { return *items[index]; }
              ObjectInfo* operator->() { return items[index]; }

              Iterator& operator++() {
                     if (reverse) index--;
                     else index++;
                     return *this;
              }

              Iterator operator++(int) {
                     Iterator tmp = *this;
                     ++(*this);
                     return tmp;
              }

              bool operator==(const Iterator& other) const {
                     return index == other.index;
              }

              bool operator!=(const Iterator& other) const {
                     return index != other.index;
              }
       };

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

       BTree(BTree&& other) noexcept
              : m_Root(std::move(other.m_Root)),
                m_Height(other.m_Height),
                m_Order(other.m_Order),
                m_NumKeys(other.m_NumKeys),
                m_Unique(other.m_Unique)
       {
              other.m_Height = 1;
              other.m_NumKeys = 0;
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

       template<typename Function, typename... Args>
       void ForEach(Function func, Args const&... args)
       {               m_Root.ForEach(func, 0, args...);              }

       template<typename Predicate, typename... Args>
       ObjectInfo* FirstThat(Predicate pred, Args const&... args)
       {               return m_Root.FirstThat(pred, 0, args...);     }

       Iterator begin() {
              collectItems.clear();
              m_Root.ForEach([](ObjectInfo& obj, size_t level, std::vector<ObjectInfo*>* vec) {
                     vec->push_back(&obj);
              }, 0, &collectItems);
              return Iterator(collectItems, 0, false);
       }

       Iterator end() {
              return Iterator(collectItems, collectItems.size(), false);
       }

       Iterator rbegin() {
              collectItems.clear();
              m_Root.ForEach([](ObjectInfo& obj, size_t level, std::vector<ObjectInfo*>* vec) {
                     vec->push_back(&obj);
              }, 0, &collectItems);
              return Iterator(collectItems, collectItems.size() - 1, true);
       }

       Iterator rend() {
              return Iterator(collectItems, (size_t)-1, true);
       }

protected:
       std::vector<ObjectInfo*> collectItems;
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

template <typename Trait>
std::ostream& operator<<(std::ostream& os, BTree<Trait>& bt)
{
       bt.Print(os);
       return os;
}

#endif