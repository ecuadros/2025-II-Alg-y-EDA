#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <fstream>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

template <typename _keyType, typename _ObjIDType>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       // TODO DONE: agregar funcion de comparacion

       struct Compare { 
              bool operator() (const keyType &a, const keyType &b) const {
                     return a < b;
              }
       };
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
              : m_Root(2 * order + 1, unique),
                m_Order(order),
                m_NumKeys(0),
                m_Unique(unique)
                
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Height = 1;
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
       template <class Fn, class... Args>
       void ForEachT(Fn&& fn, Args&&... args) {
              m_Root.ForEachT(std::forward<Fn>(fn), /*level=*/0,
                            std::forward<Args>(args)...);
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
//move constructor 
public: 
       BTree(BTree&& other) noexcept;
       BTree& operator=(BTree&& other) noexcept;

       //deshabilitar copia
       BTree(const BTree&) = delete;
       BTree& operator=(const BTree&) = delete;

// write and read
       bool Save(const std::string& filename) const;
       bool Load(const std::string& filename);
       // getters const (nuevos o sobrecargas)
       size_t size()   const { return m_NumKeys; }
       size_t height() const { return m_Height;  }
       size_t GetOrder() const { return m_Order; }

       void Print(std::ostream& os) const {
       const_cast<BTNode&>(m_Root).Print(os);
       }

protected:
       BTNode          m_Root;
       size_t          m_Height;  // height of tree
       size_t          m_Order;   // order of tree
       size_t          m_NumKeys; // number of keys
       bool            m_Unique;  // Accept the elements only once ?
       size_t computeHeight(const BTNode& n) const;
       size_t computeSize  (const BTNode& n) const;

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
//insertar template move constructor y move assignment
template <typename Trait>
BTree<Trait>::BTree(BTree&& other) noexcept
    : m_Root(std::move(other.m_Root)),
      m_Height(other.m_Height),
      m_Order(other.m_Order),
      m_NumKeys(other.m_NumKeys),
      m_Unique(other.m_Unique)
{
    other.m_Height = 0;
    other.m_NumKeys = 0;
}

template <typename Trait>
BTree<Trait>& BTree<Trait>::operator=(BTree&& other) noexcept {
    if (this != &other) {
        m_Root   = std::move(other.m_Root);  // se usa el move de CBTreePage
        m_Height = other.m_Height;
        m_Order  = other.m_Order;
        m_NumKeys= other.m_NumKeys;
        m_Unique = other.m_Unique;

        other.m_Height = 0;
        other.m_NumKeys = 0;
    }
    return *this;
}
//write and read
template <typename Trait>
size_t BTree<Trait>::computeHeight(const BTNode& n) const {
    // hoja: altura 1 
    if (!n.m_SubPages[0]) return 1;
    size_t best = 0;
    for (size_t i = 0; i <= n.m_KeyCount; ++i) {
        if (n.m_SubPages[i]) {
            best = std::max(best, computeHeight(*n.m_SubPages[i]));
        }
    }
    return best + 1;
}
template <typename Trait>
size_t BTree<Trait>::computeSize(const BTNode& n) const {
    size_t sum = n.m_KeyCount;
    for (size_t i = 0; i <= n.m_KeyCount; ++i) {
        if (n.m_SubPages[i]) sum += computeSize(*n.m_SubPages[i]);
    }
    return sum;
}

template <typename Trait>
bool BTree<Trait>::Save(const std::string& filename) const {
    std::ofstream ofs(filename);
    if (!ofs) return false;

    // encabezado mínimo del árbol por si quieres validar formato
    ofs << m_Order << ' ' << (m_Unique ? 1 : 0) << '\n';

    // delega al nodo raíz
    m_Root.Write(ofs);

    return true;
}

template <typename Trait>
bool BTree<Trait>::Load(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs) return false;

    size_t order = 0; int uniq = 1;
    ifs >> order >> uniq;

    // Si cambió el “order”, recrea root con la nueva capacidad de hijos
    m_Order  = order ? order : m_Order;
    m_Unique = (uniq != 0);

    // Reconfigura root para hijos 
    m_Root.SetMaxKeysForChilds(m_Order);

    // Limpia y carga
    m_Root.Reset();
    m_Root.Read(ifs);

    // Recalcula métricas
    m_Height = computeHeight(m_Root);
    m_NumKeys = computeSize(m_Root);

    return true;
}
template <typename Trait>
std::ostream& operator<<(std::ostream& os, const BTree<Trait>& t) {
    os << "size=" << t.size() << ", height=" << t.height() << "\n";
    t.Print(os);        
    return os;
}

#endif