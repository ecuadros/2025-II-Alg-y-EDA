#ifndef __RTREE_H__
#define __RTREE_H__

#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <fstream>
#include <string>
#include <vector>
#include "rtreepage.h"

#define DEFAULT_RTREE_MAX_ENTRIES 4

/// @brief Trait structure for R-tree template parameters
template <typename _ObjIDType>
struct RTreeTrait {
    using ObjIDType = _ObjIDType;
};

/// @brief R-tree spatial index implementation
template <typename Trait>
class RTree {
    typedef typename Trait::ObjIDType ObjIDType;
    typedef RTreePage<Trait> RTNode;
    typedef RTreeObjectInfo<ObjIDType> ObjectInfo;
    
protected:
    RTNode* m_Root;
    size_t m_Height;
    size_t m_MaxEntries;
    size_t m_NumObjects;
    
private:
    std::shared_mutex m_Mutex;
    
public:
    /// @brief Constructs a new R-tree with a given maximum entries per node
    /// @param maxEntries Maximum number of entries per node
    RTree(size_t maxEntries = DEFAULT_RTREE_MAX_ENTRIES)
        : m_MaxEntries(maxEntries),
          m_NumObjects(0),
          m_Height(1) {
        m_Root = new RTNode(maxEntries, true);
    }
    
    /// @brief Move constructor
    RTree(RTree&& other) {
        std::lock_guard<std::shared_mutex> lock(other.m_Mutex);
        m_Root = other.m_Root;
        m_Height = other.m_Height;
        m_MaxEntries = other.m_MaxEntries;
        m_NumObjects = other.m_NumObjects;
        other.m_Root = nullptr;
    }
    
    /// @brief Destructor
    ~RTree() {
        if (m_Root) {
            delete m_Root;
        }
    }
    
    /// @brief Inserts a rectangle and associated object ID into the tree
    /// @param rect The rectangle to insert
    /// @param objID The identifier associated with the rectangle
    /// @return true if insertion was successful
    bool Insert(const Rectangle& rect, ObjIDType objID) {
        std::unique_lock<std::shared_mutex> lock(m_Mutex);
        
        rt_ErrorCode error = m_Root->Insert(rect, objID);
        
        if (error == rt_duplicate) {
            return false;
        }
        
        m_NumObjects++;
        
        if (error == rt_overflow) {
            m_Root->SplitRoot();
            m_Height++;
        }
        
        return true;
    }
    
    /// @brief Removes a rectangle and its associated object ID from the tree
    /// @param rect The rectangle to remove
    /// @param objID The identifier associated with the rectangle
    /// @return true if removal was successful
    bool Remove(const Rectangle& rect, ObjIDType objID) {
        std::unique_lock<std::shared_mutex> lock(m_Mutex);
        
        rt_ErrorCode error = m_Root->Remove(rect, objID);
        
        if (error == rt_nofound) {
            return false;
        }
        
        m_NumObjects--;
        
        // If root has only one child after removal, make that child the new root
        if (!m_Root->m_IsLeaf && m_Root->m_Entries.size() == 1) {
            RTNode* oldRoot = m_Root;
            m_Root = m_Root->m_Entries[0].child;
            oldRoot->m_Entries.clear();  // Prevent deletion of child
            delete oldRoot;
            m_Height--;
        }
        
        return true;
    }
    
    /// @brief Searches for a rectangle in the tree
    /// @param rect The rectangle to search for
    /// @return The object ID associated with the rectangle, or -1 if not found
    ObjIDType Search(const Rectangle& rect) {
        std::shared_lock<std::shared_mutex> lock(m_Mutex);
        ObjIDType objID = -1;
        m_Root->Search(rect, objID);
        return objID;
    }
    
    /// @brief Performs a range query to find all objects intersecting the query rectangle
    /// @param queryRect The query rectangle
    /// @param results Output vector to store results
    void RangeQuery(const Rectangle& queryRect, vector<ObjectInfo>& results) {
        std::shared_lock<std::shared_mutex> lock(m_Mutex);
        results.clear();
        m_Root->RangeQuery(queryRect, results);
    }
    
    /// @brief Writes the R-tree to disk
    /// @param filename The file to write to
    /// @return true if successful
    bool WriteToDisk(const std::string& filename) {
        std::shared_lock<std::shared_mutex> lock(m_Mutex);
        
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // Write metadata
        file.write(reinterpret_cast<const char*>(&m_Height), sizeof(m_Height));
        file.write(reinterpret_cast<const char*>(&m_MaxEntries), sizeof(m_MaxEntries));
        file.write(reinterpret_cast<const char*>(&m_NumObjects), sizeof(m_NumObjects));
        
        // Write tree structure
        WriteNodeToDisk(file, m_Root);
        
        file.close();
        return true;
    }
    
    /// @brief Reads the R-tree from disk
    /// @param filename The file to read from
    /// @return true if successful
    bool ReadFromDisk(const std::string& filename) {
        std::unique_lock<std::shared_mutex> lock(m_Mutex);
        
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // Clear existing tree
        if (m_Root) {
            delete m_Root;
        }
        
        // Read metadata
        file.read(reinterpret_cast<char*>(&m_Height), sizeof(m_Height));
        file.read(reinterpret_cast<char*>(&m_MaxEntries), sizeof(m_MaxEntries));
        file.read(reinterpret_cast<char*>(&m_NumObjects), sizeof(m_NumObjects));
        
        // Read tree structure
        m_Root = ReadNodeFromDisk(file);
        
        file.close();
        return true;
    }
    
    /// @brief Returns the number of objects in the tree
    size_t size() {
        std::shared_lock<std::shared_mutex> lock(m_Mutex);
        return m_NumObjects;
    }
    
    /// @brief Returns the height of the tree
    size_t height() {
        std::shared_lock<std::shared_mutex> lock(m_Mutex);
        return m_Height;
    }
    
    /// @brief Prints the tree structure
    /// @param os Output stream
    void Print(ostream& os) {
        std::shared_lock<std::shared_mutex> lock(m_Mutex);
        os << "R-Tree (Height: " << m_Height << ", Objects: " << m_NumObjects << ")\n";
        m_Root->Print(os, 0);
    }
    
    /// @brief Overloaded stream operator
    template <typename T>
    friend std::ostream& operator<<(std::ostream& os, RTree<T>& tree);
    
protected:
    /// @brief Writes a node to disk recursively
    /// @param file Output file stream
    /// @param node Node to write
    void WriteNodeToDisk(std::ofstream& file, RTNode* node) {
        if (!node) return;
        
        // Write node metadata
        bool isLeaf = node->m_IsLeaf;
        size_t numEntries = node->m_Entries.size();
        
        file.write(reinterpret_cast<const char*>(&isLeaf), sizeof(isLeaf));
        file.write(reinterpret_cast<const char*>(&numEntries), sizeof(numEntries));
        
        // Write MBR
        file.write(reinterpret_cast<const char*>(&node->m_MBR), sizeof(Rectangle));
        
        // Write entries
        for (const auto& entry : node->m_Entries) {
            file.write(reinterpret_cast<const char*>(&entry.mbr), sizeof(Rectangle));
            
            if (isLeaf) {
                file.write(reinterpret_cast<const char*>(&entry.objID), sizeof(ObjIDType));
            } else {
                WriteNodeToDisk(file, entry.child);
            }
        }
    }
    
    /// @brief Reads a node from disk recursively
    /// @param file Input file stream
    /// @return Pointer to the read node
    RTNode* ReadNodeFromDisk(std::ifstream& file) {
        // Read node metadata
        bool isLeaf;
        size_t numEntries;
        
        file.read(reinterpret_cast<char*>(&isLeaf), sizeof(isLeaf));
        file.read(reinterpret_cast<char*>(&numEntries), sizeof(numEntries));
        
        RTNode* node = new RTNode(m_MaxEntries, isLeaf);
        
        // Read MBR
        file.read(reinterpret_cast<char*>(&node->m_MBR), sizeof(Rectangle));
        
        // Read entries
        for (size_t i = 0; i < numEntries; ++i) {
            Rectangle mbr;
            file.read(reinterpret_cast<char*>(&mbr), sizeof(Rectangle));
            
            if (isLeaf) {
                ObjIDType objID;
                file.read(reinterpret_cast<char*>(&objID), sizeof(ObjIDType));
                node->m_Entries.push_back(typename RTNode::Entry(mbr, objID));
            } else {
                RTNode* child = ReadNodeFromDisk(file);
                node->m_Entries.push_back(typename RTNode::Entry(mbr, child));
            }
        }
        
        return node;
    }
};

/// @brief Stream operator for R-tree
template <typename Trait>
std::ostream& operator<<(std::ostream& os, RTree<Trait>& tree) {
    std::shared_lock<std::shared_mutex> lock(tree.m_Mutex);
    tree.Print(os);
    return os;
}

#endif
