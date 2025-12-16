#ifndef _RTREE_H_
#define _RTREE_H_

#include "node.h"
#include <memory>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <shared_mutex>

/*
 * @brief R-Tree implementation for N-dimensional points.
 * @tparam Traits Traits class that must provide `T` (value type), `DIM` (dimension),
 *                 `M` (maximum number of entries per node), and `m` (minimum number of entries per node).
 */
template<typename Traits>
class RTree {
public:
    using Node = RNode<Traits>;
    using MBRType  = MBR<Traits>;
    using Ref      = typename Traits::Ref;
    using value_type = typename Traits::T;
    using EntryType = Entry<Traits>;
    using PointType  = Point<Traits>;

    static constexpr size_t DIM = Traits::DIM;
    static constexpr size_t M = Traits::M;
    static constexpr size_t m = Traits::m;

private:
    Node* m_Root;
    size_t m_Height;
    size_t m_Count;
    mutable std::shared_mutex m_mutex;

    /*
    * @brief Recursively write a node and its children to a binary stream.
    * @param out Output stream.
    * @param node Node to write.
    */
    static void writeNodeRecursive(std::ostream& out, const Node* node) {
        if(!node) return;
        size_t level = node->GetHeight();
        bool isRoot = node->IsRoot();
        bool isLeaf = node->IsLeaf();
        out.write(reinterpret_cast<const char*>(&level), sizeof(level));
        out.write(reinterpret_cast<const char*>(&isRoot), sizeof(isRoot));
        out.write(reinterpret_cast<const char*>(&isLeaf), sizeof(isLeaf));

        auto entries = node->GetEntries();
        size_t numEntries = entries.size();
        out.write(reinterpret_cast<const char*>(&numEntries), sizeof(numEntries));

        for(const auto &entry : entries) {
            uint8_t hasChild = entry.childNode != nullptr ? 1 : 0;
            out.write(reinterpret_cast<const char*>(&hasChild), sizeof(hasChild));
            entry.WriteToStream(out);
            if(hasChild) {
                writeNodeRecursive(out, entry.childNode);
            }
        }
    }

    /*
    * @brief Recursively read a node and its children from a binary stream.
    * @param in Input stream.
    * @return Pointer to the newly created node.
    */
    static Node* readNodeRecursive(std::istream& in) {
        size_t level;
        bool isRoot, isLeaf;
        in.read(reinterpret_cast<char*>(&level), sizeof(level));
        in.read(reinterpret_cast<char*>(&isRoot), sizeof(isRoot));
        in.read(reinterpret_cast<char*>(&isLeaf), sizeof(isLeaf));

        Node* node = new Node(level, isRoot, isLeaf);

        size_t numEntries = 0;
        in.read(reinterpret_cast<char*>(&numEntries), sizeof(numEntries));
        for(size_t i = 0; i < numEntries; ++i) {
            uint8_t hasChild = 0;
            in.read(reinterpret_cast<char*>(&hasChild), sizeof(hasChild));
            EntryType e;
            e.ReadFromStream(in);
            if(hasChild) {
                Node* child = readNodeRecursive(in);
                e.childNode = child;
            } else {
                e.childNode = nullptr;
            }
            node->addEntry(e);
        }
        return node;
    }

    /*
    * @brief Recursively delete a node and all its children.
    * @param n Node to delete.
    */
    static void deleteNodeRecursive(Node* n) {
        if(!n) return;
        if(!n->IsLeaf()) {
            auto entries = n->GetEntries();
            for(const auto &en : entries) if(en.childNode) deleteNodeRecursive(en.childNode);
        }
        delete n;
    }

public:

    /*
    * @brief Constructs an empty R-Tree.
    */
    RTree() {
        m_Root = new Node(0, true, true);
        m_Height = 1;
        m_Count = 0;
    }

    /*
    * @brief Copy constructor.  
    */
    RTree(const RTree& other) {
        m_Root = new Node(*(other.m_Root));
        m_Height = other.m_Height;
        m_Count = other.m_Count;
    }

    /*
    * @brief Move constructor.
    */
    RTree( RTree&& other ) noexcept {
        m_Root = other.m_Root;
        m_Height = other.m_Height;
        m_Count = other.m_Count;
        other.m_Root = nullptr;
        other.m_Height = 0;
        other.m_Count = 0;
    }

    /*
    * @brief Assignment operator.
    */
    RTree& operator=(const RTree& other) {
        if(this != &other) {
            Clear();
            m_Root = new Node(*(other.m_Root));
            m_Height = other.m_Height;
            m_Count = other.m_Count;
        }
        return *this;
    }

    ~RTree() {
        Clear();
    }

    /**
     * @brief Save entire tree to a binary file (pre-order).
     * @param filename Path to output file.
     */
    void SaveToFile(const char* filename) const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        if(!m_Root) return;
        std::ofstream os(filename, std::ios::binary);
        if(!os.is_open()) return;

        uint32_t magic = 0x52545245; // 'RTRE'
        uint32_t version = 1;
        os.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
        os.write(reinterpret_cast<const char*>(&version), sizeof(version));

        // recursive writer (use static helper)
        writeNodeRecursive(os, m_Root);
        os.close();
    }

    /**
     * @brief Load tree from a binary file previously written with SaveToFile.
     * @param filename Path to input file.
     */
    void LoadFromFile(const char* filename) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        std::ifstream is(filename, std::ios::binary);
        if(!is.is_open()) return;

        uint32_t magic = 0;
        uint32_t version = 0;
        is.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        is.read(reinterpret_cast<char*>(&version), sizeof(version));
        if(magic != 0x52545245) return; 

        if(m_Root) {
            deleteNodeRecursive(m_Root);
            m_Root = nullptr;
            m_Height = 0;
            m_Count = 0;
        }

        m_Root = readNodeRecursive(is);
        if(m_Root) m_Height = m_Root->GetHeight();

        is.close();
    }

    /**
     * @brief Clears the entire tree, deallocating all nodes.
     *
     * This will remove all nodes and reset the tree to an empty state.
     */
    void Clear() {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        if (m_Root) {
            DeleteSubtree(m_Root);
            m_Root = nullptr;
            m_Height = 0;
        }
    }

    /**
     * @brief Recursively deletes a subtree rooted at the given node.
     * @param node Pointer to the root of the subtree to delete. If null, no action is taken.
     */
    void DeleteSubtree(NodeType* node) {
        if (!node) return;
        
        if (!node->IsLeaf()) {
            const auto& entries = node->GetEntries();
            for (const auto& entry : entries) {
                if (entry.childNode) {
                    DeleteSubtree(entry.childNode);
                }
            }
        }
        
        delete node;
    }

    /**
     * @brief Inserts a new point with an associated reference into the R-Tree.
     * @param point The point to insert.
     * @param ref Reference associated with the inserted point.
     */
    void Insert(const PointType& point, RefType ref) {
        MBRType mbr(point);
        Insert(mbr, ref);
    }

    /**
     * @brief Inserts an MBR with an associated reference into the R-Tree.
     * @param mbr The minimum bounding rectangle to insert.
     * @param ref Reference associated with the inserted MBR.
     */
    void Insert(const MBRType& mbr, RefType ref) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        EntryType newEntry(mbr, ref);

        NodeType* leaf = ChooseLeaf(mbr);
        
        //Añadir entry al nodo hoja
        if (!leaf->IsFull()) {
            leaf->AddEntry(newEntry);
            AdjustTree(leaf, nullptr);
        } else {
            //Dividir si está lleno
            HandleOverflow(leaf, newEntry);
        }
    }

    /**
     * @brief Removes an entry that matches the given MBR and reference from the R-Tree.
     * @param mbr The MBR of the entry to remove.
     * @param ref The reference of the entry to remove.
     * @return true if the entry was found and removed; false otherwise.
     */
    bool Delete(const MBRType& mbr, RefType ref) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        NodeType* leaf = FindLeaf(m_Root, mbr, ref);
        if (!leaf) return false;
        
        size_t index;
        EntryType target(mbr, ref);
        if (!leaf->FindEntry(target, index)) {
            return false;
        }
        leaf->RemoveEntry(index);
        
        CondenseTree(leaf);
        
        if (m_root->GetNumberOfEntries() == 1 && !m_root->IsLeaf()) {
            NodeType* newRoot = m_root->GetEntry(0).childNode;
            newRoot->SetRoot(true);
            delete m_root;
            m_root = newRoot;
            m_height--;
        }
        
        return true;
    }

    /**
     * @brief Searches for all entries overlapping the query MBR.
     * @param query Query MBR used for the range search.
     * @return Vector of references corresponding to entries that overlap the query.
     */
    std::vector<RefType> Search(const MBRType& query) const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        std::vector<RefType> results;
        if (m_Root) {
            m_Root->range_query(query, results);
        }
        return results;
    }

    /**
     * @brief Checks whether the tree is empty.
     * @return true if the tree contains no entries or the root is null; false otherwise.
     */
    bool Empty() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return !m_Root || m_Root->GetNumberOfEntries() == 0;
    }

    /**
     * @brief Prints the structure of the tree for debugging purposes.
     * @param os Output stream used to write the tree representation.
     */
    void Print(std::ostream& os) const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        if (m_Root) {
            m_Root->Print(os, 0);
        } else {
            os << "Empty RTree\n";
        }
    }

private:
    /**
     * @brief Guttman's ChooseLeaf algorithm: selects a leaf node suitable for insertion.
     * @param mbr The MBR being inserted.
     * @return Pointer to the chosen leaf node where the entry should be added.
     */
    NodeType* ChooseLeaf(const MBRType& mbr) {
        NodeType* currentNode = m_root;
        while (!currentNode->IsLeaf()) {
            size_t index = currentNode->ChooseSubtree(mbr);
            currentNode = currentNode->GetEntry(index).childNode;
        }
        return currentNode;
    }

    /**
     * @brief Adjusts the tree after an insertion, propagating MBR updates upward
     * and handling node splits when they occur.
     * @param node The node where adjustments begin (typically the leaf).
     * @param splitNode Optional node produced by a split that must be inserted into the parent.
     */
    void AdjustTree(NodeType* node, NodeType* splitNode) {
        while (!node->IsRoot()) {
            NodeType* parent = FindParent(m_root, node);
            if (!parent) break;
            
            size_t index;
            if (FindChildIndex(parent, node, index)) {
                parent->GetEntry(index).mbr = node->GetMBR();
                
                if (splitNode) {
                    EntryType newEntry(splitNode->GetMBR(), splitNode);
                    if (!parent->IsFull()) {
                        parent->AddEntry(newEntry);
                        splitNode = nullptr;
                    } else {
                        HandleOverflow(parent, newEntry);
                        return;
                    }
                }
            }
            
            node = parent;
        }
        
        if (splitNode) {
            CreateNewRoot(node, splitNode);
        }
    }



}    

#endif // _RTREE_H_ //