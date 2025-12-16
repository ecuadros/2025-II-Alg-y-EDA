#ifndef _NODE_H_
#define _NODE_H_

#include <array>
#include <iostream>
#include <mutex>
using namespace std;

template <typename Traits>
struct Point{
    using value_type = Traits::T;
    using DIM        = Traits::DIM;

    std::array<value_type,DIM> coordinates;
    Point() { coordinates.fill(value_type()); }

    value_type& operator[](size_t index) { return coordinates[index]; }
    const value_type& operator[](size_t index) const { return coordinates[index]; }

    void Print(ostream &os) const {
        os << "(";
        for(size_t i = 0; i < DIM; ++i) {
            os << coordinates[i];
            if(i < DIM - 1) os << ", ";
        }
        os << ")";
    }

    void Read(istream &is) {
        for(size_t i = 0; i < DIM; ++i) {
            is >> coordinates[i];
        }
    }
};

template <typename Traits>
std::istream& operator>>(std::istream &is, Point<Traits> &point) {
    point.Read(is);
    return is;
}

template <typename Traits>
std::ostream& operator<<(std::ostream &os, const Point<Traits> &point) {
    point.Print(os);
    return os;
}

template <typename Traits>
struct MBR{
    using value_type = Traits::T;
    using DIM        = Traits::DIM;
    using PointType  = Point<Traits>;
    using MBRType    = MBR<Traits>;

    PointType min;
    PointType max;

    MBR() {}

    MBR(const PointType& minPoint, const PointType& maxPoint)
        : min(minPoint), max(maxPoint) {}

    bool contains(const PointType& point) const {
        for(size_t i = 0; i < DIM; ++i) {
            if(point[i] < min[i] || point[i] > max[i]) {
                return false;
            }
        }
    }

    void expand(const MBRType& other) {
        for(size_t i = 0; i < DIM; ++i) {
            if(other.min[i] < min[i]) {
                min[i] = other.min[i];
            }
            if(other.max[i] > max[i]) {
                max[i] = other.max[i];
            }
        }
    }

    value_type area() const {
        value_type a = (value_type)1;
        for(size_t i = 0; i < DIM; ++i) {
            value_type l = (value_type)(max[i] - min[i]);
            if(l < 0) return (value_type)0;
            a *= l;
        }
        return a;
    }

    value_type increase(const MBRType& other) const {
        MBRType expanded = *this;
        expanded.expand(other);
        return expanded.area() - this->area();
    }

    bool overloaps(const MBRType& other) const {
        for(size_t i = 0; i < DIM; ++i) {
            if(max[i] < other.min[i] || min[i] > other.max[i]) {
                return false;
            }
        }
        return true;
    }

    void Print(ostream &os) const {
        os << "MBR[";
        min.Print(os);
        os << " - ";
        max.Print(os);
        os << "]";
    }

    void Read(istream &is) {
        min.Read(is);
        max.Read(is);
    }

};

template <typename Traits>
std::istream& operator>>(std::istream &is, MBR<Traits> &mbr) {
    mbr.Read(is);
    return is;
}

template <typename Traits>
std::ostream& operator<<(std::ostream &os, const MBR<Traits> &mbr) {
    mbr.Print(os);
    return os;
}

template <typename Traits>
struct Entry{
    using value_type = Traits::T;
    using Ref        = Traits::Ref;
    using DIM        = Traits::DIM;
    using Node       = RNode<Traits>;
    using MBRType    = MBR<Traits>;


    MBRType        mbr;
    Node*          childNode = nullptr; //  internal nodes
    Ref            ref; //leaf nodes

    Entry() {}

    Entry(const MBRType& mbrBox, Ref reference)
        : mbr(mbrBox), ref(reference) {}
    Entry(const MBRType& mbrBox, Node* child)
        : mbr(mbrBox), childNode(child) {}

    bool isLeafEntry() const {
        return childNode == nullptr;
    }

    void Print(ostream &os) const {
        os << "Entry: ";
        mbr.Print(os);
        if(isLeafEntry()) {
            os << ", Ref: " << ref;
        } else {
            os << ", ChildNode: " << childNode;
        }
    }

    void Read(istream &is) {
        mbr.Read(is);
        is >> ref;
    }
};

template <typename Traits>
std::istream& operator>>(std::istream &is, Entry<Traits> &entry) {
    entry.Read(is);
    return is;
}   

template <typename Traits>  
std::ostream& operator<<(std::ostream &os, const Entry<Traits> &entry) {
    entry.Print(os);
    return os;
}

// R-Tree Node
template <typename Traits>
class RNode {
private:
    using m_MaxEntry = typename Traits::size_t M;
    using m_MinEntry = typename Traits::size_t m;
    using EntryType = Entry<Traits>;
    using Ref       = typename Traits::Ref;
    using MBRType   = MBR<Traits>;

    size_t m_Level; 

    bool m_isRoot;
    bool m_isLeaf;

    std::vector<EntryType> m_Entries;
    MBRType m_MBR;

    void UpdateMBR() {
        if(m_Entries.empty()) {
            return;
        }
        m_MBR = m_Entries[0].mbr;
        for(size_t i = 1; i < m_Entries.size(); ++i) {
            m_MBR.expand(m_Entries[i].mbr);
        }
    }

public:
    RNode(size_t level, bool isRoot = false, bool isLeaf = true)
        : m_Level(level), m_isRoot(isRoot), m_isLeaf(isLeaf) {}

    ~RNode() {}

    size_t GetOrder() const { return m_Order; }
    size_t GetHeight() const { return m_Level; }

    bool IsRoot() const { return m_isRoot; }
    bool IsLeaf() const { return m_isLeaf; }

    void SetRoot(bool isRoot) { m_isRoot = isRoot; }
    void SetLeaf(bool isLeaf) { m_isLeaf = isLeaf; }

    MBRType GetMBR() const { return m_MBR; }
    size_t GetNumberOfEntries() const { return m_Entries.size(); }

    bool IsFull() const {
        return m_Entries.size() >= m_MaxEntry;
    }
    bool IsUnderflow() const {
        return m_Entries.size() < m_MinEntry;
    }
    bool IsOverflow() const {
        return m_Entries.size() > m_MaxEntry;
    }
};

#endif // _NODE_H_ //