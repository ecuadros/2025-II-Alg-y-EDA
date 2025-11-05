#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <sstream>
#include <cstdint>
#include "btreepage.h"
#include "btree_format.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

template <typename _keyType, typename _ObjIDType, typename _CompareFn = std::less<_keyType>>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       using CompareFn = _CompareFn;
};
template <typename Trait>
class BTree  
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;

       typedef typename Trait::CompareFn  CompareFn;
       typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
       typedef typename BTNode::ObjectInfo      ObjectInfo;
       class Iterator {
       public:
               using iterator_category = std::forward_iterator_tag;
               using value_type        = ObjectInfo;
               using difference_type   = std::ptrdiff_t;
               using pointer           = ObjectInfo*;
               using reference         = ObjectInfo&;
       private:
               std::vector<ObjectInfo*> m_Elements;  
               size_t                   m_Index;      
               friend class BTree<Trait>;
               friend class ConstIterator;
               static void collectElements(BTNode* node, std::vector<ObjectInfo*>& elements) {
                       if(!node || node->m_KeyCount == 0) return;
                       for(size_t i = 0; i < node->m_KeyCount; i++) {
                               if(node->m_SubPages[i])
                                       collectElements(node->m_SubPages[i], elements);
                               elements.push_back(&(node->m_Keys[i]));
                       }
                       if(node->m_SubPages[node->m_KeyCount])
                               collectElements(node->m_SubPages[node->m_KeyCount], elements);
               }
               explicit Iterator(BTNode* root, bool isEnd = false)
                       : m_Index(0)
               {
                       if(!isEnd && root) {
                               collectElements(root, m_Elements);
                               if(m_Elements.empty())
                                       m_Index = 0;  
                       }
               }
       public:
               Iterator() : m_Index(0) {}
               reference operator*() const {
                       assert(m_Index < m_Elements.size() && "Cannot dereference end iterator");
                       return *(m_Elements[m_Index]);
               }
               pointer operator->() const {
                       assert(m_Index < m_Elements.size() && "Cannot dereference end iterator");
                       return m_Elements[m_Index];
               }
               Iterator& operator++() {
                       if(m_Index < m_Elements.size())
                               ++m_Index;
                       return *this;
               }
               Iterator operator++(int) {
                       Iterator tmp = *this;
                       if(m_Index < m_Elements.size())
                               ++m_Index;
                       return tmp;
               }
               bool operator==(const Iterator& other) const {
                       if(m_Index >= m_Elements.size() && other.m_Index >= other.m_Elements.size())
                               return true;
                       if(m_Index >= m_Elements.size() || other.m_Index >= other.m_Elements.size())
                               return false;
                       return m_Elements[m_Index] == other.m_Elements[other.m_Index];
               }
               bool operator!=(const Iterator& other) const {
                       return !(*this == other);
               }
       };
       class ConstIterator {
       public:
               using iterator_category = std::forward_iterator_tag;
               using value_type        = const ObjectInfo;
               using difference_type   = std::ptrdiff_t;
               using pointer           = const ObjectInfo*;
               using reference         = const ObjectInfo&;
       private:
               std::vector<ObjectInfo*> m_Elements;  
               size_t                   m_Index;      
               friend class BTree<Trait>;
               static void collectElements(BTNode* node, std::vector<ObjectInfo*>& elements) {
                       if(!node || node->m_KeyCount == 0) return;
                       for(size_t i = 0; i < node->m_KeyCount; i++) {
                               if(node->m_SubPages[i])
                                       collectElements(node->m_SubPages[i], elements);
                               elements.push_back(&(node->m_Keys[i]));
                       }
                       if(node->m_SubPages[node->m_KeyCount])
                               collectElements(node->m_SubPages[node->m_KeyCount], elements);
               }
               explicit ConstIterator(const BTNode* root, bool isEnd = false)
                       : m_Index(0)
               {
                       if(!isEnd && root) {
                               collectElements(const_cast<BTNode*>(root), m_Elements);
                               if(m_Elements.empty())
                                       m_Index = 0;
                       }
               }
       public:
               ConstIterator() : m_Index(0) {}
               ConstIterator(const Iterator& it) : m_Elements(it.m_Elements), m_Index(it.m_Index) {}
               reference operator*() const {
                       assert(m_Index < m_Elements.size() && "Cannot dereference end iterator");
                       return *(m_Elements[m_Index]);
               }
               pointer operator->() const {
                       assert(m_Index < m_Elements.size() && "Cannot dereference end iterator");
                       return m_Elements[m_Index];
               }
               ConstIterator& operator++() {
                       if(m_Index < m_Elements.size())
                               ++m_Index;
                       return *this;
               }
               ConstIterator operator++(int) {
                       ConstIterator tmp = *this;
                       if(m_Index < m_Elements.size())
                               ++m_Index;
                       return tmp;
               }
               bool operator==(const ConstIterator& other) const {
                       if(m_Index >= m_Elements.size() && other.m_Index >= other.m_Elements.size())
                               return true;
                       if(m_Index >= m_Elements.size() || other.m_Index >= other.m_Elements.size())
                               return false;
                       return m_Elements[m_Index] == other.m_Elements[other.m_Index];
               }
               bool operator!=(const ConstIterator& other) const {
                       return !(*this == other);
               }
       };
       class ReverseIterator {
       public:
               using iterator_category = std::bidirectional_iterator_tag;
               using value_type        = ObjectInfo;
               using difference_type   = std::ptrdiff_t;
               using pointer           = ObjectInfo*;
               using reference         = ObjectInfo&;
       private:
               std::vector<ObjectInfo*> m_Elements;  
               std::ptrdiff_t           m_Index;     
               friend class BTree<Trait>;
               friend class ConstReverseIterator;
               static void collectElements(BTNode* node, std::vector<ObjectInfo*>& elements) {
                       if(!node || node->m_KeyCount == 0) return;
                       for(size_t i = 0; i < node->m_KeyCount; i++) {
                               if(node->m_SubPages[i])
                                       collectElements(node->m_SubPages[i], elements);
                               elements.push_back(&(node->m_Keys[i]));
                       }
                       if(node->m_SubPages[node->m_KeyCount])
                               collectElements(node->m_SubPages[node->m_KeyCount], elements);
               }
               explicit ReverseIterator(BTNode* root, bool isREnd = false)
                       : m_Index(0)
               {
                       if(root) {
                               collectElements(root, m_Elements);
                               if(isREnd) {
                                       m_Index = -1;  
                               } else {
                                       m_Index = m_Elements.empty() ? -1 : (std::ptrdiff_t)m_Elements.size() - 1;
                               }
                       } else {
                               m_Index = -1;
                       }
               }
       public:
               ReverseIterator() : m_Index(-1) {}
               reference operator*() const {
                       assert(m_Index >= 0 && m_Index < (std::ptrdiff_t)m_Elements.size() && "Cannot dereference rend iterator");
                       return *(m_Elements[m_Index]);
               }
               pointer operator->() const {
                       assert(m_Index >= 0 && m_Index < (std::ptrdiff_t)m_Elements.size() && "Cannot dereference rend iterator");
                       return m_Elements[m_Index];
               }
               ReverseIterator& operator++() {
                       if(m_Index >= 0)
                               --m_Index;
                       return *this;
               }
               ReverseIterator operator++(int) {
                       ReverseIterator tmp = *this;
                       if(m_Index >= 0)
                               --m_Index;
                       return tmp;
               }
               ReverseIterator& operator--() {
                       if(m_Index < (std::ptrdiff_t)m_Elements.size() - 1)
                               ++m_Index;
                       return *this;
               }
               ReverseIterator operator--(int) {
                       ReverseIterator tmp = *this;
                       if(m_Index < (std::ptrdiff_t)m_Elements.size() - 1)
                               ++m_Index;
                       return tmp;
               }
               bool operator==(const ReverseIterator& other) const {
                       if(m_Index < 0 && other.m_Index < 0)
                               return true;
                       if(m_Index < 0 || other.m_Index < 0)
                               return false;
                       if(m_Index >= (std::ptrdiff_t)m_Elements.size() ||
                          other.m_Index >= (std::ptrdiff_t)other.m_Elements.size())
                               return false;
                       return m_Elements[m_Index] == other.m_Elements[other.m_Index];
               }
               bool operator!=(const ReverseIterator& other) const {
                       return !(*this == other);
               }
       };
       class ConstReverseIterator {
       public:
               using iterator_category = std::bidirectional_iterator_tag;
               using value_type        = const ObjectInfo;
               using difference_type   = std::ptrdiff_t;
               using pointer           = const ObjectInfo*;
               using reference         = const ObjectInfo&;
       private:
               std::vector<ObjectInfo*> m_Elements;  
               std::ptrdiff_t           m_Index;     
               friend class BTree<Trait>;
               static void collectElements(BTNode* node, std::vector<ObjectInfo*>& elements) {
                       if(!node || node->m_KeyCount == 0) return;
                       for(size_t i = 0; i < node->m_KeyCount; i++) {
                               if(node->m_SubPages[i])
                                       collectElements(node->m_SubPages[i], elements);
                               elements.push_back(&(node->m_Keys[i]));
                       }
                       if(node->m_SubPages[node->m_KeyCount])
                               collectElements(node->m_SubPages[node->m_KeyCount], elements);
               }
               explicit ConstReverseIterator(const BTNode* root, bool isREnd = false)
                       : m_Index(0)
               {
                       if(root) {
                               collectElements(const_cast<BTNode*>(root), m_Elements);
                               if(isREnd) {
                                       m_Index = -1;
                               } else {
                                       m_Index = m_Elements.empty() ? -1 : (std::ptrdiff_t)m_Elements.size() - 1;
                               }
                       } else {
                               m_Index = -1;
                       }
               }
       public:
               ConstReverseIterator() : m_Index(-1) {}
               ConstReverseIterator(const ReverseIterator& it) : m_Elements(it.m_Elements), m_Index(it.m_Index) {}
               reference operator*() const {
                       assert(m_Index >= 0 && m_Index < (std::ptrdiff_t)m_Elements.size() && "Cannot dereference rend iterator");
                       return *(m_Elements[m_Index]);
               }
               pointer operator->() const {
                       assert(m_Index >= 0 && m_Index < (std::ptrdiff_t)m_Elements.size() && "Cannot dereference rend iterator");
                       return m_Elements[m_Index];
               }
               ConstReverseIterator& operator++() {
                       if(m_Index >= 0)
                               --m_Index;
                       return *this;
               }
               ConstReverseIterator operator++(int) {
                       ConstReverseIterator tmp = *this;
                       if(m_Index >= 0)
                               --m_Index;
                       return tmp;
               }
               ConstReverseIterator& operator--() {
                       if(m_Index < (std::ptrdiff_t)m_Elements.size() - 1)
                               ++m_Index;
                       return *this;
               }
               ConstReverseIterator operator--(int) {
                       ConstReverseIterator tmp = *this;
                       if(m_Index < (std::ptrdiff_t)m_Elements.size() - 1)
                               ++m_Index;
                       return tmp;
               }
               bool operator==(const ConstReverseIterator& other) const {
                       if(m_Index < 0 && other.m_Index < 0)
                               return true;
                       if(m_Index < 0 || other.m_Index < 0)
                               return false;
                       if(m_Index >= (std::ptrdiff_t)m_Elements.size() ||
                          other.m_Index >= (std::ptrdiff_t)other.m_Elements.size())
                               return false;
                       return m_Elements[m_Index] == other.m_Elements[other.m_Index];
               }
               bool operator!=(const ConstReverseIterator& other) const {
                       return !(*this == other);
               }
       };
       using iterator               = Iterator;
       using const_iterator         = ConstIterator;
       using reverse_iterator       = ReverseIterator;
       using const_reverse_iterator = ConstReverseIterator;
public:
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
              : m_Root(2 * order  + 1, unique, m_Compare),
                m_Height(1),
                m_Order(order),
                m_NumKeys(0),
                m_Unique(unique),
                m_Compare()
       {
              m_Root.SetMaxKeysForChilds(order);
       }
       ~BTree() {}
       BTree(BTree&& other) noexcept;
       BTree& operator=(BTree&& other) noexcept;
       BTree(const BTree& other) = delete;
       BTree& operator=(const BTree& other) = delete;
       bool            Write (const std::string& filename) const;
       bool            Read  (const std::string& filename);
       bool            Insert (const keyType key, const long ObjID);
       bool            Remove (const keyType key, const long ObjID);
       ObjIDType       Search (const keyType key) const
       {      ObjIDType ObjID = -1;
              m_Root.Search(key, ObjID);
              return ObjID;
       }
       size_t            size() const { return m_NumKeys; }
       size_t            height() const { return m_Height;      }
       size_t            GetOrder() const { return m_Order;     }
       void            Print (ostream &os)
       {               m_Root.Print(os);                              }
       iterator begin() {
               return Iterator(&m_Root, false);
       }
       iterator end() {
               return Iterator(&m_Root, true);
       }
       const_iterator begin() const {
               return ConstIterator(&m_Root, false);
       }
       const_iterator end() const {
               return ConstIterator(&m_Root, true);
       }
       const_iterator cbegin() const {
               return ConstIterator(&m_Root, false);
       }
       const_iterator cend() const {
               return ConstIterator(&m_Root, true);
       }
       reverse_iterator rbegin() {
               return ReverseIterator(&m_Root, false);
       }
       reverse_iterator rend() {
               return ReverseIterator(&m_Root, true);
       }
       const_reverse_iterator rbegin() const {
               return ConstReverseIterator(&m_Root, false);
       }
       const_reverse_iterator rend() const {
               return ConstReverseIterator(&m_Root, true);
       }
       const_reverse_iterator crbegin() const {
               return ConstReverseIterator(&m_Root, false);
       }
       const_reverse_iterator crend() const {
               return ConstReverseIterator(&m_Root, true);
       }
       template <typename Function, typename... Args>
       void ForEach(Function&& func, Args&&... args) const
       {               m_Root.ForEach(std::forward<Function>(func), 0, std::forward<Args>(args)...);    }
       template <typename Predicate, typename... Args>
       ObjectInfo* FirstThat(Predicate&& pred, Args&&... args)
       {               return m_Root.FirstThat(std::forward<Predicate>(pred), 0, std::forward<Args>(args)...);   }
       template <typename Predicate, typename... Args>
       void FindAll(vector<ObjectInfo*>& results, Predicate&& pred, Args&&... args)
       {               m_Root.FindAll(results, std::forward<Predicate>(pred), 0, std::forward<Args>(args)...);   }
       template <typename Predicate, typename... Args>
       size_t CountIf(Predicate&& pred, Args&&... args) const
       {               return m_Root.CountIf(std::forward<Predicate>(pred), 0, std::forward<Args>(args)...);     }
       template <typename Predicate, typename... Args>
       bool AnyOf(Predicate&& pred, Args&&... args) const
       {               return m_Root.AnyOf(std::forward<Predicate>(pred), 0, std::forward<Args>(args)...);       }
       template <typename Predicate, typename... Args>
       bool AllOf(Predicate&& pred, Args&&... args) const
       {               return m_Root.AllOf(std::forward<Predicate>(pred), 0, std::forward<Args>(args)...);       }
       template <typename Predicate, typename... Args>
       bool NoneOf(Predicate&& pred, Args&&... args) const
       {               return m_Root.NoneOf(std::forward<Predicate>(pred), 0, std::forward<Args>(args)...);      }
       template <typename T, typename BinaryOp, typename... Args>
       T Accumulate(T init, BinaryOp&& op, Args&&... args) const
       {               return m_Root.Accumulate(init, std::forward<BinaryOp>(op), 0, std::forward<Args>(args)...); }
       template <typename OutputContainer, typename UnaryOp, typename... Args>
       void Transform(OutputContainer& output, UnaryOp&& op, Args&&... args) const
       {               m_Root.Transform(output, std::forward<UnaryOp>(op), 0, std::forward<Args>(args)...);      }
protected:
       BTNode          m_Root;
       size_t          m_Height;   
       size_t          m_Order;    
       size_t          m_NumKeys;  
       bool            m_Unique;   
       CompareFn       m_Compare;  
};

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const long ObjID){
       bt_ErrorCode error = m_Root.Insert(key, ObjID);
       if( error == bt_duplicate )
               return false;
       m_NumKeys++;
       if( error == bt_overflow ){
               m_Root.SplitRoot();
               m_Height++;
       }
       return true;
}

template <typename Trait>
bool BTree<Trait>::Remove (const keyType key, const long ObjID)
{
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if( error == bt_duplicate || error == bt_nofound )
               return false;
       m_NumKeys--;

       if( error == bt_rootmerged )
               m_Height--;
       return true;
}

template <typename Trait>
BTree<Trait>::BTree(BTree&& other) noexcept
       : m_Root(std::move(other.m_Root)),
         m_Height(other.m_Height),
         m_Order(other.m_Order),
         m_NumKeys(other.m_NumKeys),
         m_Unique(other.m_Unique),
         m_Compare(std::move(other.m_Compare))
{
       other.m_Height = 1;
       other.m_Order = DEFAULT_BTREE_ORDER;
       other.m_NumKeys = 0;
       other.m_Unique = true;
}
template <typename Trait>
BTree<Trait>& BTree<Trait>::operator=(BTree&& other) noexcept
{
       if(this != &other) {
               m_Root = std::move(other.m_Root);
               m_Height = other.m_Height;
               m_Order = other.m_Order;
               m_NumKeys = other.m_NumKeys;
               m_Unique = other.m_Unique;
               m_Compare = std::move(other.m_Compare);
               other.m_Height = 1;
               other.m_Order = DEFAULT_BTREE_ORDER;
               other.m_NumKeys = 0;
               other.m_Unique = true;
       }
       return *this;
}
template <typename Trait>
bool BTree<Trait>::Write(const std::string& filename) const
{
       try {
               std::ofstream out(filename, std::ios::binary);
               if(!out.is_open()) {
                       std::cerr << "Error: Cannot open file for writing: " << filename << std::endl;
                       return false;
               }
               const char magic[6] = "BTREE";
               out.write(magic, 5);
               const uint32_t version = 1;
               out.write(reinterpret_cast<const char*>(&version), sizeof(version));
               out.write(reinterpret_cast<const char*>(&m_Height), sizeof(m_Height));
               out.write(reinterpret_cast<const char*>(&m_Order), sizeof(m_Order));
               out.write(reinterpret_cast<const char*>(&m_NumKeys), sizeof(m_NumKeys));
               out.write(reinterpret_cast<const char*>(&m_Unique), sizeof(m_Unique));
               m_Root.WriteToDisk(out);
               out.close();
               return true;
       } catch(const std::exception& e) {
               std::cerr << "Exception during Write: " << e.what() << std::endl;
               return false;
       } catch(...) {
               std::cerr << "Unknown exception during Write" << std::endl;
               return false;
       }
}
template <typename Trait>
bool BTree<Trait>::Read(const std::string& filename)
{
       try {
               std::ifstream in(filename, std::ios::binary);
               if(!in.is_open()) {
                       std::cerr << "Error: Cannot open file for reading: " << filename << std::endl;
                       return false;
               }
               char magic[6] = {0};
               in.read(magic, 5);
               if(std::string(magic) != "BTREE") {
                       std::cerr << "Error: Invalid file format (magic number mismatch)" << std::endl;
                       in.close();
                       return false;
               }
               uint32_t version = 0;
               in.read(reinterpret_cast<char*>(&version), sizeof(version));
               if(version != 1) {
                       std::cerr << "Warning: File version " << version << " may not be compatible" << std::endl;
               }
               size_t fileHeight, fileOrder, fileNumKeys;
               bool fileUnique;
               in.read(reinterpret_cast<char*>(&fileHeight), sizeof(fileHeight));
               in.read(reinterpret_cast<char*>(&fileOrder), sizeof(fileOrder));
               in.read(reinterpret_cast<char*>(&fileNumKeys), sizeof(fileNumKeys));
               in.read(reinterpret_cast<char*>(&fileUnique), sizeof(fileUnique));
               if(fileOrder != m_Order) {
                       std::cerr << "Warning: File order (" << fileOrder << ") differs from current order ("
                                 << m_Order << "). Adjusting..." << std::endl;
                       m_Order = fileOrder;
               }
               m_Root.ReadFromDisk(in);
               m_Height = fileHeight;
               m_NumKeys = fileNumKeys;
               m_Unique = fileUnique;
               in.close();
               return true;
       } catch(const std::exception& e) {
               std::cerr << "Exception during Read: " << e.what() << std::endl;
               return false;
       } catch(...) {
               std::cerr << "Unknown exception during Read" << std::endl;
               return false;
       }
}
template <typename Trait>
std::ostream& operator<<(std::ostream& os, const BTree<Trait>& bt)
{
       using FormatStyle = btree_format::FormatStyle;
       FormatStyle style = btree_format::getFormatStyle(os);
       switch(style) {
       case FormatStyle::LINEAR:
               os << "[";
               {
                       bool first = true;
                       bt.ForEach([&os, &first](auto& info, size_t level) {
                               if(!first) os << ", ";
                               os << info.key;
                               first = false;
                       });
               }
               os << "]";
               break;
       case FormatStyle::COMPACT:
               bt.ForEach([&os](auto& info, size_t level) {
                       os << info.key << " ";
               });
               break;
       case FormatStyle::DETAILED:
               bt.ForEach([&os](auto& info, size_t level) {
                       os << "key=" << info.key << "(ObjID=" << info.ObjID << ") ";
               });
               break;
       case FormatStyle::TREE:
               os << "BTree(order=" << bt.GetOrder() << ", size=" << bt.size()
                  << ", height=" << bt.height() << ")\n";
               bt.ForEach([&os](auto& info, size_t level) {
                       for(size_t i = 0; i < level; i++)
                               os << "  ";
                       os << "├─ " << info.key << " (ObjID=" << info.ObjID << ")\n";
               });
               break;
       case FormatStyle::JSON_LIKE:
               os << "{\n";
               os << "  \"order\": " << bt.GetOrder() << ",\n";
               os << "  \"size\": " << bt.size() << ",\n";
               os << "  \"height\": " << bt.height() << ",\n";
               os << "  \"keys\": [";
               {
                       bool first = true;
                       bt.ForEach([&os, &first](auto& info, size_t level) {
                               if(!first) os << ", ";
                               os << info.key;
                               first = false;
                       });
               }
               os << "],\n";
               os << "  \"entries\": [\n";
               {
                       bool first = true;
                       bt.ForEach([&os, &first](auto& info, size_t level) {
                               if(!first) os << ",\n";
                               os << "    {\"key\": " << info.key
                                  << ", \"ObjID\": " << info.ObjID
                                  << ", \"level\": " << level << "}";
                               first = false;
                       });
               }
               os << "\n  ]\n";
               os << "}";
               break;
       case FormatStyle::VERTICAL:
               os << "BTree (order=" << bt.GetOrder() << ", size=" << bt.size() << ")\n";
               os << "─────────────────────────────────\n";
               bt.ForEach([&os](auto& info, size_t level) {
                       os << "[L" << level << "] " << info.key
                          << " -> ObjID(" << info.ObjID << ")\n";
               });
               os << "─────────────────────────────────";
               break;
       default:
               bt.ForEach([&os](auto& info, size_t level) {
                       os << info.key << " ";
               });
               break;
       }
       return os;
}
#endif