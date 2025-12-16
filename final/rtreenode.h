#ifndef __RTREENODE_H__
#define __RTREENODE_H__

#include <vector>
#include <limits>
#include <algorithm>
#include <cmath>

template <typename Trait>
class CRTree;

enum rt_ErrorCode {rt_ok, rt_overflow, rt_underflow, rt_duplicate, rt_nofound};

template <typename CoordType>
struct Rectangle {
    CoordType min_x, min_y;
    CoordType max_x, max_y;

    Rectangle() : min_x(0), min_y(0), max_x(0), max_y(0) {}

    Rectangle(CoordType x1, CoordType y1, CoordType x2, CoordType y2)
        : min_x(x1), min_y(y1), max_x(x2), max_y(y2) {}

    CoordType area() const {
        return (max_x - min_x) * (max_y - min_y);
    }

    CoordType enlargement(const Rectangle& other) const {
        CoordType new_min_x = std::min(min_x, other.min_x);
        CoordType new_min_y = std::min(min_y, other.min_y);
        CoordType new_max_x = std::max(max_x, other.max_x);
        CoordType new_max_y = std::max(max_y, other.max_y);
        return (new_max_x - new_min_x) * (new_max_y - new_min_y) - area();
    }

    bool intersects(const Rectangle& other) const {
        return !(max_x < other.min_x || min_x > other.max_x ||
                max_y < other.min_y || min_y > other.max_y);
    }

    bool contains(const Rectangle& other) const {
        return min_x <= other.min_x && max_x >= other.max_x &&
               min_y <= other.min_y && max_y >= other.max_y;
    }

    void expand(const Rectangle& other) {
        min_x = std::min(min_x, other.min_x);
        min_y = std::min(min_y, other.min_y);
        max_x = std::max(max_x, other.max_x);
        max_y = std::max(max_y, other.max_y);
    }
};

template <typename CoordType, typename ObjIDType>
struct tagRTreeEntry {
    Rectangle<CoordType> mbr;
    ObjIDType objID;

    tagRTreeEntry() : objID(-1) {}
    tagRTreeEntry(const Rectangle<CoordType>& rect, ObjIDType id)
        : mbr(rect), objID(id) {}
};

template <typename Trait>
class CRTreeNode {
    friend class CRTree<Trait>;
    typedef typename Trait::CoordType CoordType;
    typedef typename Trait::ObjIDType ObjIDType;
    typedef CRTreeNode<Trait> RTNode;
    typedef tagRTreeEntry<CoordType, ObjIDType> Entry;

public:
    CRTreeNode(size_t maxEntries, bool isLeaf = true);
    ~CRTreeNode();

    rt_ErrorCode Insert(const Rectangle<CoordType>& rect, const ObjIDType objID);

    Rectangle<CoordType> GetMBR() const;
    void UpdateMBR();

protected:
    size_t m_MaxEntries;
    size_t m_MinEntries;
    size_t m_Count;
    bool m_IsLeaf;

    Rectangle<CoordType> m_MBR;
    std::vector<Entry> m_Entries;
    std::vector<RTNode*> m_Children;

    bool IsLeaf() const { return m_IsLeaf; }
    bool IsOverflow() const { return m_Count > m_MaxEntries; }
    bool IsUnderflow() const { return m_Count < m_MinEntries; }

    size_t ChooseSubtree(const Rectangle<CoordType>& rect);
    void SplitNode(RTNode*& newNode);
    void PickSeeds(size_t& seed1, size_t& seed2);
};

template <typename Trait>
CRTreeNode<Trait>::CRTreeNode(size_t maxEntries, bool isLeaf)
    : m_MaxEntries(maxEntries), m_Count(0), m_IsLeaf(isLeaf) {
    m_MinEntries = maxEntries / 2;
    m_Entries.resize(maxEntries + 1);
    m_Children.resize(maxEntries + 2, nullptr);
}

template <typename Trait>
CRTreeNode<Trait>::~CRTreeNode() {
    if (!m_IsLeaf) {
        for (size_t i = 0; i <= m_Count; i++) {
            delete m_Children[i];
        }
    }
}

template <typename Trait>
Rectangle<typename Trait::CoordType> CRTreeNode<Trait>::GetMBR() const {
    return m_MBR;
}

template <typename Trait>
void CRTreeNode<Trait>::UpdateMBR() {
    if (m_Count == 0) {
        m_MBR = Rectangle<CoordType>();
        return;
    }

    m_MBR = m_Entries[0].mbr;
    for (size_t i = 1; i < m_Count; i++) {
        m_MBR.expand(m_Entries[i].mbr);
    }

    if (!m_IsLeaf) {
        for (size_t i = 0; i <= m_Count; i++) {
            if (m_Children[i]) {
                m_MBR.expand(m_Children[i]->GetMBR());
            }
        }
    }
}

template <typename Trait>
size_t CRTreeNode<Trait>::ChooseSubtree(const Rectangle<CoordType>& rect) {
    if (m_IsLeaf) {
        return m_Count;
    }

    size_t best = 0;
    CoordType minEnlargement = m_Children[0]->GetMBR().enlargement(rect);

    for (size_t i = 1; i <= m_Count; i++) {
        CoordType enlargement = m_Children[i]->GetMBR().enlargement(rect);
        if (enlargement < minEnlargement) {
            minEnlargement = enlargement;
            best = i;
        }
    }

    return best;
}

template <typename Trait>
rt_ErrorCode CRTreeNode<Trait>::Insert(const Rectangle<CoordType>& rect, const ObjIDType objID) {
    if (m_IsLeaf) {
        m_Entries[m_Count] = Entry(rect, objID);
        m_Count++;
        UpdateMBR();

        if (IsOverflow()) {
            return rt_overflow;
        }
        return rt_ok;
    }

    size_t index = ChooseSubtree(rect);
    rt_ErrorCode error = m_Children[index]->Insert(rect, objID);

    if (error == rt_overflow) {
        RTNode* newNode = nullptr;
        m_Children[index]->SplitNode(newNode);

        if (newNode) {
            for (size_t i = m_Count; i > index; i--) {
                m_Entries[i] = m_Entries[i - 1];
                m_Children[i + 1] = m_Children[i];
            }
            m_Entries[index] = Entry(newNode->GetMBR(), -1);
            m_Children[index + 1] = newNode;
            m_Count++;
        }

        UpdateMBR();

        if (IsOverflow()) {
            return rt_overflow;
        }
    }

    UpdateMBR();
    return rt_ok;
}

template <typename Trait>
void CRTreeNode<Trait>::PickSeeds(size_t& seed1, size_t& seed2) {
    CoordType maxWaste = -1;
    seed1 = 0;
    seed2 = 1;

    for (size_t i = 0; i < m_Count; i++) {
        for (size_t j = i + 1; j <= m_Count; j++) {
            Rectangle<CoordType> combined = m_Entries[i].mbr;
            combined.expand(m_Entries[j].mbr);
            CoordType waste = combined.area() - m_Entries[i].mbr.area() - m_Entries[j].mbr.area();

            if (waste > maxWaste) {
                maxWaste = waste;
                seed1 = i;
                seed2 = j;
            }
        }
    }
}

template <typename Trait>
void CRTreeNode<Trait>::SplitNode(RTNode*& newNode) {
    newNode = new RTNode(m_MaxEntries, m_IsLeaf);

    size_t seed1, seed2;
    PickSeeds(seed1, seed2);

    std::vector<Entry> tempEntries;
    std::vector<RTNode*> tempChildren;

    for (size_t i = 0; i <= m_Count; i++) {
        tempEntries.push_back(m_Entries[i]);
        if (!m_IsLeaf) {
            tempChildren.push_back(m_Children[i]);
        }
    }

    m_Count = 0;
    m_Entries[m_Count++] = tempEntries[seed1];
    if (!m_IsLeaf) {
        m_Children[0] = tempChildren[seed1];
    }

    newNode->m_Entries[newNode->m_Count++] = tempEntries[seed2];
    if (!m_IsLeaf) {
        newNode->m_Children[0] = tempChildren[seed2];
    }

    for (size_t i = 0; i < tempEntries.size(); i++) {
        if (i == seed1 || i == seed2) continue;

        Rectangle<CoordType> mbr1 = GetMBR();
        Rectangle<CoordType> mbr2 = newNode->GetMBR();

        CoordType enl1 = mbr1.enlargement(tempEntries[i].mbr);
        CoordType enl2 = mbr2.enlargement(tempEntries[i].mbr);

        if (enl1 < enl2) {
            m_Entries[m_Count] = tempEntries[i];
            if (!m_IsLeaf) {
                m_Children[m_Count] = tempChildren[i];
            }
            m_Count++;
        } else {
            newNode->m_Entries[newNode->m_Count] = tempEntries[i];
            if (!m_IsLeaf) {
                newNode->m_Children[newNode->m_Count] = tempChildren[i];
            }
            newNode->m_Count++;
        }

        UpdateMBR();
        newNode->UpdateMBR();
    }
}

#endif
