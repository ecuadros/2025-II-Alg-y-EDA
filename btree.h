#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include "btreepage.h"
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
       //typedef ObjectInfo iterator;
       typedef typename BTNode::lpfnForEach2    lpfnForEach2;
       typedef typename BTNode::lpfnForEach3    lpfnForEach3;
       typedef typename BTNode::lpfnFirstThat2  lpfnFirstThat2;
       typedef typename BTNode::lpfnFirstThat3  lpfnFirstThat3;
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
       
       // Move constructor: transfiere recursos sin copiar
       BTree(BTree&& other) noexcept
              : m_Order(other.m_Order),
                m_Root(std::move(other.m_Root)),
                m_Height(other.m_Height),
                m_NumKeys(other.m_NumKeys),
                m_Unique(other.m_Unique)
       {
              // Dejar other en estado válido
              other.m_Height = 1;
              other.m_NumKeys = 0;
       }
       
       // Move assignment operator 
       BTree& operator=(BTree&& other) noexcept {
              if (this != &other) {
                     // Transferir datos de other
                     m_Root = std::move(other.m_Root);
                     m_Order = other.m_Order;
                     m_Height = other.m_Height;
                     m_NumKeys = other.m_NumKeys;
                     m_Unique = other.m_Unique;
                     
                     // Dejar other en estado válido
                     other.m_Height = 1;
                     other.m_NumKeys = 0;
              }
              return *this;
       }
       
       ~BTree() {}
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();
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
       
       // Write: guarda el árbol en formato texto
       void Write(std::ostream& os) {
              os << m_Order << " " << m_Height << " " 
                 << m_NumKeys << " " << m_Unique << "\n";
              m_Root.Write(os);
       }
       
       // Read: carga el árbol desde formato texto
       void Read(std::istream& is) {
              is >> m_Order >> m_Height >> m_NumKeys >> m_Unique;
              m_Root.Read(is);
       }
       
       // ForEach y FirstThat generalizados con variadic templates
       template<typename Func, typename... Args>
       void ForEach(Func func, Args&&... args) { 
              m_Root.ForEach(func, std::forward<Args>(args)...); 
       }

       template<typename Pred, typename... Args>
       ObjectInfo* FirstThat(Pred predicate, Args&&... args) { 
              return m_Root.FirstThat(predicate, std::forward<Args>(args)...); 
       }
       
       
       void            ForEach( lpfnForEach2 lpfn, void *pExtra1 )
       {               m_Root.ForEach(lpfn, 0, pExtra1);              }
       void            ForEach( lpfnForEach3 lpfn, void *pExtra1, void *pExtra2)
       {               m_Root.ForEach(lpfn, 0, pExtra1, pExtra2);     }
       ObjectInfo*     FirstThat( lpfnFirstThat2 lpfn, void *pExtra1 )
       {               return m_Root.FirstThat(lpfn, 0, pExtra1);     }
       ObjectInfo*     FirstThat( lpfnFirstThat3 lpfn, void *pExtra1, void *pExtra2)
       {               return m_Root.FirstThat(lpfn, 0, pExtra1, pExtra2);   }
       //typedef               ObjectInfo iterator;

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

// Operador << para imprimir el árbol
template <typename Trait>
std::ostream& operator<<(std::ostream& os, BTree<Trait>& tree) {
       tree.Print(os);
       return os;
}

#endif