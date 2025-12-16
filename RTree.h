#ifndef __RTREE_H__
#define __RTREE_H__

#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <string>
#include "RTreePage.h"

// R-TREE

template <typename Traits>
class RTree {
    typedef typename Traits::PointType  PointType;
    typedef typename Traits::RectType   RectType;
    typedef typename Traits::ObjIDType  ObjIDType;
    typedef RTreePage<Traits>           RTNode;

    public:

    
    RTree(size_t order) :  m_root(nullptr), m_order(order), m_NumKeys(0), m_Height(1) {
        m_root = new RTNode(true, m_order);
    }

    ~RTree() {
        delete m_root;
    }

    bool Insert (const PointType point, const ObjIDType id);
    bool Remove(const PointType& point, const ObjIDType& id);
    bool Save(const std::string& filename);
    bool Load(const std::string& filename);

    size_t size() const { return m_NumKeys; }
    size_t height() const { return m_Height; }
    size_t getOrder() const { return m_order; }

    using ObjectInfo = std::pair<PointType, ObjIDType>;
    std::vector<ObjectInfo> RangeQuery (const RectType& region) {
        std::vector<ObjectInfo> results;
        m_root->RangeQuery(region, results);
        return results;
    }

    using Neighbour = typename RTNode::Neighbour;
    std::vector<Neighbour> KNN (const PointType& point, size_t k) {
        std::vector<Neighbour> results;
        results.reserve(k + 1);
        m_root->KNN(point, k, results);
        return results;
    } 

    protected:
    RTNode* m_root;
    size_t m_order;
    size_t m_NumKeys;
    size_t m_Height;    
};

template <typename Traits>
bool RTree<Traits>::Insert (const PointType point, const ObjIDType id) {
    
    typename RTNode::Branch newBranch(point, id);
    RTNode* newPage = nullptr;
    typename Traits::RectType newMBR;

    rt_ErrorCode res = m_root->insert(newBranch, &newPage, &newMBR);

    if (res == rt_overflow) {
        RTNode* newRoot = new RTNode(false, m_order);
        typename RTNode::Branch oldBranch(m_root, m_root->m_NodeMBR);
        typename RTNode::Branch newBranch(newPage, newMBR);

        newRoot->m_branches.push_back(oldBranch);
        newRoot->m_branches.push_back(newBranch);
        newRoot->updateMBR();

        m_root = newRoot;
        m_Height++;
    }
    m_NumKeys++;
    return true;
}

template <typename Traits>
bool RTree<Traits>::Remove(const PointType& point, const ObjIDType& id){
    if (!m_root) {
        return false;
    }

    bool found = false;
    bool rootEmpty = m_root->Remove(point, id, found);

    if (found) {
        m_NumKeys--;
        if (rootEmpty) {
            delete m_root;
            m_root = new RTNode(true, m_order);
            m_Height = 1;
        }
        else if (!m_root->m_isLeaf && m_root->m_branches.size() == 1) {
            RTNode* newRoot = m_root->m_branches[0].child;
            m_root->m_branches.clear();
            delete m_root;

            m_root = newRoot;
            m_Height--;
        }
    }
    return found;
}

template <typename Traits>
bool RTree<Traits>::Save(const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) return false;

    outFile << "RTree_v1" << "\n";
    outFile << "Order: " << m_order << "\n";
    outFile << "NumKeys: " << m_NumKeys << "\n";
    outFile << "Height: " << m_Height << "\n";
    
    bool hasRoot = (m_root != nullptr);
    outFile << "HasRoot: " << hasRoot << "\n";

    if (hasRoot) {
        m_root->WriteToDisk(outFile, "");
    }

    outFile.close();
    return true;
}

template <typename Traits>
bool RTree<Traits>::Load(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) return false;

    delete m_root;
    m_root = nullptr;
    m_NumKeys = 0;

    std::string label, headerVersion;
    
    inFile >> headerVersion;
    if (headerVersion != "RTree_v1") return false;

    inFile >> label >> m_order;
    inFile >> label >> m_NumKeys;
    inFile >> label >> m_Height;

    bool hasRoot;
    inFile >> label >> hasRoot;

    if (hasRoot) {
        m_root = new RTNode(true, m_order);
        m_root->ReadFromText(inFile);
    } else {
        m_root = new RTNode(true, m_order);
    }

    inFile.close();
    return true;
}

#endif