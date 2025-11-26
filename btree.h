#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <fstream>
#include <sstream>
#include <string>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

template <typename _keyType, typename _ObjIDType, typename _CompareFn = std::less<_keyType>>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       using CompareFn = _CompareFn;// TODO: agregar funcion de comparacion
};

template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;
       typedef typename Trait::CompareFn    CompareFn;
       
       typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
       //typedef ObjectInfo iterator;
       //typedef typename BTNode::lpfnForEach2    lpfnForEach2;
       //typedef typename BTNode::lpfnForEach3    lpfnForEach3;
       //typedef typename BTNode::lpfnFirstThat2  lpfnFirstThat2;
       //typedef typename BTNode::lpfnFirstThat3  lpfnFirstThat3;
       typedef typename BTNode::ObjectInfo      ObjectInfo;

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
       ~BTree() {}
       // Move constructor
       BTree(BTree&& other) {
              std::lock_guard<std::shared_mutex> lock (other.m_Mutex);
              m_Order = std::move(other.m_Order);
              m_Root = std::move(other.m_Root);
              m_Unique = std::move(other.m_Unique);
              m_NumKeys = std::move(other.m_NumKeys);
              m_Height = std::move(other.m_Height);
       }
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();
       bool            Insert (const keyType key, const long ObjID);
       bool            Remove (const keyType key, const long ObjID);
       ObjIDType       Search (const keyType key)
       {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              ObjIDType ObjID = -1;
              m_Root.Search(key, ObjID);
              return ObjID;
       }
       size_t            size()  { std::shared_lock<std::shared_mutex> lock(m_Mutex); return m_NumKeys; }
       size_t            height() { std::shared_lock<std::shared_mutex> lock(m_Mutex); return m_Height;      }
       size_t            GetOrder() { std::shared_lock<std::shared_mutex> lock(m_Mutex); return m_Order;     }

       void            Print (ostream &os)
       {               std::shared_lock<std::shared_mutex> lock(m_Mutex); m_Root.Print(os);                              }
       /*void            ForEach( lpfnForEach2 lpfn, void *pExtra1 )
       {               m_Root.ForEach(lpfn, 0, pExtra1);              }
       void            ForEach( lpfnForEach3 lpfn, void *pExtra1, void *pExtra2)
       {               m_Root.ForEach(lpfn, 0, pExtra1, pExtra2);     }*/
       template <typename Func, typename... Args>
       void ForEach(Func&& func, Args&&... args) {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              m_Root.ForEach(std::forward<Func>(func), 0, std::forward<Args>(args)...);
       }
       /*ObjectInfo*     FirstThat( lpfnFirstThat2 lpfn, void *pExtra1 )
       {               return m_Root.FirstThat(lpfn, 0, pExtra1);     }
       ObjectInfo*     FirstThat( lpfnFirstThat3 lpfn, void *pExtra1, void *pExtra2)
       {               return m_Root.FirstThat(lpfn, 0, pExtra1, pExtra2);   }*/
       template <typename Pred, typename... Args>
       typename BTNode::ObjectInfo* FirstThat(Pred&& pred, Args&&... args) {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              return m_Root.FirstThat(std::forward<Pred>(pred), 0, std::forward<Args>(args)...);
       }
       //typedef               ObjectInfo iterator;

       // Forward iterator
       using iterator = BTreeForwardIterator<Trait>;
       iterator begin()
       {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              if (m_NumKeys == 0) {
                     return iterator();
              }
              return iterator(&m_Root);
       }
       iterator end()
       {
              return iterator();
       }
       // Backward iterator
       using reverse_iterator = BTreeBackwardIterator<Trait>;
       reverse_iterator rbegin()
       {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              if (m_NumKeys == 0) {
                     return reverse_iterator();
              }
              return reverse_iterator(&m_Root);
       }
       reverse_iterator rend()
       {
              return reverse_iterator();
       }

       template <typename T>
       friend std::ostream& operator<<(std::ostream& os, BTree<T>& obj);

       // Write
       void Write(std::ostream& os)
       {
              os << *this;
       }

       void Write(const std::string& filename)
       {
              std::ofstream file(filename);
              if (file.is_open()) {
                     Write(file);
                     file.close();
              }
       }

       // Read
       void Read(std::istream& is)
       {
              std::lock_guard<std::shared_mutex> lock(m_Mutex);
              /*keyType key;
              ObjIDType objId;
              while (is >> key >> objId) {
                     Insert(key, objId);
              }*/
              std::string line;
              while (std::getline(is, line)) {
                     if (line.empty()) continue;
                     size_t start = 0;
                     while (start < line.size() && line[start] == '\t') {
                            start++;
                     }

                     size_t arrowPos = line.find("->", start);
                     if (arrowPos == std::string::npos) continue;
                     std::string keyStr = line.substr(start, arrowPos - start);
                     std::string objIdStr = line.substr(arrowPos + 2);
                     std::istringstream keyStream(keyStr);
                     std::istringstream objIdStream(objIdStr);

                     keyType key;
                     ObjIDType objId;
                     if (keyStream >> key && objIdStream >> objId) {
                            //Insert(key, objId);
                            bt_ErrorCode error = m_Root.Insert(key, objId);
                            if (error != bt_duplicate) {
                                   m_NumKeys++;
                                   if (error == bt_overflow) {
                                          m_Root.SplitRoot();
                                          m_Height++;
                                   }
                            }
                     }
              }
       }

       void Read(const std::string& filename)
       {
              std::ifstream file(filename);
              if (file.is_open()) {
                     Read(file);
                     file.close();
              }
       }
       
protected:
       size_t          m_Order;   // order of tree
       BTNode          m_Root;
       bool            m_Unique;  // Accept the elements only once ?
       size_t          m_NumKeys; // number of keys
       size_t          m_Height;  // height of tree
       std::shared_mutex m_Mutex;
};     

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const long ObjID){
       std::lock_guard<std::shared_mutex> lock(m_Mutex);
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
       std::lock_guard<std::shared_mutex> lock(m_Mutex);
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if( error == bt_duplicate || error == bt_nofound )
               return false;
       m_NumKeys--;

       if( error == bt_rootmerged )
               m_Height--;
       return true;
}

template <typename Trait>
std::ostream& operator<<(std::ostream& os, BTree<Trait>& obj)
{
       std::shared_lock<std::shared_mutex> lock(obj.m_Mutex);
       obj.m_Root.Print(os);
       return os;
}


#endif