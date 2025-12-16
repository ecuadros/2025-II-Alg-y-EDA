#ifndef __RTREEPAGE_H__
#define __RTREEPAGE_H__

#include <vector>
#include <limits>
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;

/// @brief Error codes for R-tree operations
enum rt_ErrorCode {rt_ok, rt_overflow, rt_underflow, rt_duplicate, rt_nofound, rt_rootmerged};

/// @brief Represents a 2D rectangle (bounding box) for spatial indexing
struct Rectangle {
    double x_min, y_min, x_max, y_max;
    
    /// @brief Default constructor - creates an empty rectangle
    Rectangle() : x_min(0), y_min(0), x_max(0), y_max(0) {}
    
    /// @brief Constructor with coordinates
    Rectangle(double xmin, double ymin, double xmax, double ymax) 
        : x_min(xmin), y_min(ymin), x_max(xmax), y_max(ymax) {}
    
    /// @brief Calculates the area of the rectangle
    /// @return Area of the rectangle
    double Area() const {
        if (x_max < x_min || y_max < y_min) return 0.0;
        return (x_max - x_min) * (y_max - y_min);
    }
    
    /// @brief Calculates the overlap area with another rectangle
    /// @param other The other rectangle
    /// @return Overlap area
    double Overlap(const Rectangle& other) const {
        double x_overlap = std::max(0.0, std::min(x_max, other.x_max) - std::max(x_min, other.x_min));
        double y_overlap = std::max(0.0, std::min(y_max, other.y_max) - std::max(y_min, other.y_min));
        return x_overlap * y_overlap;
    }
    
    /// @brief Calculates the area enlargement needed to include another rectangle
    /// @param other The rectangle to include
    /// @return Area enlargement
    double Enlargement(const Rectangle& other) const {
        Rectangle combined = CombineWith(other);
        return combined.Area() - Area();
    }
    
    /// @brief Checks if this rectangle completely contains another
    /// @param other The other rectangle
    /// @return true if this contains other
    bool Contains(const Rectangle& other) const {
        return x_min <= other.x_min && x_max >= other.x_max &&
               y_min <= other.y_min && y_max >= other.y_max;
    }
    
    /// @brief Checks if this rectangle intersects with another
    /// @param other The other rectangle
    /// @return true if rectangles intersect
    bool Intersects(const Rectangle& other) const {
        return !(x_max < other.x_min || x_min > other.x_max ||
                 y_max < other.y_min || y_min > other.y_max);
    }
    
    /// @brief Combines this rectangle with another to create a bounding box
    /// @param other The other rectangle
    /// @return Combined bounding box
    Rectangle CombineWith(const Rectangle& other) const {
        return Rectangle(
            std::min(x_min, other.x_min),
            std::min(y_min, other.y_min),
            std::max(x_max, other.x_max),
            std::max(y_max, other.y_max)
        );
    }
    
    /// @brief Expands this rectangle to include another
    /// @param other The rectangle to include
    void Expand(const Rectangle& other) {
        x_min = std::min(x_min, other.x_min);
        y_min = std::min(y_min, other.y_min);
        x_max = std::max(x_max, other.x_max);
        y_max = std::max(y_max, other.y_max);
    }
    
    /// @brief Checks if two rectangles are equal
    bool operator==(const Rectangle& other) const {
        return x_min == other.x_min && y_min == other.y_min &&
               x_max == other.x_max && y_max == other.y_max;
    }
};

template <typename Trait>
class RTree;

template <typename Trait>
class RTreePage;

/// @brief Stores information about an object in the R-tree
template <typename ObjIDType>
struct RTreeObjectInfo {
    Rectangle rect;
    ObjIDType ObjID;
    
    RTreeObjectInfo() : rect(), ObjID(-1) {}
    RTreeObjectInfo(const Rectangle& r, ObjIDType id) : rect(r), ObjID(id) {}
};

/// @brief Represents an entry in an R-tree node
template <typename Trait>
struct RTreeEntry {
    typedef typename Trait::ObjIDType ObjIDType;
    
    Rectangle mbr;  // Minimum Bounding Rectangle
    RTreePage<Trait>* child;  // Child pointer (for internal nodes)
    ObjIDType objID;  // Object ID (for leaf nodes)
    
    RTreeEntry() : mbr(), child(nullptr), objID(-1) {}
    RTreeEntry(const Rectangle& r, RTreePage<Trait>* c) : mbr(r), child(c), objID(-1) {}
    RTreeEntry(const Rectangle& r, ObjIDType id) : mbr(r), child(nullptr), objID(id) {}
};

/// @brief R-tree page/node implementation
template <typename Trait>
class RTreePage {
    friend class RTree<Trait>;
    
    typedef typename Trait::ObjIDType ObjIDType;
    typedef RTreeEntry<Trait> Entry;
    typedef RTreeObjectInfo<ObjIDType> ObjectInfo;
    
protected:
    vector<Entry> m_Entries;
    size_t m_MaxEntries;
    size_t m_MinEntries;
    bool m_IsLeaf;
    Rectangle m_MBR;  // Minimum Bounding Rectangle for this page
    
public:
    /// @brief Constructor
    RTreePage(size_t maxEntries, bool isLeaf = true)
        : m_MaxEntries(maxEntries), 
          m_MinEntries(maxEntries / 2),
          m_IsLeaf(isLeaf),
          m_MBR() {
        m_Entries.reserve(maxEntries + 1);  // +1 for temporary overflow
    }
    
    /// @brief Destructor
    ~RTreePage() {
        if (!m_IsLeaf) {
            for (auto& entry : m_Entries) {
                if (entry.child) {
                    delete entry.child;
                }
            }
        }
    }
    
    /// @brief Inserts a rectangle into the tree
    /// @param rect The rectangle to insert
    /// @param objID The object ID
    /// @return Error code
    rt_ErrorCode Insert(const Rectangle& rect, ObjIDType objID) {
        if (m_IsLeaf) {
            // Check for duplicates
            for (const auto& entry : m_Entries) {
                if (entry.mbr == rect && entry.objID == objID) {
                    return rt_duplicate;
                }
            }
            
            // Insert into leaf
            m_Entries.push_back(Entry(rect, objID));
            UpdateMBR();
            
            if (m_Entries.size() > m_MaxEntries) {
                return rt_overflow;
            }
            return rt_ok;
        } else {
            // Choose subtree with minimum enlargement
            size_t bestIdx = ChooseSubtree(rect);
            rt_ErrorCode result = m_Entries[bestIdx].child->Insert(rect, objID);
            
            if (result == rt_overflow) {
                // Split the child
                RTreePage* newPage = new RTreePage(m_MaxEntries, m_Entries[bestIdx].child->m_IsLeaf);
                QuadraticSplit(m_Entries[bestIdx].child, newPage);
                
                // Update the MBR of the split child
                m_Entries[bestIdx].mbr = m_Entries[bestIdx].child->m_MBR;
                
                // Add the new child
                m_Entries.push_back(Entry(newPage->m_MBR, newPage));
                UpdateMBR();
                
                if (m_Entries.size() > m_MaxEntries) {
                    return rt_overflow;
                }
            } else {
                // Update MBR after insertion
                m_Entries[bestIdx].mbr = m_Entries[bestIdx].child->m_MBR;
                UpdateMBR();
            }
            
            return rt_ok;
        }
    }
    
    /// @brief Removes a rectangle from the tree
    /// @param rect The rectangle to remove
    /// @param objID The object ID
    /// @return Error code
    rt_ErrorCode Remove(const Rectangle& rect, ObjIDType objID) {
        if (m_IsLeaf) {
            // Find and remove the entry
            for (size_t i = 0; i < m_Entries.size(); ++i) {
                if (m_Entries[i].mbr == rect && m_Entries[i].objID == objID) {
                    m_Entries.erase(m_Entries.begin() + i);
                    UpdateMBR();
                    
                    if (m_Entries.size() < m_MinEntries && m_Entries.size() > 0) {
                        return rt_underflow;
                    }
                    return rt_ok;
                }
            }
            return rt_nofound;
        } else {
            // Find subtrees that might contain the rectangle
            for (size_t i = 0; i < m_Entries.size(); ++i) {
                if (m_Entries[i].mbr.Intersects(rect)) {
                    rt_ErrorCode result = m_Entries[i].child->Remove(rect, objID);
                    
                    if (result == rt_ok || result == rt_underflow) {
                        if (m_Entries[i].child->m_Entries.empty()) {
                            // Remove empty child
                            delete m_Entries[i].child;
                            m_Entries.erase(m_Entries.begin() + i);
                        } else {
                            // Update MBR
                            m_Entries[i].mbr = m_Entries[i].child->m_MBR;
                        }
                        
                        UpdateMBR();
                        
                        if (m_Entries.size() < m_MinEntries && m_Entries.size() > 0) {
                            return rt_underflow;
                        }
                        return rt_ok;
                    }
                }
            }
            return rt_nofound;
        }
    }
    
    /// @brief Searches for a rectangle in the tree
    /// @param rect The rectangle to search for
    /// @param objID Output parameter for the object ID
    /// @return true if found
    bool Search(const Rectangle& rect, ObjIDType& objID) {
        if (m_IsLeaf) {
            for (const auto& entry : m_Entries) {
                if (entry.mbr == rect) {
                    objID = entry.objID;
                    return true;
                }
            }
            return false;
        } else {
            for (const auto& entry : m_Entries) {
                if (entry.mbr.Intersects(rect)) {
                    if (entry.child->Search(rect, objID)) {
                        return true;
                    }
                }
            }
            return false;
        }
    }
    
    /// @brief Performs a range query to find all rectangles intersecting the query rectangle
    /// @param queryRect The query rectangle
    /// @param results Output vector of results
    void RangeQuery(const Rectangle& queryRect, vector<ObjectInfo>& results) {
        if (m_IsLeaf) {
            for (const auto& entry : m_Entries) {
                if (entry.mbr.Intersects(queryRect)) {
                    results.push_back(ObjectInfo(entry.mbr, entry.objID));
                }
            }
        } else {
            for (const auto& entry : m_Entries) {
                if (entry.mbr.Intersects(queryRect)) {
                    entry.child->RangeQuery(queryRect, results);
                }
            }
        }
    }
    
    /// @brief Splits the root node
    void SplitRoot() {
        RTreePage* newPage = new RTreePage(m_MaxEntries, m_IsLeaf);
        QuadraticSplit(this, newPage);
        
        // Create new root
        vector<Entry> oldEntries = m_Entries;
        m_Entries.clear();
        m_IsLeaf = false;
        
        // Create two children from the split
        RTreePage* child1 = new RTreePage(m_MaxEntries, oldEntries[0].child ? false : true);
        RTreePage* child2 = newPage;
        
        child1->m_Entries = oldEntries;
        child1->UpdateMBR();
        
        m_Entries.push_back(Entry(child1->m_MBR, child1));
        m_Entries.push_back(Entry(child2->m_MBR, child2));
        UpdateMBR();
    }
    
    /// @brief Prints the tree structure
    /// @param os Output stream
    /// @param level Current level in the tree
    void Print(ostream& os, size_t level = 0) {
        for (size_t i = 0; i < level; ++i) os << "  ";
        os << (m_IsLeaf ? "Leaf" : "Internal") << " [" << m_Entries.size() << " entries]";
        os << " MBR: (" << m_MBR.x_min << "," << m_MBR.y_min << ")-(" 
           << m_MBR.x_max << "," << m_MBR.y_max << ")\n";
        
        for (const auto& entry : m_Entries) {
            for (size_t i = 0; i < level + 1; ++i) os << "  ";
            if (m_IsLeaf) {
                os << "Object " << entry.objID << ": (" 
                   << entry.mbr.x_min << "," << entry.mbr.y_min << ")-(" 
                   << entry.mbr.x_max << "," << entry.mbr.y_max << ")\n";
            } else {
                os << "Child MBR: (" << entry.mbr.x_min << "," << entry.mbr.y_min << ")-(" 
                   << entry.mbr.x_max << "," << entry.mbr.y_max << ")\n";
                entry.child->Print(os, level + 2);
            }
        }
    }
    
protected:
    /// @brief Updates the MBR of this page based on its entries
    void UpdateMBR() {
        if (m_Entries.empty()) {
            m_MBR = Rectangle();
            return;
        }
        
        m_MBR = m_Entries[0].mbr;
        for (size_t i = 1; i < m_Entries.size(); ++i) {
            m_MBR.Expand(m_Entries[i].mbr);
        }
    }
    
    /// @brief Chooses the best subtree for insertion
    /// @param rect The rectangle to insert
    /// @return Index of the best subtree
    size_t ChooseSubtree(const Rectangle& rect) {
        size_t bestIdx = 0;
        double minEnlargement = m_Entries[0].mbr.Enlargement(rect);
        double minArea = m_Entries[0].mbr.Area();
        
        for (size_t i = 1; i < m_Entries.size(); ++i) {
            double enlargement = m_Entries[i].mbr.Enlargement(rect);
            double area = m_Entries[i].mbr.Area();
            
            if (enlargement < minEnlargement || 
                (enlargement == minEnlargement && area < minArea)) {
                minEnlargement = enlargement;
                minArea = area;
                bestIdx = i;
            }
        }
        
        return bestIdx;
    }
    
    /// @brief Quadratic split algorithm for R-tree
    /// @param page1 First page (will be modified)
    /// @param page2 Second page (will receive split entries)
    void QuadraticSplit(RTreePage* page1, RTreePage* page2) {
        vector<Entry> allEntries = page1->m_Entries;
        page1->m_Entries.clear();
        
        // Pick seeds - find the pair with maximum waste
        size_t seed1 = 0, seed2 = 1;
        double maxWaste = -1;
        
        for (size_t i = 0; i < allEntries.size(); ++i) {
            for (size_t j = i + 1; j < allEntries.size(); ++j) {
                Rectangle combined = allEntries[i].mbr.CombineWith(allEntries[j].mbr);
                double waste = combined.Area() - allEntries[i].mbr.Area() - allEntries[j].mbr.Area();
                if (waste > maxWaste) {
                    maxWaste = waste;
                    seed1 = i;
                    seed2 = j;
                }
            }
        }
        
        page1->m_Entries.push_back(allEntries[seed1]);
        page2->m_Entries.push_back(allEntries[seed2]);
        
        // Mark seeds as used
        vector<bool> used(allEntries.size(), false);
        used[seed1] = true;
        used[seed2] = true;
        
        // Distribute remaining entries
        while (page1->m_Entries.size() + page2->m_Entries.size() < allEntries.size()) {
            // Check if one group needs all remaining entries
            size_t remaining = allEntries.size() - page1->m_Entries.size() - page2->m_Entries.size();
            if (page1->m_Entries.size() + remaining == page1->m_MinEntries) {
                for (size_t i = 0; i < allEntries.size(); ++i) {
                    if (!used[i]) {
                        page1->m_Entries.push_back(allEntries[i]);
                        used[i] = true;
                    }
                }
                break;
            }
            if (page2->m_Entries.size() + remaining == page2->m_MinEntries) {
                for (size_t i = 0; i < allEntries.size(); ++i) {
                    if (!used[i]) {
                        page2->m_Entries.push_back(allEntries[i]);
                        used[i] = true;
                    }
                }
                break;
            }
            
            // Pick next entry with maximum preference
            size_t nextEntry = 0;
            double maxDiff = -std::numeric_limits<double>::max();
            bool preferPage1 = true;
            
            page1->UpdateMBR();
            page2->UpdateMBR();
            
            for (size_t i = 0; i < allEntries.size(); ++i) {
                if (used[i]) continue;
                
                double enl1 = page1->m_MBR.Enlargement(allEntries[i].mbr);
                double enl2 = page2->m_MBR.Enlargement(allEntries[i].mbr);
                double diff = std::abs(enl1 - enl2);
                
                if (diff > maxDiff) {
                    maxDiff = diff;
                    nextEntry = i;
                    preferPage1 = (enl1 < enl2);
                }
            }
            
            if (preferPage1) {
                page1->m_Entries.push_back(allEntries[nextEntry]);
            } else {
                page2->m_Entries.push_back(allEntries[nextEntry]);
            }
            used[nextEntry] = true;
        }
        
        page1->UpdateMBR();
        page2->UpdateMBR();
    }
};

#endif
