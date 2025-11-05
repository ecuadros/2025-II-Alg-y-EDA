#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <fstream>
#include <memory>
#include <functional>
#include <stdexcept>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5;

template <typename _keyType, typename _ObjIDType>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       struct Compare
       {
              bool operator()(const keyType &a, const keyType &b) const
              {
                     return a < b;
              }
       };
};

template <typename Trait>
class BTree
{
       typedef typename Trait::keyType keyType;
       typedef typename Trait::ObjIDType ObjIDType;
       typedef CBTreePage<Trait> BTNode;

public:
       typedef typename BTNode::lpfnForEach2 lpfnForEach2;
       typedef typename BTNode::lpfnForEach3 lpfnForEach3;
       typedef typename BTNode::lpfnFirstThat2 lpfnFirstThat2;
       typedef typename BTNode::lpfnFirstThat3 lpfnFirstThat3;
       typedef typename BTNode::ObjectInfo ObjectInfo;

protected:
       template <bool Reverse>
       class iterator_base
       {
       public:
              using iterator_category = std::bidirectional_iterator_tag;
              using value_type = ObjectInfo;
              using reference = ObjectInfo &;
              using pointer = ObjectInfo *;
              using difference_type = std::ptrdiff_t;

              iterator_base() : m_tree(nullptr), m_pos(-1), m_seq(nullptr) {}

              reference operator*() const
              {
                     if (!valid())
                            throw std::runtime_error("Invalid iterator");
                     return m_seq->at(m_pos).first->m_Keys[m_seq->at(m_pos).second];
              }

              pointer operator->() const
              {
                     if (!valid())
                            throw std::runtime_error("Invalid iterator");
                     return &m_seq->at(m_pos).first->m_Keys[m_seq->at(m_pos).second];
              }

              iterator_base &operator++()
              {
                     if (!m_seq)
                            return *this;
                     if constexpr (!Reverse)
                     {
                            if (valid())
                                   ++m_pos;
                     }
                     else
                     {
                            --m_pos;
                     }
                     return *this;
              }

              iterator_base operator++(int)
              {
                     iterator_base tmp = *this;
                     ++*this;
                     return tmp;
              }

              iterator_base &operator--()
              {
                     if (!m_seq)
                            return *this;
                     if constexpr (!Reverse)
                     {
                            --m_pos;
                     }
                     else
                     {
                            if (m_pos < (difference_type)m_seq->size())
                                   ++m_pos;
                     }
                     return *this;
              }

              iterator_base operator--(int)
              {
                     iterator_base tmp = *this;
                     --*this;
                     return tmp;
              }

              bool operator==(const iterator_base &other) const
              {
                     // Two iterators are equal if they point to the same tree and have the same position
                     if (m_tree != other.m_tree)
                            return false;
                     if (!m_seq && !other.m_seq)
                            return true;
                     if (!m_seq || !other.m_seq)
                            return false;
                     return m_pos == other.m_pos;
              }

              bool operator!=(const iterator_base &other) const
              {
                     return !(*this == other);
              }

       private:
              friend class BTree;
              using Entry = std::pair<BTNode *, size_t>;

              iterator_base(BTree *tree, bool at_begin) : m_tree(tree), m_pos(-1), m_seq(nullptr)
              {
                     if (!tree)
                            return;

                     m_seq = build_sequence(tree);
                     if (!m_seq || m_seq->empty())
                            return;

                     if constexpr (!Reverse)
                     {
                            m_pos = at_begin ? 0 : (difference_type)m_seq->size();
                     }
                     else
                     {
                            m_pos = at_begin ? ((difference_type)m_seq->size() - 1) : -1;
                     }
              }

              bool valid() const
              {
                     return m_seq && m_pos >= 0 && m_pos < (difference_type)m_seq->size();
              }

              static std::shared_ptr<std::vector<Entry>> build_sequence(BTree *tree)
              {
                     auto seq = std::make_shared<std::vector<Entry>>();
                     if (!tree)
                            return seq;

                     std::function<void(BTNode *)> dfs = [&](BTNode *n)
                     {
                            if (!n || n->m_KeyCount == 0)
                                   return;
                            for (size_t i = 0; i < n->m_KeyCount; ++i)
                            {
                                   if (n->m_SubPages[i])
                                          dfs(n->m_SubPages[i]);
                                   seq->push_back({n, i});
                            }
                            if (n->m_SubPages[n->m_KeyCount])
                                   dfs(n->m_SubPages[n->m_KeyCount]);
                     };

                     dfs(&tree->m_Root);
                     return seq;
              }

              BTree *m_tree;
              difference_type m_pos;
              std::shared_ptr<std::vector<Entry>> m_seq;
       };

public:
       using iterator = iterator_base<false>;
       using reverse_iterator = iterator_base<true>;

       iterator begin() { return iterator(this, true); }
       iterator end() { return iterator(this, false); }
       reverse_iterator rbegin() { return reverse_iterator(this, true); }
       reverse_iterator rend() { return reverse_iterator(this, false); }

public:
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
           : m_Root(2 * order + 1, unique),
             m_Order(order),
             m_NumKeys(0),
             m_Unique(unique)
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Height = 1;
       }

       ~BTree() {}

       bool Insert(const keyType key, const long ObjID);
       bool Remove(const keyType key, const long ObjID);
       ObjIDType Search(const keyType key)
       {
              ObjIDType ObjID = -1;
              m_Root.Search(key, ObjID);
              return ObjID;
       }

       size_t size() { return m_NumKeys; }
       size_t height() { return m_Height; }
       size_t GetOrder() { return m_Order; }

       void Print(ostream &os)
       {
              m_Root.Print(os);
       }

       template <class Fn, class... Args>
       void ForEachT(Fn &&fn, Args &&...args)
       {
              m_Root.ForEachT(std::forward<Fn>(fn), 0,
                              std::forward<Args>(args)...);
       }

       void ForEach(lpfnForEach2 lpfn, void *pExtra1)
       {
              m_Root.ForEach(lpfn, 0, pExtra1);
       }

       void ForEach(lpfnForEach3 lpfn, void *pExtra1, void *pExtra2)
       {
              m_Root.ForEach(lpfn, 0, pExtra1, pExtra2);
       }

       ObjectInfo *FirstThat(lpfnFirstThat2 lpfn, void *pExtra1)
       {
              return m_Root.FirstThat(lpfn, 0, pExtra1);
       }

       ObjectInfo *FirstThat(lpfnFirstThat3 lpfn, void *pExtra1, void *pExtra2)
       {
              return m_Root.FirstThat(lpfn, 0, pExtra1, pExtra2);
       }

public:
       BTree(BTree &&other) noexcept;
       BTree &operator=(BTree &&other) noexcept;
       BTree(const BTree &) = delete;
       BTree &operator=(const BTree &) = delete;

       bool Save(const std::string &filename) const;
       bool Load(const std::string &filename);
       size_t size() const { return m_NumKeys; }
       size_t height() const { return m_Height; }
       size_t GetOrder() const { return m_Order; }

       void Print(std::ostream &os) const
       {
              const_cast<BTNode &>(m_Root).Print(os);
       }

protected:
       BTNode m_Root;
       size_t m_Height;
       size_t m_Order;
       size_t m_NumKeys;
       bool m_Unique;
       size_t computeHeight(const BTNode &n) const;
       size_t computeSize(const BTNode &n) const;
};

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const long ObjID)
{
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
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if (error == bt_duplicate || error == bt_nofound)
              return false;
       m_NumKeys--;

       if (error == bt_rootmerged)
              m_Height--;
       return true;
}

template <typename Trait>
BTree<Trait>::BTree(BTree &&other) noexcept
    : m_Root(std::move(other.m_Root)),
      m_Height(other.m_Height),
      m_Order(other.m_Order),
      m_NumKeys(other.m_NumKeys),
      m_Unique(other.m_Unique)
{
       other.m_Height = 0;
       other.m_NumKeys = 0;
}

template <typename Trait>
BTree<Trait> &BTree<Trait>::operator=(BTree &&other) noexcept
{
       if (this != &other)
       {
              m_Root = std::move(other.m_Root);
              m_Height = other.m_Height;
              m_Order = other.m_Order;
              m_NumKeys = other.m_NumKeys;
              m_Unique = other.m_Unique;

              other.m_Height = 0;
              other.m_NumKeys = 0;
       }
       return *this;
}

template <typename Trait>
size_t BTree<Trait>::computeHeight(const BTNode &n) const
{
       if (!n.m_SubPages[0])
              return 1;
       size_t best = 0;
       for (size_t i = 0; i <= n.m_KeyCount; ++i)
       {
              if (n.m_SubPages[i])
              {
                     best = std::max(best, computeHeight(*n.m_SubPages[i]));
              }
       }
       return best + 1;
}

template <typename Trait>
size_t BTree<Trait>::computeSize(const BTNode &n) const
{
       size_t sum = n.m_KeyCount;
       for (size_t i = 0; i <= n.m_KeyCount; ++i)
       {
              if (n.m_SubPages[i])
                     sum += computeSize(*n.m_SubPages[i]);
       }
       return sum;
}

template <typename Trait>
bool BTree<Trait>::Save(const std::string &filename) const
{
       std::ofstream ofs(filename);
       if (!ofs)
              return false;

       ofs << m_Order << ' ' << (m_Unique ? 1 : 0) << '\n';
       m_Root.Write(ofs);
       return true;
}

template <typename Trait>
bool BTree<Trait>::Load(const std::string &filename)
{
       std::ifstream ifs(filename);
       if (!ifs)
              return false;

       size_t order = 0;
       int uniq = 1;
       ifs >> order >> uniq;

       m_Order = order ? order : m_Order;
       m_Unique = (uniq != 0);

       m_Root.SetMaxKeysForChilds(m_Order);
       m_Root.Reset();
       m_Root.Read(ifs);

       m_Height = computeHeight(m_Root);
       m_NumKeys = computeSize(m_Root);

       return true;
}

template <typename Trait>
std::ostream &operator<<(std::ostream &os, const BTree<Trait> &t)
{
       os << "size=" << t.size() << ", height=" << t.height() << "\n";
       t.Print(os);
       return os;
}

#endif