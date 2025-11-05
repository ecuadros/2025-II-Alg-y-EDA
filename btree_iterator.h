#ifndef __BTREE_ITERATOR_H__
#define __BTREE_ITERATOR_H__

#include "btreepage.h"

template <typename Trait>
class CBTreePage;

template <typename Trait>
class btree_iterator
{
    friend class BTree<Trait>;
    friend class CBTreePage<Trait>;

public:
    using BTPage = CBTreePage<Trait>;
    using ObjectInfo = typename BTPage::ObjectInfo;

private:
    // state is a pointer to current node and index within node
    BTPage* m_node;
    size_t  m_pos;

    btree_iterator(BTPage* node, size_t pos)
        : m_node(node), m_pos(pos) {}

public:
    btree_iterator() : m_node(nullptr), m_pos(0) {}

    ObjectInfo& operator*() const
    {
        return m_node->m_Keys[m_pos];
    }

    ObjectInfo* operator->() const
    {
        return &(m_node->m_Keys[m_pos]);
    }

    // inorder
    btree_iterator& operator++()
    {
        BTPage* right_subtree = m_node->m_SubPages[m_pos + 1];
        if (right_subtree)
        {
            m_node = right_subtree->GetLeftmostLeaf();
            m_pos = 0;
        }
        else
        {
            m_pos++;

            if (m_pos < m_node->m_KeyCount)
            {
                return *this;
            }

            BTPage* child = m_node;
            m_node = m_node->m_Parent;

            while (m_node != nullptr && child == m_node->m_SubPages[m_node->m_KeyCount])
            {
                child = m_node;
                m_node = m_node->m_Parent;
            }

            if (m_node == nullptr)
            {
                m_pos = 0;
            }
            else
            {
                for (m_pos = 0; m_pos <= m_node->m_KeyCount; m_pos++)
                {
                    if (m_node->m_SubPages[m_pos] == child)
                    {
                        break;
                    }
                }
            }
        }
        return *this;
    }

    btree_iterator operator++(int)
    {
        btree_iterator old = *this; // copy actual state
        operator++();
        return old;                 // return old state
    }


    bool operator==(const btree_iterator& other) const
    {
        return m_node == other.m_node && m_pos == other.m_pos;
    }

    bool operator!=(const btree_iterator& other) const
    {
        return !(*this == other);
    }

    BTPage* getCurrentPage() const { return m_node; }
    size_t getCurrentIndex() const { return m_pos; }
};

template <typename Trait>
class btree_reverse_iterator
{
    friend class BTree<Trait>;
    friend class CBTreePage<Trait>;

public:
    using BTPage = CBTreePage<Trait>;
    using ObjectInfo = typename BTPage::ObjectInfo;

private:
    BTPage* m_node;
    size_t  m_pos;

    btree_reverse_iterator(BTPage* node, size_t pos)
        : m_node(node), m_pos(pos) {}

public:
    btree_reverse_iterator() : m_node(nullptr), m_pos(0) {}

    ObjectInfo& operator*() const
    {
        return m_node->m_Keys[m_pos];
    }

    ObjectInfo* operator->() const
    {
        return &(m_node->m_Keys[m_pos]);
    }

    // reverse in-order right -> key -> left
    btree_reverse_iterator& operator++()
    {
        BTPage* left_subtree = m_node->m_SubPages[m_pos];
        if (left_subtree)
        {
            m_node = left_subtree->GetRightmostLeaf();
            m_pos = m_node->m_KeyCount - 1;
        }
        else
        {
            if (m_pos > 0)
            {
                m_pos--;
                return *this;
            }

            BTPage* child = m_node;
            m_node = m_node->m_Parent;

            while (m_node != nullptr && child == m_node->m_SubPages[0])
            {
                child = m_node;
                m_node = m_node->m_Parent;
            }

            if (m_node == nullptr)
            {
                m_pos = 0;
            }
            else
            {
                for (m_pos = 0; m_pos < m_node->m_KeyCount; m_pos++)
                {
                    if (m_node->m_SubPages[m_pos + 1] == child)
                    {
                        break;
                    }
                }
            }
        }
        return *this;
    }

    btree_reverse_iterator operator++(int)
    {
        btree_reverse_iterator old = *this;
        operator++();
        return old;
    }

    bool operator==(const btree_reverse_iterator& other) const
    {
        return m_node == other.m_node && m_pos == other.m_pos;
    }

    bool operator!=(const btree_reverse_iterator& other) const
    {
        return !(*this == other);
    }

    BTPage* getCurrentPage() const { return m_node; }
    size_t getCurrentIndex() const { return m_pos; }
};

#endif // __BTREE_ITERATOR_H__