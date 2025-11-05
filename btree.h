#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <fstream>
#include <string>
#include <shared_mutex>
#include <mutex>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5;

template <typename _keyType, typename _ObjIDType, typename _Compare>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       using Compare = _Compare;
};

template <typename Key, typename Value>
struct BTreeDescTrait
{
       using keyType = Key;
       using ObjIDType = Value;
       using Compare = std::greater<Key>;
};

template <typename Key, typename Value>
struct BTreeAscTrait
{
       using keyType = Key;
       using ObjIDType = Value;
       using Compare = std::less<Key>;
};

template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType keyType;
       typedef typename Trait::ObjIDType ObjIDType;
       typedef typename Trait::Compare Compare;

       typedef CBTreePage<Trait> BTNode; // useful shorthand

public:
       // typedef ObjectInfo iterator;
       typedef typename BTNode::lpfnForEach2 lpfnForEach2;
       typedef typename BTNode::lpfnForEach3 lpfnForEach3;
       typedef typename BTNode::lpfnFirstThat2 lpfnFirstThat2;
       typedef typename BTNode::lpfnFirstThat3 lpfnFirstThat3;
       typedef typename BTNode::ObjectInfo ObjectInfo;

public:
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
           : m_Order(order),
             m_Root(2 * order + 1, unique),
             m_Unique(unique),
             m_NumKeys(0)
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Height = 1;
       }
       ~BTree() {}

       template <typename Trait>
       BTree<Trait>::BTree(BTree &&other) noexcept
       {
              std::shared_lock<std::shared_mutex> lk(other.m_mutex);
              m_Root(std::move(other.m_Root)),
              m_Height(std::exchange(other.m_Height, 0)),
              m_Order(std::exchange(other.m_Order, 0)),
              m_NumKeys(std::exchange(other.m_NumKeys, 0)),
              m_Unique(std::exchange(other.m_Unique, true)),
              m_Compare(std::move(other.m_Compare))
       }

       // int           Open (char * name, int mode);
       // int           Create (char * name, int mode);
       // int           Close ();
       bool Insert(const keyType key, const long ObjID);
       bool Remove(const keyType key, const long ObjID);
       ObjIDType Search(const keyType key)
       {
              std::shared_lock<std::shared_mutex> lk(m_mutex);
              ObjIDType ObjID = -1;
              m_Root.Search(key, ObjID);
              return ObjID;
       }
       size_t size()
       {
              std::shared_lock<std::shared_mutex> lk(m_mutex);
              return m_NumKeys;
       }
       size_t height()
       {
              std::shared_lock<std::shared_mutex> lk(m_mutex);
              return m_Height;
       }
       size_t GetOrder()
       {
              std::shared_lock<std::shared_mutex> lk(m_mutex);
              return m_Order;
       }

       void Print(ostream &os)
       {
              std::shared_lock<std::shared_mutex> lk(m_mutex);
              m_Root.Print(os);
       }
       template <typename F, typename... Args>
       void ForEach(F &&f, Args &&...args)
       {
              std::shared_lock<std::shared_mutex> lk(m_mutex);
              m_Root.ForEach(std::forward<F>(f), 0, std::forward<Args>(args)...);
       }

       template <typename P, typename... Args>
       ObjectInfo *FirstThat(P &&pred, Args &&...args)
       {
              std::shared_lock<std::shared_mutex> lk(m_mutex);
              return m_Root.FirstThat(std::forward<P>(pred), 0, std::forward<Args>(args)...);
       }
       // typedef               ObjectInfo iterator;

       template <typename T>
       friend std::ostream &operator<<(std::ostream &os, BTree<Trait> &obj);

       bool Write(const std::string &filename) const;
       bool Write(std::ostream &os) const;
       bool Read(const std::string &filename);
       bool Read(std::istream &is);

private:
       void Clear();

protected:
       BTNode m_Root;
       size_t m_Height;  // height of tree
       size_t m_Order;   // order of tree
       size_t m_NumKeys; // number of keys
       bool m_Unique;    // Accept the elements only once ?
       Compare m_Compare;
       mutable std::shared_mutex m_mutex;
};

template <typename Trait>
std::ostream &operator<<(std::ostream &os, BTree<Trait> &obj)
{
       obj.Print(os);
       return os;
}

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const long ObjID)
{
       std::unique_lock<std::shared_mutex> lk(m_mutex);
       bt_ErrorCode error = m_Root.Insert(key, ObjID);
       if (error == bt_duplicate)
              return false;
       m_NumKeys++;
       if (error == bt_overflow)
       {
              m_Root.SplitRoot();
              m_Height++;
       }
       return true;
}

template <typename Trait>
bool BTree<Trait>::Remove(const keyType key, const long ObjID)
{
       std::unique_lock<std::shared_mutex> lk(m_mutex);
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if (error == bt_duplicate || error == bt_nofound)
              return false;
       m_NumKeys--;

       if (error == bt_rootmerged)
              m_Height--;
       return true;
}

template <typename Trait>
void BTree<Trait>::Clear()
{
       m_Root.Reset();
       m_Root.Create();
       m_Root.SetMaxKeysForChilds(m_Order);
       m_NumKeys = 0;
       m_Height = 1;
}

template <typename Trait>
bool BTree<Trait>::Write(const std::string &filename) const
{
       std::ofstream ofs(filename);
       if (!ofs)
              return false;
       Write(ofs);
       return true;
}

template <typename Trait>
bool BTree<Trait>::Write(std::ostream &os) const
{
       std::shared_lock<std::shared_mutex> lk(m_mutex);
       os << m_Order << " " << m_Height << " " << m_NumKeys << " " << m_Unique << "\n";
       m_Root.Write(os);
       return true;
}

template <typename Trait>
bool BTree<Trait>::Read(const std::string &filename)
{
       std::ifstream ifs(filename);
       if (!ifs)
              return false;
       return Read(ifs);
}

template <typename Trait>
bool BTree<Trait>::Read(std::istream &is)
{
       std::unique_lock<std::shared_mutex> lk(m_mutex);
       size_t order, height, numKeys;
       bool unique;
       is >> order >> height >> numKeys >> unique;

       Clear();

       m_Order = order;
       m_Height = height;
       m_NumKeys = numKeys;
       m_Unique = unique;

       m_Root = BTNode(2 * order + 1, unique);
       m_Root.SetMaxKeysForChilds(order);
       m_Root.Read(is);
       return true;
}

#endif