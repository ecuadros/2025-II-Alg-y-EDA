#ifndef __RTREE_H__
#define __RTREE_H__

#include "rtreepage.h"
#include <fstream>
#include <utility>
#include <functional>
#include <cmath>

template <typename Trait>
class RTree
{
    public:
        typedef typename Trait::CoordsType CoordsType;
        typedef typename Trait::DataType DataType;
        typedef Rect<Trait> RectType;
        typedef RTreeEntry<Trait> Entry;
        typedef RTreeNode<Trait> Node;
        
        // Results of range query (MBR + Data)
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
      
       // Insert element with MBR and data
       void Insert(CoordsType xMin, CoordsType yMin, CoordsType xMax, CoordsType yMax, const DataType& data)
       {
              Entry entry;
              entry.rect = RectType{xMin, xMax, yMin, yMax};
              entry.data = data;
              entry.child = nullptr;
              
              Node* newNode = nullptr;
              if (InsertRec(entry, m_Root, newNode, 0))
              {
                     // Root split, create new root
                     Node* newRoot = new Node();
                     newRoot->level = m_Root->level + 1;
                     
                     Entry e1, e2;
                     e1.rect = m_Root->CalculateMBR();
                     e1.child = m_Root;
                     e2.rect = newNode->CalculateMBR();
                     e2.child = newNode;
                     
                     newRoot->entries.push_back(e1);
                     newRoot->entries.push_back(e2);
                     m_Root = newRoot;
              }
              ++m_Count;
       }
       
       // Delete element looking for MBR and data
       bool Remove(CoordsType xMin, CoordsType yMin, CoordsType xMax, CoordsType yMax, const DataType& data)
       {
              RectType targetRect{xMin, xMax, yMin, yMax};
              std::vector<Node*> reinsertList;
              
              if (RemoveRec(targetRect, data, m_Root, reinsertList))
                     return false;  // Not found
              
              // Reinsert entries of nodes with underflow (CondenseTree)
              for (Node* node : reinsertList)
              {
                     ReinsertEntries(node);
                     delete node;
              }
              
              while (!m_Root->IsLeaf() && m_Root->entries.size() == 1)
              {
                     Node* oldRoot = m_Root;
                     m_Root = m_Root->entries[0].child;
                     delete oldRoot;
              }
              --m_Count;
              return true;
       }
       
       bool Remove(const DataType& data)
       {
              std::vector<Node*> reinsertList;
              
              if (RemoveByDataRec(data, m_Root, reinsertList))
                     return false;
              
              for (Node* node : reinsertList)
              {
                     ReinsertEntries(node);
                     delete node;
              }
              
              while (!m_Root->IsLeaf() && m_Root->entries.size() == 1)
              {
                     Node* oldRoot = m_Root;
                     m_Root = m_Root->entries[0].child;
                     delete oldRoot;
              }
              --m_Count;
              return true;
       }

       // RangeQuery: Return all elements that intersects with the given MBR
       std::vector<QueryResult> RangeQuery(CoordsType xMin, CoordsType yMin, CoordsType xMax, CoordsType yMax) const
       {
              std::vector<QueryResult> results;
              RectType queryRect{xMin, xMax, yMin, yMax};
              SearchRec(m_Root, queryRect, results);
              return results;
       }
       
       // Write to disk
       bool Write(const std::string& filename) const
       {
              std::ofstream file(filename);
              if (!file) return false;
              
              // Header: Number of elements
              file << m_Count << "\n";
              WriteNode(file, m_Root);
              return true;
       }
       
       // Read from disk
       bool Read(const std::string& filename)
       {
              std::ifstream file(filename);
              if (!file) return false;
              
              // Read header
              file >> m_Count;
              Destroy(m_Root);
              m_Root = ReadNode(file);
              return true;
       }
       
       size_t size() const { return m_Count; }
       bool empty() const { return m_Count == 0; }
       
       void Print() const { Print(std::cout); }
       
       void Print(std::ostream& os) const
       {
              os << "RTree [" << m_Count << " elementos]\n";
              PrintNode(os, m_Root, 0);
       }

       // Variadic template traversal , applies function to all leaf entries
       template<typename Func, typename... Args>
       void ForEach(Func&& func, Args&&... args) const
       {
              ForEachRec(m_Root, std::forward<Func>(func), std::forward<Args>(args)...);
       }

       // Specialized print using custom function
       template<typename Func, typename... Args>
       void PrintWith(Func&& func, Args&&... args) const
       {
              std::cout << "RTree [" << m_Count << " elementos] - Custom traversal:\n";
              ForEach(std::forward<Func>(func), std::forward<Args>(args)...);
       }

private:
      
       bool IsFull(Node* node) const
       {
              return node->entries.size() >= Trait::MaxNodes;
       }
       
       bool HasUnderflow(Node* node) const
       {
              return node->entries.size() < Trait::MinNodes;
       }
       
       // Insert recursive, return true if split
       bool InsertRec(const Entry& entry, Node* node, Node*& newNode, int targetLevel)
       {
              if (node->level > targetLevel)
              {
                     size_t best = node->ChooseBestChild(entry.rect);
                     Node* childNew = nullptr;
                     bool split = InsertRec(entry, node->entries[best].child, childNew, targetLevel);
                     
                     // Update MBR  
                     node->entries[best].rect = node->entries[best].child->CalculateMBR();
                     
                     if (!split) return false;
                     
                     Entry newEntry;
                     newEntry.rect = childNew->CalculateMBR();
                     newEntry.child = childNew;
                     
                     if (!IsFull(node))
                     {
                            node->entries.push_back(newEntry);
                            return false;
                     }
                     Split(node, newEntry, newNode);
                     return true;
              }
              else
              {
                     // Leaf
                     if (!IsFull(node))
                     {
                            node->entries.push_back(entry);
                            return false;
                     }
                     Split(node, entry, newNode);
                     return true;
              }
       }
       
       // Guttman Quadratic split
       void Split(Node* node, const Entry& entry, Node*& newNode)
       {
              std::vector<Entry> all = node->entries;
              all.push_back(entry);
              
              // PickSeeds: pick two entries that would waste the most area if put in the same node
              size_t seed1 = 0, seed2 = 1;
              CoordsType worstWaste = all[0].rect.Combine(all[1].rect).Area()
                                   - all[0].rect.Area() - all[1].rect.Area();
              
              for (size_t i = 0; i < all.size() - 1; ++i)
              {
                     for (size_t j = i + 1; j < all.size(); ++j)
                     {
                            // Wasted space
                            CoordsType waste = all[i].rect.Combine(all[j].rect).Area()
                                            - all[i].rect.Area() - all[j].rect.Area();
                            if (waste > worstWaste)
                            {
                                   worstWaste = waste;
                                   seed1 = i;
                                   seed2 = j;
                            }
                     }
              }
              
              // Create new node and distribute seeds
              newNode = new Node();
              newNode->level = node->level;
              node->entries.clear();
              
              node->entries.push_back(all[seed1]);
              newNode->entries.push_back(all[seed2]);
              
              // Mark as used
              std::vector<bool> used(all.size(), false);
              used[seed1] = used[seed2] = true;
              
              // PickNext: Distribute the rest
              for (size_t remaining = all.size() - 2; remaining > 0; --remaining)
              {
                     if (node->entries.size() + remaining <= Trait::MinNodes)
                     {
                            for (size_t i = 0; i < all.size(); ++i)
                                   if (!used[i]) { node->entries.push_back(all[i]); used[i] = true; }
                            break;
                     }
                     if (newNode->entries.size() + remaining <= Trait::MinNodes)
                     {
                            for (size_t i = 0; i < all.size(); ++i)
                                   if (!used[i]) { newNode->entries.push_back(all[i]); used[i] = true; }
                            break;
                     }
                     
                     RectType mbr1 = node->CalculateMBR();
                     RectType mbr2 = newNode->CalculateMBR();
                     size_t bestIdx = 0;
                     CoordsType bestDiff = -1;
                     int bestGroup = 0;
                     
                     for (size_t i = 0; i < all.size(); ++i)
                     {
                            if (used[i]) continue;
                            CoordsType enl1 = mbr1.Enlargement(all[i].rect);
                            CoordsType enl2 = mbr2.Enlargement(all[i].rect);
                            CoordsType diff = std::abs(enl1 - enl2);
                            
                            if (bestDiff < 0 || diff > bestDiff)
                            {
                                   bestDiff = diff;
                                   bestIdx = i;
                                   bestGroup = (enl1 < enl2) ? 0 : 1;
                            }
                     }
                     used[bestIdx] = true;
                     if (bestGroup == 0)
                            node->entries.push_back(all[bestIdx]);
                     else
                            newNode->entries.push_back(all[bestIdx]);
              }
       }
       
       // Recursive delete (use MBR) 
       bool RemoveRec(const RectType& targetRect, const DataType& data, Node* node, std::vector<Node*>& reinsertList)
       {
              if (!node->IsLeaf())
              {
                     // Internal node
                     for (size_t i = 0; i < node->entries.size(); ++i)
                     {
                            // Only explore if child MBR contains target
                            if (!node->entries[i].rect.Contains(targetRect))
                                   continue;
                            
                            if (!RemoveRec(targetRect, data, node->entries[i].child, reinsertList))
                            {
                                   // Update child MBR
                                   if (node->entries[i].child->entries.empty())
                                   {
                                          delete node->entries[i].child;
                                          node->entries.erase(node->entries.begin() + i);
                                   }
                                   else
                                   {
                                          node->entries[i].rect = node->entries[i].child->CalculateMBR();
                                          
                                          if (HasUnderflow(node->entries[i].child))
                                          {
                                                 reinsertList.push_back(node->entries[i].child);
                                                 node->entries[i].child = nullptr;
                                                 node->entries.erase(node->entries.begin() + i);
                                          }
                                   }
                                   return false;  
                            }
                     }
                     return true;  // Not found
              }
              else
              {
                     // Leaf node
                     for (size_t i = 0; i < node->entries.size(); ++i)
                     {
                            if (node->entries[i].rect == targetRect && node->entries[i].data == data)
                            {
                                   node->entries.erase(node->entries.begin() + i);
                                   return false;  
                            }
                     }
                     return true;  // Not found
              }
       }
       
       // Search through all the tree. Just require data
       bool RemoveByDataRec(const DataType& data, Node* node, std::vector<Node*>& reinsertList)
       {
              if (!node->IsLeaf())
              {
                     for (size_t i = 0; i < node->entries.size(); ++i)
                     {
                            if (!RemoveByDataRec(data, node->entries[i].child, reinsertList))
                            {
                                   if (node->entries[i].child->entries.empty())
                                   {
                                          delete node->entries[i].child;
                                          node->entries.erase(node->entries.begin() + i);
                                   }
                                   else
                                   {
                                          node->entries[i].rect = node->entries[i].child->CalculateMBR();
                                          if (HasUnderflow(node->entries[i].child))
                                          {
                                                 reinsertList.push_back(node->entries[i].child);
                                                 node->entries[i].child = nullptr;
                                                 node->entries.erase(node->entries.begin() + i);
                                          }
                                   }
                                   return false;
                            }
                     }
                     return true;
              }
              else
              {
                     for (size_t i = 0; i < node->entries.size(); ++i)
                     {
                            if (node->entries[i].data == data)
                            {
                                   node->entries.erase(node->entries.begin() + i);
                                   return false;
                            }
                     }
                     return true;
              }
       }
       
       void ReinsertEntries(Node* node)
       {
              if (node->IsLeaf())
              {
                     for (auto& entry : node->entries)
                     {
                            Insert(entry.rect.xMin, entry.rect.yMin,
                                   entry.rect.xMax, entry.rect.yMax, entry.data);
                            --m_Count;  // Compensate Insert increment (entry already counted)
                     }
              }
              else
              {
                     for (auto& entry : node->entries)
                     {
                            ReinsertEntries(entry.child);
                            delete entry.child;
                     }
              }
       }
       
       void SearchRec(Node* node, const RectType& query, std::vector<QueryResult>& results) const
       {
              for (auto& entry : node->entries)
              {
                     if (query.Overlaps(entry.rect))
                     {
                            if (node->IsLeaf())
                                   results.push_back({entry.rect, entry.data});  
                            else
                                   SearchRec(entry.child, query, results);
                     }
              }
       }
       
       void WriteNode(std::ostream& os, Node* node) const
       {
              os << node->level << " " << node->entries.size() << "\n";
              for (auto& entry : node->entries)
              {
                     os << entry.rect.xMin << " " << entry.rect.yMin << " "
                        << entry.rect.xMax << " " << entry.rect.yMax << "\n";
                     if (node->IsLeaf())
                            os << entry.data << "\n";
                     else
                            WriteNode(os, entry.child);
              }
       }
       
       Node* ReadNode(std::istream& is)
       {
              Node* node = new Node();
              size_t n;
              is >> node->level >> n;
              
              for (size_t i = 0; i < n; ++i)
              {
                     Entry entry;
                     is >> entry.rect.xMin >> entry.rect.yMin
                        >> entry.rect.xMax >> entry.rect.yMax;
                     if (node->IsLeaf())
                            is >> entry.data;
                     else
                            entry.child = ReadNode(is);
                     node->entries.push_back(entry);
              }
              return node;
       }
       
       void PrintNode(std::ostream& os, Node* node, int depth) const
       {
              std::string indent(depth * 2, ' ');
              os << indent << (node->IsLeaf() ? "LEAF" : "INTERNAL")
                 << " [level=" << node->level << ", n=" << node->entries.size() << "]\n";
              
              for (size_t i = 0; i < node->entries.size(); ++i)
              {
                     auto& entry = node->entries[i];
                     os << indent << "  (" << entry.rect.xMin << "," << entry.rect.yMin
                        << ")-(" << entry.rect.xMax << "," << entry.rect.yMax << ")";
                     if (node->IsLeaf())
                            os << " data=" << entry.data;
                     os << "\n";
                     if (!node->IsLeaf())
                            PrintNode(os, entry.child, depth + 1);
              }
       }

       // Recursive variadic traversal implementation
       template<typename Func, typename... Args>
       void ForEachRec(Node* node, Func&& func, Args&&... args) const
       {
              if (!node) return;
              
              for (auto& entry : node->entries)
              {
                     if (node->IsLeaf())
                     {
                            // Apply function to leaf entries
                            func(entry.rect, entry.data, std::forward<Args>(args)...);
                     }
                     else
                     {
                            // Recurse into internal nodes
                            ForEachRec(entry.child, std::forward<Func>(func), std::forward<Args>(args)...);
                     }
              }
       }
};

#endif