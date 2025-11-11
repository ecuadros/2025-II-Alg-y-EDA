#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <utility>
#include <functional>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

template <typename _keyType, typename _ObjIDType, typename _CompareF>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;     
       using CompareF = _CompareF;
};

template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;
       
       typedef CBTreePage <Trait> BTNode;// useful shorthand

       // File header structure
       struct FileHeader {
           size_t numKeys;      
           size_t height;       

           bool Write(std::ostream& os) const {
               os.write(reinterpret_cast<const char*>(this), sizeof(FileHeader));
               return os.good();
           }

           bool Read(std::istream& is) {
               is.read(reinterpret_cast<char*>(this), sizeof(FileHeader));
               return is.good();
           }
       };

public:
       //typedef ObjectInfo iterator;
       typedef typename BTNode::ObjectInfo      ObjectInfo;
       friend std::ostream& operator<<(std::ostream& os, const Btree<Trait>& tree);

public:
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
              : m_Order(order),
                m_Root(2 * order  + 1, unique),
                m_Unique(unique),
                m_NumKeys(0)
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Height = 1;
       }
       // Move contructor
       BTree(Btree&& btree){
       // Transfering resources
              m_Order = btree.m_Order;
              m_Root = std::move(btree.m_Root);
              m_Height = btree.m_Height;
              m_Unique = btree.m_Unique,
              m_NumKeys = btree.m_NumKeys;

       // Leaving btree in "valid" state
              btree.m_Height = 1;
              btree.m_numKeys = 0;

       }
       // Move assignment: transfers ownership to existing object
       BTree& operator=(BTree&& btree  ) noexcept 
       {
              if (this != &btree  )
              {
                     m_Order = btree  .m_Order;
                     m_Root = std::move(btree  .m_Root);
                     m_Height = btree  .m_Height;
                     m_Unique = btree  .m_Unique;
                     m_NumKeys = btree  .m_NumKeys;

                     btree  .m_Height = 1;  // Leave source in valid state
                     btree  .m_NumKeys = 0;
              }
              return *this;
       }
       ~BTree() {}
       // Write tree to file
       bool Write(const std::string& filename) {
           std::ofstream file(filename, std::ios::binary);
           if (!file) return false;

           // Write Header
           FileHeader header{m_NumKeys, m_Height};
           if (!header.Write(file)) return false;

           // Write Tree recursively
           return WriteNode(file, &m_Root);
       }

       // Read tree from file
       bool Read(const std::string& filename) {
           std::ifstream file(filename, std::ios::binary);
           if (!file) return false;

           // read header
           FileHeader header;
           if (!header.Read(file)) return false;

           // Update numKeys y height
           m_NumKeys = header.numKeys;
           m_Height = header.height;

           // Read Tree recursively (m_Root ya está inicializada por el constructor)
           return ReadNode(file, &m_Root);
       }
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();
       bool            Insert (const keyType key, const long ObjID);
       bool            Remove   (const keyType key, const long ObjID);
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
       // Generalized ForEach 
       template <typename Func, typename... Args>
       void            ForEach( Func&& func, Args&&... args )
       {               m_Root.ForEach(std::forward<Func>(func), 0, std::forward<Args>(args)...);              }

       // Generalized FirstThat
       template <typename Func, typename... Args>
       ObjectInfo*     FirstThat( Func&& func, Args&&... args )
       {               return m_Root.FirstThat(std::forward<Func>(func), 0, std::forward<Args>(args)...);     }
      
       //typedef               ObjectInfo iterator;
private:
       
       bool WriteNode(std::ostream& os, BTNode* node) {
           if (!node) return true;

           // Write  number of keys
           size_t count = node->GetNumberOfKeys();
           os.write(reinterpret_cast<const char*>(&count), sizeof(count));
           if (!os.good()) return false;

           // write keys
           for (size_t i = 0; i < count; i++) {
               if (!node->m_Keys[i].Write(os)) return false;
           }

           // Write childs recursively
           for (size_t i = 0; i <= count; i++) {
               bool hasChild = node->m_SubPages[i] != nullptr;
               os.write(reinterpret_cast<const char*>(&hasChild), sizeof(hasChild));
               if (hasChild && !WriteNode(os, node->m_SubPages[i])) {
                   return false;
               }
           }

           return true;
       }


       bool ReadNode(std::istream& is, BTNode* node) {
           if (!node) return false;

           // Read number of keys
           size_t count;
           is.read(reinterpret_cast<char*>(&count), sizeof(count));
           if (!is.good()) return false;

           // Read keys
           for (size_t i = 0; i < count; i++) {
               ObjectInfo info;
               if (!info.Read(is)) return false;
               node->m_Keys[i] = std::move(info);
           }
           node->m_KeyCount = count;

           // Read childs recursively
           for (size_t i = 0; i <= count; i++) {
               bool hasChild;
               is.read(reinterpret_cast<char*>(&hasChild), sizeof(hasChild));
               if (hasChild) {
                   node->m_SubPages[i] = new BTNode(node->m_MaxKeysForChilds, node->m_Unique);
                   if (!ReadNode(is, node->m_SubPages[i])) {
                       return false;
                   }
               }
           }

           return true;
       }

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

template <typename Trait>
std::ostream& operator<<(std::ostream& os, const BTree<Trait>& tree) {
    os << "BTree: order=" << tree.m_Order << ", height=" << tree.m_Height 
       << ", keys=" << tree.m_NumKeys << "\n";
    tree.m_Root.Print(os);  
    return os;
}

#endif