#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <fstream>
#include <memory>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <iterator>
#include <string>
#include <utility>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

template <typename Trait>
class BTree;
template <typename Trait>
class BtreeIterator;
template <typename Trait>
class BTreeReverseIterator;

const size_t MaxHeight = 5;

template <typename _keyType, typename _ObjIDType, typename _Compare = std::less<_keyType>>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       using Compare = _Compare;
};

template <typename Key, typename Value>
using BTreeAscTrait = BTreeTrait<Key, Value, std::less<Key>>;

template <typename Key, typename Value>
using BTreeDescTrait = BTreeTrait<Key, Value, std::greater<Key>>;

template <typename Trait>
class BTreeIterator
{
       friend class BTree<Trait>;
       typedef CBTreePage<Trait> BTPage;
       typedef typename BTPage::ObjectInfo ObjectInfo;

private:
       BTPage *m_CurrentPage;
       size_t m_CurrentIndex;

       BTreeIterator(BTPage *page, size_t index) : m_CurrentPage(page), m_CurrentIndex(index) {}

public:
       using iterator_category = std::forward_iterator_tag;
       using value_type = ObjectInfo;
       using difference_type = std::ptrdiff_t;
       using pointer = ObjectInfo *;
       using reference = ObjectInfo &;

       BTreeIterator() : m_CurrentPage(nullptr), m_CurrentIndex(0) {}

       ObjectInfo &operator*() { return m_CurrentPage->m_Keys[m_CurrentIndex]; }
       ObjectInfo *operator->() { return &(m_CurrentPage->m_Keys[m_CurrentIndex]); }
       BTreeIterator &operator++();
       BTreeIterator operator++(int)
       {
              BTreeIterator temp = *this;
              ++(*this);
              return temp;
       }

       bool operator==(cosnt BTreeIterator &other) const
       {
              if (m_CurrentPage == nullptr && other.m_CurrentPage == nullptr)
                     return true;
              return m_CurrentPage == other.m_CurrentPage && m_CurrentIndex == other.m_CurrentIndex;
       }

       bool operator!=(const BTreeIterator &other) const
       {
              return !(*this == other);
       }
};

template <typename Trait>
class BTreeReverseIterator
{
       friend class BTree<Trait>;
       typedef CBTreePage<Trait> BTPage;
       typedef typename BTPage::ObjectInfo ObjectInfo;

private:
       BTPage *m_CurrentPage;
       size_t m_CurrentIndex;

       BTreeReverseIterator(BTPage *page, size_t index) : m_CurrentPage(page), m_CurrentIndex(index) {}

public:
       using iterator_category = std::forward_iterator_tag;
       using value_type = ObjectInfo;
       using difference_type = std::ptrdiff_t;
       using pointer = ObjectInfo *;
       using reference = ObjectInfo &;

       BTreeReverseIterator() : m_CurrentPage(nullptr), m_CurrentIndex(0) {}

       ObjectInfo &operator*() { return m_CurrentPage->m_Keys[m_CurrentIndex]; }
       ObjectInfo *operator->() { return &(m_CurrentPage->m_Keys[m_CurrentIndex]); }
       BTreeReverseIterator &operator++();
       BTreeReverseIterator operator++(int)
       {
              BTreeReverseIterator temp = *this;
              ++(*this);
              return temp;
       }

       bool operator==(const BTreeReverseIterator &other) const
       {
              if (m_CurrentPAge == nullptr && other.m_CurrentPage == nullptr)
                     return true;
              return m_CurrentPage == other.m_CurrentPage && m_CurrentIndex == other.m_CurrentIndex;
       }

       bool operator!=(const BTreeReverseIterator &other) const
       {
              return !(*this == other);
       }
};

template <typename Trait>
class BTree
{
       typedef typename Trait::keyType keyType;
       typedef typename Trait : ObjIDType ObjIDType;
       typedef CBTreePage<Trait> BTNode;

public:
       typedef typename BTNode::ObjectInfo ObjectInfo;
       friend class BTreeIterator<Trait>;
       friend class BTReeReverseIterator<Trait>;
       typedef BTreeIterator<Trait> iterator;
       typedef BTreeReverseIterator<Trait> reverse_iterator;

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

       BTree(BTree &&other) noexcept
       {
              std::unique_lock<std::shared_mutex> lock(other.m_Mutex);
              m_Root 0 std::move(other.m_Root);
              m_Height = std::exchange(other.m_Height, 1);
              m_Unique = other.m_Unique;
              m_NumKeys = std::exchange(other.m_NumKeys, 0);
       }

       BTree &operator=(BTree &&other) noexcept
       {
              if (this != &other)
              {
                     std::unique_lock<std::shared_mutex> lock1(m_Mutex, std::defer_lock);
                     std::unique_lock<std::shared_mutex> lock2(other.m_Mutex, std::defer_lock);
                     std::lock(lock1, lock2);
                     m_Order = other.m_Order;
                     m_Root = std::move(other.m_Root);
                     m_Height = std::exchange(other.m_Height, 1);
                     m_Unique = other.m_Unique;
                     m_NumKeys = std::exchange(other.m_NumKeys, 0);
              }
              return *this;
       }

       ~BTree() {}

       bool Insert(const keyType key, const long ObjID);
       bool Remove(const keyType key, const long ObjID);

       ObjIDType Search(const keyType key)
       {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              ObjIDType ObjID = -1;
              m_Root.Search(key, ObjID);
              return ObjID;
       }

       size_t size() const
       {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              return m_NumKeys;
       }

       size_t height() const
       {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              return m_Height;
       }

       size_t GetOrder() const { return m_Order; }

       void Print(ostream &os) const
       {
              std::shared_lock<std::mutex> lock(m_Mutex);
              m_Root.Print(os);
       }

       std::ostream &Write(std::ostream &os) const;
       std::istream &Read(std::istream &is);

       template <typename Func, typename... Args>
       void ForEach(Func &&func, Args &&..args)
       {
              m_Root.ForEach(std::forward<Func>(func), 0, std::forward<Args>(args)...);
       }

       template <typename Pred, typename... Args>
       ObjectInfo *FirsThat(Pred &&predicatem Args &&...args)
       {
              return m_Root.FirstThat(std::forward<Pred>(predicate), 0, std::forward<Args>(args)...);
       }

       friend std::ostream &operator<<(std::ostream &os, const BTree<Trait> &tree)
       {
              tree.Print(os);
              return os;
       }

       iterator begin()
       {
              std::shared_lock<std::shared_mutex> lock(m_Mutex)

                  if (m_NumKeys == 0) return end();

              BTNode *page = &m_Root;
              while (page->m_SubPages[0])
                     page = page->m_SubPages[0];

              return iterator(page, 0);
       }

       iterator end()
       {
              return iterator(nullptr, 0);
       }

       reverse_iterator rbegin()
       {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);

              if (m_NumKeys == 0)
                     return rend();

              BTNode *page = &m_Root;
              while (page->m_SubPages[page->m_KeyCount])
                     page = page->m_SubPages[page->m_KeyCount];

              return reverse_iterator(page, page->m_KeyCount - 1);
       }

       reverse_iterator rend()
       {
              return reverse_iterator(nullptr, 0);
       }

protected:
       BTNode m_Root;
       size_t m_Height;
       size_t m_Order;
       size_t m_NumKeys;
       bool m_Unique;
       mutable std::shared_mutex m_Mutex;
};

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const long ObjID)
{
       std::unique_lock<std::shared_mutex> lock(m_Mutex);

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
       std::unique_lock<std::shared_mutex> lock(m_Mutex);

       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if (error == bt_duplicate || error == bt_nofound)
              return false;
       m_NumKeys--;

       if (error == bt_rootmerged)
              m_Height--;
       return true;
}

template <typename Trait>
std::ostream &BTree<Trait>::Write(std::ostream &os) const
{
       std::shared_lock<std::shared_mutex> lock(m_Mutex);

       os << m_Order << " " << m_Height << " " << m_NumKeys << " " << m_Unique << "\n";

       const_cast<BTNode &>(m_Root).WriteStructure(os);

       return os;
}

template <typename Trait>
std::istream &BTree<Trait>::Read(std::istream &is)
{
       std::unique_lock<std::shared_mutex> lock(m_Mutex);

       is >> m_Order >> m_Height >> m_NumKeys >> m_Unique;

       m_Root.Reset();
       m_Root = BTNode(2 * m_Order + 1, m_Unique);
       m_Root.SetMaxKeysForChilds(m_Order);

       m_Root.ReadStructure(is);

       return is;
}

template <typename Trait>
BTreeIterator<Trait> &BTreeIterator<Trait>::operator++()
{
       if (!m_CurrentPage)
              return *this;

       BTPage *rightChild = m_CurrentPage->m_SubPages[m_CurrentIndex + 1];
       if (rightChild != nullptr)
       {
              BTPage *leftmost = rightChild;
              while (leftmost->m_SubPages[0] != nullptr)
              {
                     leftmost = leftmost->m_SubPages[0];
              }
              m_CurrentPage = leftmost;
              m_CurrentIndex = 0;
              return *this;
       }

       if (m_CurrentIndex + 1 < m_CurrentPage->m_KeyCount)
       {
              m_CurrentIndex++;
              return *this;
       }

       BTPage *child = m_CurrentPage;
       BTPage *parent = m_CurrentPage->m_Parent;

       while (parent != nullptr)
       {
              size_t childPos = 0;
              while (childPos <= parent->m_KeyCount && parent->m_SubPages[childPos] != child)
              {
                     childPos++;
              }

              if (childPos < parent->m_KeyCount)
              {
                     m_CurrentPage = parent;
                     m_CurrentIndex = childPos;
                     return *this;
              }

              child = parent;
              parent = parent->m_Parent;
       }

       m_CurrentPage = nullptr;
       m_CurrentIndex = 0;
       return *this;
}

template <typename Trait>
BTreeReverseIterator<Trait> &BTreeReverseIterator<Trait>::operator++()
{
       if (!m_CurrentPage)
              return *this;

       BTPage *leftChild = m_CurrentPage->m_SubPages[m_CurrentIndex];
       if (leftChild != nullptr)
       {
              BTPage *rightmost = leftChild;
              while (rightmost->m_SubPages[rightmost->m_KeyCount] != nullptr)
              {
                     rightmost = rightmost->m_SubPages[rightmost->m_KeyCount];
              }
              m_CurrentPage = rightmost;
              m_CurrentIndex = rightmost->m_KeyCount - 1;
              return *this;
       }

       if (m_CurrentIndex > 0)
       {
              m_CurrentIndex--;
              return *this;
       }

       BTPage *child = m_CurrentPage;
       BTPage *parent = m_CurrentPage->m_Parent;

       while (parent != nullptr)
       {
              size_t childPos = 0;
              while (childPos <= parent->m_KeyCount && parent->m_SubPages[childPos] != child)
              {
                     childPos++;
              }

              if (childPos > 0)
              {
                     m_CurrentPage = parent;
                     m_CurrentIndex = childPos - 1;
                     return *this;
              }

              child = parent;
              parent = parent->m_Parent;
       }

       m_CurrentPage = nullptr;
       m_CurrentIndex = 0;
       return *this;
}

#endif