#ifndef __RTREE_H__
#define __RTREE_H__

#include <iostream>
#include <vector>
#include "rtreenode.h"

#define DEFAULT_RTREE_MAX_ENTRIES 4

template <typename _CoordType, typename _ObjIDType>
struct RTreeTrait {
    using CoordType = _CoordType;
    using ObjIDType = _ObjIDType;
};

template <typename Trait>
class CRTree {
    typedef typename Trait::CoordType CoordType;
    typedef typename Trait::ObjIDType ObjIDType;
    typedef CRTreeNode<Trait> RTNode;

public:
    CRTree(size_t maxEntries = DEFAULT_RTREE_MAX_ENTRIES);
    ~CRTree();

    bool Insert(const Rectangle<CoordType>& rect, const ObjIDType objID);
    bool Remove(const Rectangle<CoordType>& rect, const ObjIDType objID);
    void RangeQuery(const Rectangle<CoordType>& range, std::vector<ObjIDType>& results);

    size_t GetHeight() const { return m_Height; }
    size_t GetSize() const { return m_Size; }

protected:
    RTNode* m_Root;
    size_t m_Height;
    size_t m_MaxEntries;
    size_t m_Size;

    void DestroyTree(RTNode* node);
};

template <typename Trait>
CRTree<Trait>::CRTree(size_t maxEntries)
    : m_MaxEntries(maxEntries), m_Height(1), m_Size(0) {
    m_Root = new RTNode(maxEntries, true);
}

template <typename Trait>
CRTree<Trait>::~CRTree() {
    DestroyTree(m_Root);
}

template <typename Trait>
void CRTree<Trait>::DestroyTree(RTNode* node) {
    if (node) {
        delete node;
    }
}

template <typename Trait>
bool CRTree<Trait>::Insert(const Rectangle<CoordType>& rect, const ObjIDType objID) {
    rt_ErrorCode error = m_Root->Insert(rect, objID);

    if (error == rt_overflow) {
        RTNode* newRoot = new RTNode(m_MaxEntries, false);
        RTNode* newNode = nullptr;

        m_Root->SplitNode(newNode);

        newRoot->m_Children[0] = m_Root;
        newRoot->m_Entries[0] = tagRTreeEntry<CoordType, ObjIDType>(m_Root->GetMBR(), -1);
        newRoot->m_Count++;

        if (newNode) {
            newRoot->m_Children[1] = newNode;
            newRoot->m_Entries[1] = tagRTreeEntry<CoordType, ObjIDType>(newNode->GetMBR(), -1);
            newRoot->m_Count++;
        }

        newRoot->UpdateMBR();
        m_Root = newRoot;
        m_Height++;
    }

    if (error != rt_duplicate) {
        m_Size++;
        return true;
    }

    return false;
}

template <typename Trait>
bool CRTree<Trait>::Remove(const Rectangle<CoordType>& rect, const ObjIDType objID) {
    rt_ErrorCode error = m_Root->Remove(rect, objID);

    if (error == rt_ok || error == rt_underflow) {
        m_Size--;
        return true;
    }

    return false;
}

template <typename Trait>
void CRTree<Trait>::RangeQuery(const Rectangle<CoordType>& range, std::vector<ObjIDType>& results) {
    m_Root->RangeQuery(range, results);
}

#endif
