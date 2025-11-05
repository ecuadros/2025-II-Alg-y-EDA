#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <utility>
#include <fstream>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

template <typename _keyType, typename _ObjIDType, typename _Compare = std::less<_keyType>>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       // TODO: agregar funcion de comparacion (DONE)
       using Compare = _Compare;
};

template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;
       
       typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
       typedef typename BTNode::ObjectInfo      ObjectInfo;
public:
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
              : m_Root(2 * order  + 1, unique),
                m_Order(order),
                m_NumKeys(0),
                m_Unique(unique)
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Height = 1;
       }
       BTree(BTree&& other) noexcept
              : m_Root(std::move(other.m_Root)),
                m_Height(std::exchange(other.m_Height, 0)),
                m_Order(std::exchange(other.m_Order, 0)),
                m_NumKeys(std::exchange(other.m_NumKeys, 0)),
                m_Unique(std::exchange(other.m_Unique, false))
       {
              
       }
       ~BTree() {}
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();
       void            Write(ostream& os);
       void            Read(istream& is);

       bool            Insert (const keyType key, const ObjIDType ObjID);
       bool            Remove (const keyType key, const ObjIDType ObjID);
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

       template<typename Func, typename... Args>
       void ForEach(size_t level, Func&& func, Args&&... args) {
              m_Root.ForEach(level, std::forward<Func>(func), std::forward<Args>(args)...);
       }

       template<typename Func, typename... Args>
       ObjectInfo* FirstThat(size_t level, Func&& func, Args&&... args) {
              return m_Root.FirstThat(level, std::forward<Func>(func), std::forward<Args>(args)...);
       }
protected:
       BTNode          m_Root;
       size_t          m_Height;  // height of tree
       size_t          m_Order;   // order of tree
       size_t          m_NumKeys; // number of keys
       bool            m_Unique;  // Accept the elements only once ?
};     

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const ObjIDType ObjID){
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
void BTree<Trait>::Write(ostream& os) {
    os.write(reinterpret_cast<const char*>(&m_Order), sizeof(m_Order));
    os.write(reinterpret_cast<const char*>(&m_Unique), sizeof(m_Unique));
    os.write(reinterpret_cast<const char*>(&m_NumKeys), sizeof(m_NumKeys));
    os.write(reinterpret_cast<const char*>(&m_Height), sizeof(m_Height));
    m_Root.Write(os);
}

template <typename Trait>
void BTree<Trait>::Read(istream& is) {
    is.read(reinterpret_cast<char*>(&m_Order), sizeof(m_Order));
    is.read(reinterpret_cast<char*>(&m_Unique), sizeof(m_Unique));
    is.read(reinterpret_cast<char*>(&m_NumKeys), sizeof(m_NumKeys));
    is.read(reinterpret_cast<char*>(&m_Height), sizeof(m_Height));
    m_Root = BTNode(2 * m_Order + 1, m_Unique);
    m_Root.Read(is);
}

template <typename Trait>
bool BTree<Trait>::Remove (const keyType key, const ObjIDType ObjID)
{
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if( error == bt_duplicate || error == bt_nofound )
               return false;
       m_NumKeys--;

       if( error == bt_rootmerged )
               m_Height--;
       return true;
}

#endif