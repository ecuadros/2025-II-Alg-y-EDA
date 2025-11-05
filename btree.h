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
       
       static bool compare(const keyType& k1, const keyType& k2) {
           static Compare comp;
           return comp(k1, k2);
       }
       
       static bool equal(const keyType& k1, const keyType& k2) {
           return !compare(k1, k2) && !compare(k2, k1);
       }
};

template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;
       
       typedef CBTreePage <Trait> BTNode;// useful shorthand

       // Estructura para el header del archivo
       struct FileHeader {
           size_t numKeys;      // Número total de claves
           size_t height;       // Altura del árbol

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
       ~BTree() {}

       // Escribe el árbol a un archivo
       bool Write(const std::string& filename) {
           std::ofstream file(filename, std::ios::binary);
           if (!file) return false;

           // Escribir el header solo con numKeys y height
           FileHeader header{m_NumKeys, m_Height};
           if (!header.Write(file)) return false;

           // Escribir el árbol recursivamente
           return WriteNode(file, &m_Root);
       }

       // Lee el árbol desde un archivo
       bool Read(const std::string& filename) {
           std::ifstream file(filename, std::ios::binary);
           if (!file) return false;

           // Leer el header
           FileHeader header;
           if (!header.Read(file)) return false;

           // Actualizar solo numKeys y height
           m_NumKeys = header.numKeys;
           m_Height = header.height;

           // Leer el árbol recursivamente (m_Root ya está inicializada por el constructor)
           return ReadNode(file, &m_Root);
       }

       bool Insert (const keyType key, const long ObjID);
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
       void            ForEach( lpfnForEach2 lpfn, void *pExtra1 )
       {               m_Root.ForEach(lpfn, 0, pExtra1);              }
       void            ForEach( lpfnForEach3 lpfn, void *pExtra1, void *pExtra2)
       {               m_Root.ForEach(lpfn, 0, pExtra1, pExtra2);     }
       ObjectInfo*     FirstThat( lpfnFirstThat2 lpfn, void *pExtra1 )
       {               return m_Root.FirstThat(lpfn, 0, pExtra1);     }
       ObjectInfo*     FirstThat( lpfnFirstThat3 lpfn, void *pExtra1, void *pExtra2)
       {               return m_Root.FirstThat(lpfn, 0, pExtra1, pExtra2);   }
       //typedef               ObjectInfo iterator;

private:
       // Escribe un nodo y sus subárboles recursivamente
       bool WriteNode(std::ostream& os, BTNode* node) {
           if (!node) return true;

           // Escribir número de claves
           size_t count = node->GetNumberOfKeys();
           os.write(reinterpret_cast<const char*>(&count), sizeof(count));
           if (!os.good()) return false;

           // Escribir las claves
           for (size_t i = 0; i < count; i++) {
               if (!node->m_Keys[i].Write(os)) return false;
           }

           // Escribir recursivamente los subárboles
           for (size_t i = 0; i <= count; i++) {
               bool hasChild = node->m_SubPages[i] != nullptr;
               os.write(reinterpret_cast<const char*>(&hasChild), sizeof(hasChild));
               if (hasChild && !WriteNode(os, node->m_SubPages[i])) {
                   return false;
               }
           }

           return true;
       }

       // Lee un nodo y sus subárboles recursivamente
       bool ReadNode(std::istream& is, BTNode* node) {
           if (!node) return false;

           // Leer número de claves
           size_t count;
           is.read(reinterpret_cast<char*>(&count), sizeof(count));
           if (!is.good()) return false;

           // Leer las claves
           for (size_t i = 0; i < count; i++) {
               ObjectInfo info;
               if (!info.Read(is)) return false;
               node->m_Keys[i] = std::move(info);
           }
           node->m_KeyCount = count;

           // Leer recursivamente los subárboles
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

#endif