#ifndef __RTREE_H__
#define __RTREE_H__

#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <cmath> 
#include <algorithm>

template <typename Trait> struct Rect;
template <typename Trait> struct RTreeEntry;
template <typename Trait> class RTreeNode;
template <typename Trait> class RTree;

// Traits for RTree with configurable node sizes
template <typename _CoordsType, typename _DataType, size_t _MinNodes = 2, size_t _MaxNodes = 4>
struct RTreeTrait 
{
    using CoordsType = _CoordsType;
    using DataType = _DataType;
    
    // Compile-time validation of template parameters
    static_assert(_MinNodes >= 1, "MinNodes must be at least 1");
    static_assert(_MaxNodes >= _MinNodes, "MaxNodes must be >= MinNodes");
    static_assert(_MaxNodes >= 2, "MaxNodes must be at least 2");
    
    // Configuration constants
    static constexpr size_t MinNodes = _MinNodes;
    static constexpr size_t MaxNodes = _MaxNodes;
    
    using CoordsType_t = _CoordsType;
    using DataType_t = _DataType;
};

// MBR (Minimum Bounding Rectangle)
template <typename Trait>
struct Rect
{
    typedef typename Trait::CoordsType CoordsType;

    CoordsType xMin, xMax, yMin, yMax;

    Rect(CoordsType minX, CoordsType maxX, CoordsType minY, CoordsType maxY)
        : xMin(minX), xMax(maxX), yMin(minY), yMax(maxY) 
    {
        if (xMin > xMax) std::swap(xMin, xMax);
        if (yMin > yMax) std::swap(yMin, yMax);
    }

    Rect(CoordsType x, CoordsType y) : xMin(x), xMax(x), yMin(y), yMax(y) {}

    // Default
    Rect() : xMin(0), xMax(0), yMin(0), yMax(0) {}

    bool IsValid() const {
        return xMin <= xMax && yMin <= yMax;
    }

    bool Overlaps(const Rect& other) const // Intersection between rects
    {
        return xMin <= other.xMax && xMax >= other.xMin && yMin <= other.yMax && yMax >= other.yMin;
    }

    bool Contains(const Rect& other) const  // One includes the other
    {
        return xMin <= other.xMin && xMax >= other.xMax && yMin <= other.yMin && yMax >= other.yMax;
    }    

    bool operator==(const Rect& other) const
    {
        return xMin == other.xMin && yMin == other.yMin && xMax == other.xMax && yMax == other.yMax;
    }

    bool operator!=(const Rect& other) const
    {
        return !(*this == other);
    }

    CoordsType Area() const
    {
        return (xMax - xMin) * (yMax - yMin);
    }

    CoordsType Enlargement(const Rect& other) const // How much would this rect grow to include other
    {
        return Combine(other).Area() - Area();
    }

    Rect Combine(const Rect& other) const // Union
    {
        return Rect{std::min(xMin, other.xMin), std::max(xMax, other.xMax), 
                   std::min(yMin, other.yMin), std::max(yMax, other.yMax)};
    }
};

// Entry for each node
template <typename Trait>
struct RTreeEntry
{
    typedef typename Trait::DataType DataType;
    typedef Rect<Trait> RectType;

    RectType rect;          // MBR
    DataType data;          // For leaf entries
    RTreeNode<Trait>* child; // For internal entries
};

// RTree Node
template <typename Trait>
class RTreeNode
{
    public:
        typedef typename Trait::CoordsType CoordsType;
        typedef typename Trait::DataType DataType;
        typedef Rect<Trait> RectType;
        typedef RTreeEntry<Trait> Entry;

        int level = 0; // Leaf 
        std::vector<Entry> entries; 

        bool IsLeaf() const
        {
            return level == 0; 
        }

        RectType CalculateMBR() const
        {
            if (entries.empty()) {
                // Return default/invalid rect for empty nodes
                return RectType{};
            }
            RectType mbr = entries[0].rect;
            for ( size_t i = 1; i < entries.size(); ++i)
                mbr = mbr.Combine(entries[i].rect);
            return mbr;
        }

        size_t ChooseBestChild(const RectType& rect) const
        {
            size_t best = 0; // Assume first child is the best
            CoordsType minEnlargement = entries[0].rect.Enlargement(rect);
            CoordsType minArea = entries[0].rect.Area(); 

            for ( size_t i = 1; i < entries.size(); ++i ) // Comparing with the other children
            {
                CoordsType enl = entries[i].rect.Enlargement(rect);
                CoordsType area = entries[i].rect.Area();

                if ( enl < minEnlargement || (enl == minEnlargement && area < minArea )) 
                {
                    minEnlargement = enl;
                    minArea = area;
                    best = i;
                }
            }
            return best;
        }
};

// Complete R-Tree
template <typename Trait>
class RTree
{
    public:
        typedef typename Trait::CoordsType CoordsType;
        typedef typename Trait::DataType DataType;
        typedef Rect<Trait> RectType;
        typedef RTreeEntry<Trait> Entry;
        typedef RTreeNode<Trait> Node;
        
        // Results of range query(MBR + Data)
        typedef std::pair<RectType, DataType> QueryResult;

    private:
        Node* m_Root;
        size_t m_Count;
        
        void Destroy(Node* node) {
            if (!node) return;
            for (auto& entry : node->entries) {
                if (entry.child) Destroy(entry.child);
            }
            delete node;
        }

    public:
        RTree() : m_Root(new Node()), m_Count(0) {}
        
        ~RTree() { Destroy(m_Root); }
        
};

#endif