#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <fstream>
#include <shared_mutex>
#include "btreepage.h"
#include "btree_iterator.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

template <typename _keyType, typename _ObjIDType, typename _Compare = std::less<_keyType>>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       using Compare = _Compare;
};

template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;
       
       typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
       // Iterator 
       typedef BTreeIterator<Trait> iterator;
       typedef std::reverse_iterator<iterator> reverse_iterator;

       typedef typename BTNode::lpfnForEach2    lpfnForEach2;
       typedef typename BTNode::lpfnForEach3    lpfnForEach3;
       typedef typename BTNode::lpfnFirstThat2  lpfnFirstThat2;
       typedef typename BTNode::lpfnFirstThat3  lpfnFirstThat3;
       typedef typename BTNode::ObjectInfo      ObjectInfo;

public:
       // Constructor
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
              : m_Order(order),
                m_Root(2 * order  + 1, unique),
                m_Unique(unique),
                m_NumKeys(0)
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Height = 1;
       }

       // Destructor
       ~BTree() {}
       BTree(const BTree&) = delete;
       BTree& operator=(const BTree&) = delete;

       // Move Constructor
       BTree(BTree&& other) noexcept
              : m_Root(std::move(other.m_Root)),
                m_Height(other.m_Height),
                m_Order(other.m_Order),
                m_NumKeys(other.m_NumKeys),
                m_Unique(other.m_Unique)
       {
              other.m_Height = 1;
              other.m_NumKeys = 0;
       }

       BTree& operator=(BTree&& other) noexcept
       {
              if (this != &other) {
                     m_Root = std::move(other.m_Root);
                     m_Height = other.m_Height;
                     m_Order = other.m_Order;
                     m_NumKeys = other.m_NumKeys;
                     m_Unique = other.m_Unique;
                     other.m_Height = 1;
                     other.m_NumKeys = 0;
              }
              return *this;
       }
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();
       bool            Write (const char* filename);
       bool            Read (const char* filename);
       bool            Insert (const keyType key, const long ObjID);
       bool            Remove (const keyType key, const long ObjID);
       ObjIDType       Search (const keyType key)
       {      ObjIDType ObjID = -1;
              m_Root.Search(key, ObjID);
              return ObjID;
       }
       size_t            size()  { return m_NumKeys; }
       size_t            height() { return m_Height;      }
       size_t            GetOrder() { return m_Order;     }

       void            Print (ostream &os)
       {               m_Root.Print(os);                              }

       // foreach antiguo
       void            ForEach( lpfnForEach2 lpfn, void *pExtra1 )
       {               m_Root.ForEach(lpfn, 0, pExtra1);              }
       void            ForEach( lpfnForEach3 lpfn, void *pExtra1, void *pExtra2)
       {               m_Root.ForEach(lpfn, 0, pExtra1, pExtra2);     }
       // firsthat antiguo
       ObjectInfo*     FirstThat( lpfnFirstThat2 lpfn, void *pExtra1 )
       {               return m_Root.FirstThat(lpfn, 0, pExtra1);     }
       ObjectInfo*     FirstThat( lpfnFirstThat3 lpfn, void *pExtra1, void *pExtra2)
       {               return m_Root.FirstThat(lpfn, 0, pExtra1, pExtra2);   }

       // foreach
       template<typename Func, typename... Args>
       void ForEach(Func&& func, Args&&... args)
       {               m_Root.ForEach(std::forward<Func>(func), std::forward<Args>(args)...);  }
       // firstthat
       template<typename Pred, typename... Args>
       ObjectInfo* FirstThat(Pred&& pred, Args&&... args)
       {               return m_Root.FirstThat(std::forward<Pred>(pred), std::forward<Args>(args)...);   }

       iterator begin() { return iterator(&m_Root); }
       iterator end() { return iterator::end(); }
       reverse_iterator rbegin() { return reverse_iterator(end()); }
       reverse_iterator rend() { return reverse_iterator(begin()); }

       //operator<<
       template<typename T>
       friend std::ostream& operator<<(std::ostream& os, BTree<T>& bt);

protected:
       BTNode          m_Root;
       size_t          m_Height;  // height of tree
       size_t          m_Order;   // order of tree
       size_t          m_NumKeys; // number of keys
       bool            m_Unique;  // Accept the elements only once ?
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

// operator<<
template <typename Trait>
std::ostream& operator<<(std::ostream& os, BTree<Trait>& bt)
{
       os << "BTree[Order=" << bt.m_Order
          << ", Size=" << bt.m_NumKeys
          << ", Height=" << bt.m_Height
          << ", Unique=" << (bt.m_Unique ? "true" : "false")
          << "]:\n";
       bt.Print(os);
       return os;
}

// Write
template <typename Trait>
bool BTree<Trait>::Write(const char* filename)
{
       std::ofstream ofs(filename, std::ios::binary);
       if (!ofs.is_open()) {
              std::cerr << "Error: Cannot open file '" << filename << "' for writing" << std::endl;
              return false;
       }

       const char magic[6] = "BTREE";
       ofs.write(magic, 5);
       ofs.write(reinterpret_cast<const char*>(&m_Order), sizeof(m_Order));
       ofs.write(reinterpret_cast<const char*>(&m_Height), sizeof(m_Height));
       ofs.write(reinterpret_cast<const char*>(&m_NumKeys), sizeof(m_NumKeys));
       ofs.write(reinterpret_cast<const char*>(&m_Unique), sizeof(m_Unique));
       bool success = m_Root.WritePage(ofs);

       ofs.close();

       if (!success) {
              std::cerr << "Error: Failed to write BTree structure to file" << std::endl;
              return false;
       }

       return true;
}

// Read 
template <typename Trait>
bool BTree<Trait>::Read(const char* filename)
{
       std::ifstream ifs(filename, std::ios::binary);
       if (!ifs.is_open()) {
              std::cerr << "Error: Cannot open file '" << filename << "' for reading" << std::endl;
              return false;
       }

       char magic[6] = {0};
       ifs.read(magic, 5);
       if (std::string(magic) != "BTREE") {
              std::cerr << "Error: Invalid file format (magic number mismatch)" << std::endl;
              ifs.close();
              return false;
       }

       size_t fileOrder, fileHeight, fileNumKeys;
       bool fileUnique;

       ifs.read(reinterpret_cast<char*>(&fileOrder), sizeof(fileOrder));
       ifs.read(reinterpret_cast<char*>(&fileHeight), sizeof(fileHeight));
       ifs.read(reinterpret_cast<char*>(&fileNumKeys), sizeof(fileNumKeys));
       ifs.read(reinterpret_cast<char*>(&fileUnique), sizeof(fileUnique));

       if (fileOrder != m_Order) {
              std::cerr << "Error: BTree order mismatch (file=" << fileOrder
                        << ", current=" << m_Order << ")" << std::endl;
              ifs.close();
              return false;
       }

       if (fileUnique != m_Unique) {
              std::cerr << "Warning: Unique flag mismatch (file=" << fileUnique
                        << ", current=" << m_Unique << ")" << std::endl;
       }

       m_Root.Reset();
       m_Root.Create();
       m_Height = 1;
       m_NumKeys = 0;
       bool success = m_Root.ReadPage(ifs);
       ifs.close();

       if (!success) {
              std::cerr << "Error: Failed to read BTree structure from file" << std::endl;
              return false;
       }
       m_Height = fileHeight;
       m_NumKeys = fileNumKeys;

       return true;
}

#endif