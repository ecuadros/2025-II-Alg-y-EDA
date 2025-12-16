#ifndef __RTREE_H__
#define __RTREE_H__

#include "rtreenode.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <limits> // infinity

using namespace std;

template <typename Trait>
class RTree
{
    public:
        using NodeType      = RTreeNode<Trait>;
        using RectType      = Rectangle<Trait>;
        using BranchType    = Branch<Trait>;
        using ObjIDType     = typename Trait::ObjIDType;
        using ElemType      = typename Trait::ElemType;

    NodeType* m_Root;

    RTree()
    {
        m_Root = new NodeType(0); // start with leaf node
    }

    ~RTree()
    {
        // TODO: implement full tree deletion traversing the nodes
        // for now, just delete root (memory leak for other nodes)
        delete m_Root;
    }

    bool Insert(RectType& rect, const ObjIDType& id)
    {
        BranchType newBranch;
        newBranch.m_rect = rect;
        newBranch.m_Data = id;
        newBranch.m_Child = nullptr; // no child for leaf insertion

        NodeType* newChild = _Insert(m_Root, newBranch);
        if (newChild != nullptr)
        {
            // level increase because root was split
            NodeType* newRoot = new NodeType(m_Root -> m_Level + 1);

            BranchType rootBranch1;
            rootBranch1.m_rect = m_Root -> getNodeMBR();
            rootBranch1.m_Child = m_Root;
            newRoot -> addBranch(rootBranch1);

            // new child branch
            BranchType rootBranch2;
            rootBranch2.m_rect = newChild -> getNodeMBR();
            rootBranch2.m_Child = newChild;
            newRoot -> addBranch(rootBranch2);

            m_Root = newRoot;
        }

        return true;
    }

    size_t Search(RectType& searchRect, vector<ObjIDType>& results)
    {
        results.clear();
        _Search(m_Root, searchRect, results);
        return results.size();
    }

    friend ostream& operator<<(ostream& os, RTree& tree)
    {
        tree.m_Root -> Print(os, 0);
        return os;
    }

    friend istream& operator>>(istream& is, RTree& tree)
    {
        // because we delete existing root
        delete tree.m_Root;

        tree.m_Root = new NodeType(0);
        if (!tree.m_Root->Read(is))
        {
            delete tree.m_Root;
            tree.m_Root = new NodeType(0);
        }
        return is;
    }

    bool Remove(RectType& rect, const ObjIDType& id)
    {
        vector<BranchType> orphans;
        bool removed = _Remove(m_Root, rect, id, orphans);
        if (removed) return true;

        if (m_Root -> isEmpty() && !(m_Root -> isLeaf()))
        {
            delete m_Root;
            m_Root = new NodeType(0);
        }
        else if (m_Root -> hasSingleChild() && !(m_Root -> isLeaf()))
        {
            // if root has single child, make child the new root
            NodeType* singleChild = m_Root -> m_Branches[0].m_Child;
            m_Root -> m_Branches[0].m_Child = nullptr;
            delete m_Root;
            m_Root = singleChild;
        }

        for (auto& branch : orphans)
        {
            _ReInsert(branch);
        }
        return true;
    }

    private:
        NodeType* _Insert(NodeType* node, BranchType& branch)
        {
            if (node -> isLeaf())
            {
                if (node -> addBranch(branch)) return nullptr;
                else
                {
                    // overflow so split
                    // add the branch first
                    // then split
                    node -> m_Branches[node -> m_Count++] = branch;
                    return node -> SplitNode();
                }
            }
            size_t bestIndex = _ChooseSubtree(node, branch.m_rect);
            BranchType& bestBranch = node -> m_Branches[bestIndex];
            NodeType* newChild = _Insert(bestBranch.m_Child, branch);

            bestBranch.m_rect = bestBranch.m_Child -> getNodeMBR();

            if (newChild != nullptr)
            {
                // child was split, need to add new branch
                BranchType newBranch;
                newBranch.m_rect = newChild -> getNodeMBR();
                newBranch.m_Child = newChild;

                if (node -> addBranch(newBranch)) return nullptr;
                else
                {
                    // overflow so split
                    // add the branch first
                    // then split
                    node -> m_Branches[node -> m_Count++] = newBranch;
                    return node -> SplitNode();
                }
            }
            return nullptr;
        }

        size_t _ChooseSubtree(NodeType* node, RectType& rect)
        {
            size_t bestIndex = -1;
            ElemType minEnlargement = numeric_limits<ElemType>::max();
            ElemType minArea = numeric_limits<ElemType>::max();

            // all children
            for (size_t i = 0; i < node -> m_Count; i++)
            {
                RectType tmpRect = node -> m_Branches[i].m_rect;
                ElemType areaBefore = tmpRect.Area();

                RectType combinedRect = tmpRect.Union_(rect);
                // combinedRect.Union(rect);
                ElemType areaAfter = combinedRect.Area();

                ElemType enlargement = areaAfter - areaBefore;
                if (enlargement < minEnlargement)
                {
                    minEnlargement = enlargement;
                    bestIndex = i;
                    minArea = areaBefore;
                }
                else if (enlargement == minEnlargement)
                {
                    if (areaBefore < minArea)
                    {
                        bestIndex = i;
                        minArea = areaBefore;
                    }
                }
            }

            return bestIndex;
        }

        void _Search(NodeType* node, RectType& searchRect, vector<ObjIDType>& results)
        {
            for (size_t i = 0; i < node -> m_Count; ++i)
            {
                BranchType& branch = node -> m_Branches[i];
                if (branch.m_rect.Intersects(searchRect))
                {
                    if (node -> isLeaf())
                    {
                        results.push_back(branch.m_Data);
                    }
                    else
                    {
                        _Search(branch.m_Child, searchRect, results);
                    }
                }
            }
        }

        void _ReInsert(BranchType& branch)
        {
            if (branch.m_Child != nullptr)
            {
                for (size_t i = 0; i < branch.m_Child -> m_Count; ++i)
                {
                    _ReInsert(branch.m_Child -> m_Branches[i]);
                }
                delete branch.m_Child;
            }
            else
            {
                Insert(branch.m_rect, branch.m_Data);
            }
        }

        bool _Remove(NodeType* node, RectType& rect, ObjIDType id, vector<BranchType>& orphans)
        {
            if (node -> isLeaf())
            {
                for (size_t i = 0; i < node -> m_Count; ++i)
                {
                    if (node -> m_Branches[i].m_Data == id)
                    {
                        // found!
                        node -> RemoveBranch(i);
                        return true;
                    }
                }
                return false;
            }
            else
            {
                for (size_t i = 0; i < node -> m_Count; ++ i)
                {
                    RectType& currentRectangle = node -> m_Branches[i].m_rect;
                    if (currentRectangle.Intersects(rect))
                    {
                        NodeType* childNode = node -> m_Branches[i].m_Child;
                        bool removed = _Remove(childNode, rect, id, orphans);
                        if (removed)
                        {
                            if (childNode -> isUnderflow())
                            {
                                for (size_t k = 0; k < childNode -> m_Count; ++k)
                                {
                                    orphans.push_back(childNode -> m_Branches[k]);
                                }

                                for (size_t k = 0; k < childNode -> m_Count; ++ k)
                                {
                                    childNode -> m_Branches[k].m_Child = nullptr;
                                }

                                delete childNode;
                                node -> RemoveBranch(i);
                            }
                            else
                            {
                                node -> m_Branches[i].m_rect = childNode -> getNodeMBR();
                            }
                            return true;
                        }
                    }
                }
                return false;
            }
        }
};

#endif // __RTREE_H__
