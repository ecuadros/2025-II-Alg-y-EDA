#ifndef _NODE_H_
#define _NODE_H_

#include <array>
#include <iostream>
#include <mutex>
#include <shared_mutex>
using namespace std;

/*
*
**/
/**
 * @brief Point in an N-dimensional space.
 * @tparam Traits Traits class that must provide `T` (value type) and `DIM` (dimension).
 *
 * The `Point` stores coordinates in a fixed-size std::array and provides
 * element access and simple IO helpers used by the R-Tree implementation.
 */
template <typename Traits>
struct Point{
    using value_type = Traits::T;
    using DIM        = Traits::DIM;

    std::array<value_type,DIM> coordinates;
    Point() { coordinates.fill(value_type()); }

    value_type& operator[](size_t index) { return coordinates[index]; }
    const value_type& operator[](size_t index) const { return coordinates[index]; }

    /**
     * @brief Print point to output stream.
     * @param os Output stream.
     */
    void Print(ostream &os) const {
        os << "(";
        for(size_t i = 0; i < DIM; ++i) {
            os << coordinates[i];
            if(i < DIM - 1) os << ", ";
        }
        os << ")";
    }

    /**
     * @brief Read coordinates from input stream.
     * @param is Input stream.
     */
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

/**
 * @brief Minimum Bounding Rectangle (MBR) for N-dimensional points.
 * @tparam Traits Traits class that must provide `T` (value type) and `DIM` (dimension).
 *
 * The MBR stores `min` and `max` corner points and provides operations
 * such as containment, expansion and area calculation used by the R-Tree.
 */
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

    /**
     * @brief Check whether the MBR contains a point.
     * @param point Point to test.
     * @return true if the point is inside the MBR (inclusive), false otherwise.
     */
    bool contains(const PointType& point) const {
        for(size_t i = 0; i < DIM; ++i) {
            if(point[i] < min[i] || point[i] > max[i]) {
                return false;
            }
        }
    }

    /**
     * @brief Expand this MBR to include `other`.
     * @param other MBR to include.
     */
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

    /**
     * @brief Compute the hyper-rectangular "area" (volume) of the MBR.
     * @return product of side lengths; zero if an invalid side length is found.
     */
    value_type area() const {
        value_type a = (value_type)1;
        for(size_t i = 0; i < DIM; ++i) {
            value_type l = (value_type)(max[i] - min[i]);
            if(l < 0) return (value_type)0;
            a *= l;
        }
        return a;
    }

    /**
     * @brief Compute area increase required to expand this MBR to include `other`.
     * @param other MBR to include.
     * @return area(expanded) - area(this)
     */
    value_type increase(const MBRType& other) const {
        MBRType expanded = *this;
        expanded.expand(other);
        return expanded.area() - this->area();
    }

    /**
     * @brief Test whether two MBRs overlap.
     * @param other Other MBR to test.
     * @return true if MBRs overlap, false otherwise.
     */
    bool overloaps(const MBRType& other) const {
        for(size_t i = 0; i < DIM; ++i) {
            if(max[i] < other.min[i] || min[i] > other.max[i]) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Compute center point of the MBR (coordinate-wise midpoint).
     * @return center Point.
     */
    PointType center() const {
        PointType center;
        for(size_t i = 0; i < DIM; ++i) {
            center[i] = (min[i] + max[i]) / value_type(2);
        }
        return center;
    }

    /**
     * @brief Print MBR to output stream.
     * @param os Output stream.
     */
    void Print(ostream &os) const {
        os << "MBR[";
        min.Print(os);
        os << " - ";
        max.Print(os);
        os << "]";
    }

    /**
     * @brief Read MBR from input stream (reads min then max points).
     * @param is Input stream.
     */
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

/**
 * @brief Entry stored in an R-Tree node.
 * @tparam Traits Traits class that must provide `T` (value type) and `Ref` (leaf reference type).
 *
 * An Entry contains an MBR and either a `ref` (for leaf entries) or a
 * `childNode` pointer (for internal entries).
 */
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

    /**
     * @brief Test whether the entry is a leaf entry.
     * @return true if this entry references data (leaf), false if it references a child node.
     */
    bool isLeafEntry() const {
        return childNode == nullptr;
    }

    /**
     * @brief Print entry information to output stream.
     * @param os Output stream.
     */
    void Print(ostream &os) const {
        os << "Entry: ";
        mbr.Print(os);
        if(isLeafEntry()) {
            os << ", Ref: " << ref;
        } else {
            os << ", ChildNode: " << childNode;
        }
    }

    /**
     * @brief Read entry from input stream.
     * @param is Input stream.
     */
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
/**
 * @brief Node of an R-Tree.
 * @tparam Traits Traits class that must provide sizing parameters and reference types.
 *
 * `RNode` stores a collection of `Entry` objects and maintains an MBR that
 * surrounds all entries.
 */
template <typename Traits>
class RNode {
private:
    using m_MaxEntry = typename Traits::size_t M;
    using m_MinEntry = typename Traits::size_t m;
    using EntryType = Entry<Traits>;
    using Ref       = typename Traits::Ref;
    using MBRType   = MBR<Traits>;
    using value_type = typename Traits::value_type;

    size_t m_Level; 

    bool m_isRoot;
    bool m_isLeaf;

    std::vector<EntryType> m_Entries;
    MBRType m_MBR;
    mutable std::shared_mutex m_mutex;

    /**
     * @brief Recompute node MBR from contained entries.
     *
     * This sets `m_MBR` to the MBR covering all entries (no-op when empty).
     */
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
    /**
     * @brief Construct an RNode.
     * @param level Height level of the node (leaf level = 0 typically).
     * @param isRoot Whether this node is the tree root.
     * @param isLeaf Whether this node is a leaf.
     */
    RNode(size_t level, bool isRoot = false, bool isLeaf = true)
        : m_Level(level), m_isRoot(isRoot), m_isLeaf(isLeaf) {}

    /**
     * @brief Copy constructor for RNode.
     * @param other RNode to copy from.
    **/
    RNode(const RNode& other)
        : m_Level(other.m_Level),
          m_MaxEntry(other.m_MaxEntry),
          m_MinEntry(other.m_MinEntry),
          m_isRoot(other.m_isRoot),
          m_isLeaf(other.m_isLeaf),
          m_Entries(other.m_Entries),
          m_MBR(other.m_MBR) {}


    /**
     * @brief Copy assignment operator for RNode.
     * @param other RNode to copy from.
    **/
    RNode& operator=(const RNode& other) {
        if(this != &other) {
            m_Level = other.m_Level;
            m_MaxEntry = other.m_MaxEntry;
            m_MinEntry = other.m_MinEntry;
            m_isRoot = other.m_isRoot;
            m_isLeaf = other.m_isLeaf;
            m_Entries = other.m_Entries;
            m_MBR = other.m_MBR;
        }
        return *this;
    }
    
    /**
     * @brief Move constructor for RNode.
     * @param other RNode to move from.
    **/
    RNode(RNode&& other) noexcept
        : m_Level(other.m_Level),
          m_MaxEntry(other.m_MaxEntry),
          m_MinEntry(other.m_MinEntry),
          m_isRoot(other.m_isRoot),
          m_isLeaf(other.m_isLeaf),
          m_Entries(std::move(other.m_Entries)),
          m_MBR(std::move(other.m_MBR)) {}
    
    /**
     * @brief Destructor for RNode.
     */
    ~RNode() {}

    /**
     * @brief Get node order (maximum entries allowed).
     * @return maximum number of entries for this node.
     */
    size_t GetOrder() const { std::shared_lock lock(m_mutex); return m_MaxEntry; }

    /**
     * @brief Get node height/level.
     * @return level value (0 for leaves by convention).
     */
    size_t GetHeight() const { std::shared_lock lock(m_mutex); return m_Level; }

    bool IsRoot() const { std::shared_lock lock(m_mutex); return m_isRoot; }
    bool IsLeaf() const { std::shared_lock lock(m_mutex); return m_isLeaf; }

    /**
     * @brief Mark node as root or not.
     * @param isRoot true to mark as root.
     */
    void SetRoot(bool isRoot) { std::unique_lock lock(m_mutex); m_isRoot = isRoot; }

    /**
     * @brief Set whether node is a leaf.
     * @param isLeaf true for leaf nodes.
     */
    void SetLeaf(bool isLeaf) { std::unique_lock lock(m_mutex); m_isLeaf = isLeaf; }

    MBRType GetMBR() const { std::shared_lock lock(m_mutex); return m_MBR; }
    size_t GetNumberOfEntries() const { std::shared_lock lock(m_mutex); return m_Entries.size(); }

    /**
     * @brief Check if node has reached maximum capacity.
     * @return true if full.
     */
    bool IsFull() const {
        std::shared_lock lock(m_mutex);
        return m_Entries.size() >= m_MaxEntry;
    }
    /**
     * @brief Check if node is under minimum occupancy.
     * @return true if underflow.
     */
    bool IsUnderflow() const {
        std::shared_lock lock(m_mutex);
        return m_Entries.size() < m_MinEntry;
    }
    /**
     * @brief Check if node has more entries than allowed (overflow).
     * @return true if overflow.
     */
    bool IsOverflow() const {
        std::shared_lock lock(m_mutex);
        return m_Entries.size() > m_MaxEntry;
    }

    /**
     * @brief Add an entry to the node and expand the node MBR accordingly.
     * @param entry Entry to add.
     */
    void addEntry(const EntryType& entry) {
        std::unique_lock lock(m_mutex);
        m_Entries.push_back(entry);
        m_MBR.expand(entry.mbr);
    }
    
    /**
     * @brief Insert entry and recompute the node MBR.
     * @param entry Entry to insert.
     */
    void insertEntry(const EntryType& entry) {
        std::unique_lock lock(m_mutex);
        m_Entries.push_back(entry);
        UpdateMBR();
    }

    /**
     * @brief Remove entry by index and update MBR.
     * @param index Index of the entry to remove.
     */
    void removeEntry(size_t index) {
        std::unique_lock lock(m_mutex);
        if(index >= m_Entries.size()) return;
        m_Entries.erase(m_Entries.begin() + index);
        UpdateMBR();
    }

    /**
     * @brief Find an entry equal to `target`.
     * @param target Entry to find.
     * @param index Output index where the entry was found (if any).
     * @return true if found, false otherwise.
     */
    bool findEntry(const EntryType& target, size_t& index) const {
        std::shared_lock lock(m_mutex);
        for(size_t i = 0; i < m_Entries.size(); ++i) {
            if(m_Entries[i] == target) {
                index = i;
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Choose the best subtree index to insert `newBox` based on minimal expansion.
     * @param newBox MBR of the new entry to insert.
     * @return index of the chosen child entry.
     */
    size_t chooseSubtree(const MBRType& newBox) const {
        std::shared_lock lock(m_mutex);
        if(m_Entries.empty()) {
            return 0;
        }

        size_t bestIndex = 0;
        value_type minExpand = m_Entries[0].mbr.increase(newBox);
        value_type minArea = m_Entries[0].mbr.area();

        for(size_t i = 1; i < m_Entries.size(); ++i) {
            value_type expand = m_Entries[i].mbr.increase(newBox);
            value_type area = m_Entries[i].mbr.area();

            if(expand < minExpand) {
                bestIndex = i;
                minExpand = expand;
                minArea = area;
            } 
            else if(expand == minExpand && area < minArea) {
                bestIndex = i;
                minArea = area;
            }
        }

        return bestIndex;
    }
};

#endif // _NODE_H_ //