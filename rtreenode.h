#ifndef __RTREENODE_H__
#define __RTREENODE_H__

#include "rtree_rectangle.h"
#include <cstddef>
#include <vector>
#include <iostream>
#include <string>

using namespace std;

template <typename Trait>
class RTreeNode;

template <typename Trait>
struct Branch
{
    using RectType  = Rectangle<Trait>;
    using NodeType  = RTreeNode<Trait>;
    using ObjIDType = typename Trait::ObjIDType;

    RectType    m_rect;     // mbr
    NodeType*   m_Child;    // if internal
    ObjIDType   m_Data;     // if leaf

    // clear all
    Branch(): m_Child(nullptr), m_Data(0) {}
};

template <typename Trait>
class RTreeNode
{
    public:
        using RectType      = Rectangle<Trait>;
        using NodeType      = RTreeNode<Trait>;
        using BranchType    = Branch<Trait>;
        using ObjIDType     = typename Trait::ObjIDType;
        using ElemType      = typename Trait::ElemType;

    static const size_t MAX_NODES = 4;
    static const size_t MIN_NODES = 2;

    size_t m_Level, m_Count;

    vector<BranchType> m_Branches;

    RTreeNode(size_t level): m_Level(level), m_Count(0)
    {
        // + 1 for temporary overflow
        m_Branches.resize(MAX_NODES + 1);
    }

    ~RTreeNode()
    {
        if (m_Level > 0) // internal node
        {
            for (size_t i = 0; i < m_Count; i++)
            {
                delete m_Branches[i].m_Child;
            }
        }
    }

    bool isLeaf() { return m_Level == 0; }
    bool isUnderflow() { return m_Count < MIN_NODES; }
    bool isFull() { return m_Count == MAX_NODES; }
    bool isEmpty() { return m_Count == 0; }
    bool hasSingleChild() { return m_Count == 1; }

    bool addBranch(BranchType& branch)
    {
        if (isFull()) return false;
        m_Branches[m_Count++] = branch;

        return true;
    }

    RectType getNodeMBR()
    {
        if (isEmpty()) return RectType();

        RectType mbr = m_Branches[0].m_rect;
        for (size_t i = 1; i < m_Count; i++)
        {
            mbr.Union(m_Branches[i].m_rect);
        }
        return mbr;
    }

    void Print(ostream& os, size_t indent = 0)
    {
        string tab(indent * 2, ' ');
        os << tab << "Node Level: " << m_Level << ", Count: " << m_Count << endl;
        for (size_t i = 0; i < m_Count; i++)
        {
            os << tab << " Branch " << i << ": ";
            m_Branches[i].m_rect.Print(os);
            if (!isLeaf())
            {
                os << endl;
                m_Branches[i].m_Child->Print(os, indent + 4);
            }
            else
            {
                os << ", Data ID: " << m_Branches[i].m_Data << endl;
            }
        }
    }

    // format: "Node Level: X, Count: Y"
    bool Read(istream& is)
    {
        string word;
        char comma;

        // "Node Level: X, Count: Y"
        is >> word >> word;      // "Node" "Level:"
        is >> m_Level;           // X
        is >> comma;             // ","
        is >> word;              // "Count:"
        is >> m_Count;           // Y

        if (!is) return false;

        // all branches
        for (size_t i = 0; i < m_Count; ++i)
        {
            is >> word >> word;  // "Branch" "N:"

            if (!m_Branches[i].m_rect.Read(is)) return false;

            if (isLeaf())
            {
                // ", Data ID: X"
                is >> comma;             // ","
                is >> word >> word;      // "Data" "ID:"
                is >> m_Branches[i].m_Data;
                m_Branches[i].m_Child = nullptr;
            }
            else
            {
                m_Branches[i].m_Child = new NodeType(0);
                if (!m_Branches[i].m_Child->Read(is)) return false;
            }
        }
        return true;
    }

    void PickSeeds(size_t& seed1, size_t& seed2)
    {
        ElemType maxWaste = -1;
        // for all pairs of rectangles
        for (size_t i = 0; i < m_Count; i++)
        {
            RectType rectI = m_Branches[i].m_rect;
            for (size_t j = i + 1; j < m_Count; j++)
            {
                RectType rectJ = m_Branches[j].m_rect;
                RectType combinedRect = rectI.Union_(rectJ);
                // combinedRect.Union(rectJ);
                ElemType waste = combinedRect.Area() - rectI.Area() - rectJ.Area();
                if (waste > maxWaste)
                {
                    maxWaste = waste;
                    seed1 = i;
                    seed2 = j;
                }
            }
        }
    }

    RTreeNode* SplitNode()
    {
        RTreeNode* newNode = new RTreeNode(m_Level);
        size_t seed1, seed2;
        PickSeeds(seed1, seed2);

        vector<BranchType> tempBranches = m_Branches;
        size_t totalItems = m_Count;

        // clean this and newNode to start fresh
        m_Count = 0;
        newNode -> m_Count = 0;

        this -> addBranch(tempBranches[seed1]);
        newNode -> addBranch(tempBranches[seed2]);

        vector<bool> taken(totalItems, false);
        taken[seed1] = true;
        taken[seed2] = true;

        // distribute remaining branches
        for (size_t i = 0; i < totalItems; i++)
        {
            // d
            if (taken[i]) continue;

            BranchType& curr = tempBranches[i];
            ElemType areaIncreaseThis = _CalculateAreaIncrease(curr.m_rect, this);
            ElemType areaIncreaseNew = _CalculateAreaIncrease(curr.m_rect, newNode);
            if (areaIncreaseThis < areaIncreaseNew)
                this -> addBranch(curr);
            else
                newNode -> addBranch(curr);
        }
        return newNode;
    }

    ElemType _CalculateAreaIncrease(RectType& rect, RTreeNode* node)
    {
        RectType nodeMBR = node -> getNodeMBR();
        ElemType areaBefore = nodeMBR.Area();

        RectType combinedRect = nodeMBR.Union_(rect);
        // combinedRect.Union(rect);
        ElemType areaAfter = combinedRect.Area();

        return areaAfter - areaBefore;
    }
};

#endif // __RTREENODE_H__
