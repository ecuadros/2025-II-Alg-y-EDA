#ifndef __RTREE_H__
#define __RTREE_H__

#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include "general_iterator.h"

template <typename T, int D>
struct Point {
    T coords[D];

    Point() {
        for(int i=0; i<D; ++i) coords[i] = 0;
    }
    
    Point(std::initializer_list<T> l) {
        int i=0;
        for(auto val : l) {
            if(i<D) coords[i++] = val;
        }
    }

    bool operator==(const Point& other) {
        for(int i=0; i<D; ++i) if(coords[i] != other.coords[i]) return false;
        return true;
    }

    friend std::ostream& operator<<(std::ostream& os, const Point& p) {
        os << "(";
        for(int i=0; i<D; ++i) os << p.coords[i] << (i<D-1?",":"");
        os << ")";
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Point& p) {
        char c;
        is >> c; 
        for(int i=0; i<D; ++i) {
            is >> p.coords[i];
            if(i<D-1) is >> c; 
        }
        is >> c; 
        return is;
    }
};

template <typename T, int D>
struct Rect {
    Point<T, D> minPt;
    Point<T, D> maxPt;

    Rect() {}
    Rect(Point<T, D> minP, Point<T, D> maxP) : minPt(minP), maxPt(maxP) {}

    double area() {
        double a = 1.0;
        for(int i=0; i<D; ++i) a *= (maxPt.coords[i] - minPt.coords[i]);
        return a;
    }

    bool overlaps(const Rect& other) {
        for(int i=0; i<D; ++i) {
            if (minPt.coords[i] > other.maxPt.coords[i] || maxPt.coords[i] < other.minPt.coords[i]) return false;
        }
        return true;
    }

    double expansionNeeded(const Rect& other) {
        double expandedArea = 1.0;
        for(int i=0; i<D; ++i) {
            T minC = std::min(minPt.coords[i], other.minPt.coords[i]);
            T maxC = std::max(maxPt.coords[i], other.maxPt.coords[i]);
            expandedArea *= (maxC - minC);
        }
        return expandedArea - area();
    }

    void expand(const Rect& other) {
        for(int i=0; i<D; ++i) {
            minPt.coords[i] = std::min(minPt.coords[i], other.minPt.coords[i]);
            maxPt.coords[i] = std::max(maxPt.coords[i], other.maxPt.coords[i]);
        }
    }

    static Rect boundingBox(const Rect& r1, const Rect& r2) {
        Rect res = r1;
        res.expand(r2);
        return res;
    }

    friend std::ostream& operator<<(std::ostream& os, const Rect& r) {
        os << "[" << r.minPt << ";" << r.maxPt << "]";
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Rect& r) {
        char c; 
        is >> c; 
        is >> r.minPt >> c >> r.maxPt; 
        is >> c; 
        return is;
    }
};

template <typename Traits> class CRTree;

template <typename Traits>
class CRTreeNode {
    friend class CRTree<Traits>;
public:
    using CoordinateType = typename Traits::CoordinateType;
    using DataType       = typename Traits::DataType;
    static const int DIM = Traits::D;
    static const int M   = Traits::M;
    static const int m   = Traits::m;
    
    using Node = CRTreeNode<Traits>;
    using RectType = Rect<CoordinateType, DIM>;

    struct Entry {
        RectType    mbr;
        Node*       pChild = nullptr; 
        DataType    data;             
        
        bool operator==(const Entry& other) {
            return pChild == other.pChild; 
        }
    };

protected:
    bool           m_isLeaf;
    std::vector<Entry>  m_entries;
    Node*          m_pParent = nullptr;

public:
    CRTreeNode(bool leaf = true) : m_isLeaf(leaf), m_pParent(nullptr) {}

    virtual ~CRTreeNode() {
        if (!m_isLeaf) {
            for (auto& e : m_entries) {
                if(e.pChild) delete e.pChild;
            }
        }
    }

    bool isLeaf() { return m_isLeaf; }
    
    RectType getMBR() {
        if (m_entries.empty()) return RectType();
        RectType res = m_entries[0].mbr;
        for (size_t i = 1; i < m_entries.size(); ++i) {
            res.expand(m_entries[i].mbr);
        }
        return res;
    }

    Entry& getEntry(size_t index) { return m_entries[index]; }
    size_t getCount() { return m_entries.size(); }
    
    DataType& getDataRef(size_t index) { return m_entries[index].data; }
};

template <typename Container>
class rtree_iterator : public general_iterator<Container, rtree_iterator<Container>> {
public:
    using Parent = general_iterator<Container, rtree_iterator<Container>>;
    using Node   = typename Container::Node;
    using ValueType = typename Container::DataType; 

    int m_index; 

    rtree_iterator(Container* pContainer, Node* pNode, int index) 
        : Parent(pContainer, pNode), m_index(index) {}

    rtree_iterator& operator++() {
        if (!Parent::m_pNode) return *this;

        if (m_index + 1 < Parent::m_pNode->getCount()) {
            m_index++;
        } else {
            Node* current = Parent::m_pNode;
            Node* parent = current->m_pParent;
            
            while (parent) {
                int siblingIndex = -1;
                for(size_t i=0; i<parent->getCount(); ++i) {
                    if (parent->getEntry(i).pChild == current) {
                        siblingIndex = i;
                        break;
                    }
                }
                
                if (siblingIndex != -1 && siblingIndex + 1 < parent->getCount()) {
                    Node* nextSibling = parent->getEntry(siblingIndex + 1).pChild;
                    Parent::m_pNode = findFirstLeaf(nextSibling);
                    m_index = 0;
                    return *this;
                }
                current = parent;
                parent = current->m_pParent;
            }
            Parent::m_pNode = nullptr;
            m_index = 0;
        }
        return *this;
    }

    ValueType& operator*() {
        return Parent::m_pNode->getDataRef(m_index);
    }
    
private:
    Node* findFirstLeaf(Node* node) {
        if (!node) return nullptr;
        while (!node->isLeaf()) {
            if (node->getCount() == 0) return node; 
            node = node->getEntry(0).pChild;
        }
        return node;
    }
};

template <typename Traits>
class CRTree {
public:
    using Node           = CRTreeNode<Traits>;
    using CoordinateType = typename Traits::CoordinateType;
    using DataType       = typename Traits::DataType;
    using RectType       = Rect<CoordinateType, Traits::D>;
    using Entry          = typename Node::Entry;
    using iterator       = rtree_iterator<CRTree<Traits>>;

protected:
    Node* m_pRoot = nullptr;
    size_t m_size = 0;

public:
    CRTree() {
        m_pRoot = new Node(true); 
    }

    virtual ~CRTree() {
        if(m_pRoot) delete m_pRoot;
    }

    void insert(const RectType& r, const DataType& data) {
        Entry e;
        e.mbr = r;
        e.data = data;
        e.pChild = nullptr;
        
        insert_entry(m_pRoot, e);
        m_size++;
    }

    std::vector<DataType> search(const RectType& query) {
        std::vector<DataType> results;
        search_rec(m_pRoot, query, results);
        return results;
    }

    void Write(std::ostream& os) {
        write_rec(m_pRoot, os);
    }
    
    void Read(std::istream& is) {
        if(m_pRoot) delete m_pRoot;
        m_pRoot = read_rec(is, nullptr);
    }
    
    bool remove(const RectType& r, const DataType& data) {
        Node* leaf = find_leaf(m_pRoot, r, data);
        if (!leaf) return false;
        
        int entryIdx = -1;
        for(size_t i=0; i<leaf->getCount(); ++i) {
            if (leaf->m_entries[i].data == data) {
                entryIdx = i;
                break;
            }
        }
        
        if (entryIdx == -1) return false; 
        
        leaf->m_entries.erase(leaf->m_entries.begin() + entryIdx);
        
        condense_tree(leaf);
        
        if (!m_pRoot->isLeaf() && m_pRoot->getCount() == 1) {
            Node* newRoot = m_pRoot->m_entries[0].pChild;
            m_pRoot->m_entries.clear(); 
            delete m_pRoot;
            m_pRoot = newRoot;
            m_pRoot->m_pParent = nullptr;
        }
        
        if(m_pRoot->getCount() == 0 && !m_pRoot->isLeaf()) {
            delete m_pRoot;
            m_pRoot = new Node(true);
        }
        
        m_size--;
        return true;
    }

private:
    Node* find_leaf(Node* node, const RectType& r, const DataType& data) {
        if (!node) return nullptr;
        if (node->isLeaf()) {
            for(auto& e : node->m_entries) {
                if (e.data == data) return node; 
            }
            return nullptr;
        } else {
            for(auto& e : node->m_entries) {
                if (e.mbr.overlaps(r) || e.mbr.expansionNeeded(r) == 0) { 
                    Node* res = find_leaf(e.pChild, r, data);
                    if (res) return res;
                }
            }
        }
        return nullptr;
    }

    void condense_tree(Node* node) {
        Node* p = node;
        std::vector<Node*> q; 
        
        while (p != m_pRoot) {
            Node* parent = p->m_pParent;
            int min_entries = Traits::m;
            
            if (p->getCount() < (size_t)min_entries) {
                int pIdx = -1;
                for(size_t i=0; i<parent->getCount(); ++i) {
                    if (parent->m_entries[i].pChild == p) {
                        pIdx = i;
                        break;
                    }
                }
                if (pIdx != -1) {
                    parent->m_entries.erase(parent->m_entries.begin() + pIdx);
                }
                q.push_back(p);
            } else {
                for(auto& e : parent->m_entries) {
                    if (e.pChild == p) {
                        e.mbr = p->getMBR();
                        break;
                    }
                }
            }
            p = parent;
        }

        for(Node* orphaned : q) {
            reinsert_node_entries(orphaned);
            orphaned->m_entries.clear(); 
            delete orphaned;
        }
    }

    void reinsert_node_entries(Node* node) {
        if (node->isLeaf()) {
            for(const auto& e : node->m_entries) {
                insert(e.mbr, e.data);
            }
        } else {
            for(const auto& e : node->m_entries) {
                reinsert_node_entries(e.pChild);
                delete e.pChild; 
            }
        }
    }

protected: 
    void insert_entry(Node* node, Entry& e) {
        if (node->isLeaf()) {
            node->m_entries.push_back(e);
            if (node->m_entries.size() > Traits::M) {
                split_node(node);
            }
        } else {
            Node* child = choose_subtree(node, e.mbr);
            insert_entry(child, e);
            
            adjust_tree(node); 
        }
    }
    
    Node* choose_subtree(Node* node, const RectType& r) {
        double minEnl = 1e300; 
        int bestIdx = -1;
        
        for(size_t i=0; i<node->getCount(); ++i) {
            double expansion = node->m_entries[i].mbr.expansionNeeded(r);
            if(expansion < minEnl) {
                minEnl = expansion;
                bestIdx = i;
            } else if (expansion == minEnl) {
                if (node->m_entries[i].mbr.area() < node->m_entries[bestIdx].mbr.area()) {
                    bestIdx = i;
                }
            }
        }
        return node->m_entries[bestIdx].pChild;
    }
    
    void adjust_tree(Node* node) {
        for(auto& entry : node->m_entries) {
            if (entry.pChild) {
                entry.mbr = entry.pChild->getMBR();
            }
        }
    }

    void split_node(Node* node) {
        Node* newNode = new Node(node->isLeaf());
        newNode->m_pParent = node->m_pParent;

        int seed1, seed2;
        pick_seeds(node->m_entries, seed1, seed2);

        std::vector<Entry> group1, group2;
        group1.push_back(node->m_entries[seed1]);
        group2.push_back(node->m_entries[seed2]);
        
        if(seed1 > seed2) std::swap(seed1, seed2);
        
        std::vector<Entry> remaining;
        for(size_t i=0; i<node->m_entries.size(); ++i) {
            if((int)i != seed1 && (int)i != seed2) remaining.push_back(node->m_entries[i]);
        }

        RectType mbr1 = group1[0].mbr;
        RectType mbr2 = group2[0].mbr;
        
        for(const auto& entry : remaining) {
            if (Traits::M + 1 - (group1.size() + group2.size()) == Traits::m - group1.size()) {
                group1.push_back(entry);
                continue;
            }
            if (Traits::M + 1 - (group1.size() + group2.size()) == Traits::m - group2.size()) {
                group2.push_back(entry);
                continue;
            }

            double d1 = mbr1.expansionNeeded(entry.mbr);
            double d2 = mbr2.expansionNeeded(entry.mbr);

            if(d1 < d2) {
                group1.push_back(entry);
                mbr1.expand(entry.mbr);
            } else {
                group2.push_back(entry);
                mbr2.expand(entry.mbr);
            }
        }

        node->m_entries = group1;
        newNode->m_entries = group2;
        
        if (!node->isLeaf()) {
            for(auto& e : node->m_entries) if(e.pChild) e.pChild->m_pParent = node;
            for(auto& e : newNode->m_entries) if(e.pChild) e.pChild->m_pParent = newNode;
        }

        if (!node->m_pParent) {
            Node* newRoot = new Node(false);
            Entry e1; e1.pChild = node; e1.mbr = node->getMBR();
            Entry e2; e2.pChild = newNode; e2.mbr = newNode->getMBR();
            newRoot->m_entries.push_back(e1);
            newRoot->m_entries.push_back(e2);
            node->m_pParent = newRoot;
            newNode->m_pParent = newRoot;
            m_pRoot = newRoot;
        } else {
            Node* parent = node->m_pParent;
            for(auto& e : parent->m_entries) {
                if(e.pChild == node) {
                    e.mbr = node->getMBR();
                    break;
                }
            }
            Entry eNew; eNew.pChild = newNode; eNew.mbr = newNode->getMBR();
            parent->m_entries.push_back(eNew);
            
            if (parent->m_entries.size() > Traits::M) {
                split_node(parent);
            }
        }
    }

    void pick_seeds(const std::vector<Entry>& entries, int& seed1, int& seed2) {
        double maxNormalizedSep = -1.0;
        
        seed1 = 0; seed2 = 1;
        
        RectType totalMBR = entries[0].mbr;
        for(size_t i=1; i<entries.size(); ++i) totalMBR.expand(entries[i].mbr);

        for (int d = 0; d < Traits::D; ++d) {
            int highestLowIdx = -1; 
            CoordinateType highestLow = -1e9; 
            int lowestHighIdx = -1;
            CoordinateType lowestHigh = 1e9; 
            
            for(size_t i=0; i<entries.size(); ++i) {
                if(entries[i].mbr.minPt.coords[d] > highestLow || highestLowIdx == -1) {
                    highestLow = entries[i].mbr.minPt.coords[d];
                    highestLowIdx = i;
                }
                if(entries[i].mbr.maxPt.coords[d] < lowestHigh || lowestHighIdx == -1) {
                    lowestHigh = entries[i].mbr.maxPt.coords[d];
                    lowestHighIdx = i;
                }
            }
            
            if (highestLowIdx != -1 && lowestHighIdx != -1 && highestLowIdx != lowestHighIdx) {
                double sep = std::abs((double)highestLow - (double)lowestHigh);
                double width = std::abs((double)totalMBR.maxPt.coords[d] - (double)totalMBR.minPt.coords[d]);
                double normSep = (width > 1e-9) ? sep/width : sep;
                
                if (normSep > maxNormalizedSep) {
                    maxNormalizedSep = normSep;
                    seed1 = highestLowIdx;
                    seed2 = lowestHighIdx;
                }
            }
        }
        if (seed1 == seed2 && entries.size() > 1) {
            seed2 = (seed1 + 1) % entries.size();
        }
    }

    void search_rec(Node* node, const RectType& query, std::vector<DataType>& results) {
        if (!node) return;
        
        for (auto& e : node->m_entries) {
            if (e.mbr.overlaps(query)) {
                if (node->isLeaf()) {
                    results.push_back(e.data);
                } else {
                    search_rec(e.pChild, query, results);
                }
            }
        }
    }

    void write_rec(Node* node, std::ostream& os) {
        if (!node) return;
        os << node->isLeaf() << " " << node->getCount() << std::endl;
        for (size_t i = 0; i < node->getCount(); ++i) {
            os << node->m_entries[i].mbr << " ";
            if (node->isLeaf()) {
                os << node->m_entries[i].data << std::endl;
            } else {
                os << std::endl;
                write_rec(node->m_entries[i].pChild, os);
            }
        }
    }

    Node* read_rec(std::istream& is, Node* parent) {
        bool isLeaf;
        size_t count;
        if (!(is >> isLeaf)) return nullptr;
        is >> count;
        
        Node* node = new Node(isLeaf);
        node->m_pParent = parent;
        
        for (size_t i = 0; i < count; ++i) {
            Entry e;
            is >> e.mbr;
            if (isLeaf) {
                is >> e.data;
                e.pChild = nullptr;
            } else {
                e.pChild = read_rec(is, node);
            }
            node->m_entries.push_back(e);
        }
        return node;
    }
};

void DemoRTree();

#endif
