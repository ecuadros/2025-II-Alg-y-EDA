#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

template <typename _keyType, typename _ObjIDType>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       // TODO: agregar funcion de comparacion
       using CompareFunction = std::less<keyType>;
};

template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;
       typedef typename Trait::CompareFunction CompareFunction;
       typedef CBTreePage <Trait> BTNode;// useful shorthand

       
public:
       // typedef CBTreePage <Trait> BTNode;// decomentar para probar el move test
       //typedef ObjectInfo iterator;
       // typedef typename BTNode::lpfnForEach2    lpfnForEach2;
       // typedef typename BTNode::lpfnForEach3    lpfnForEach3;
       // typedef typename BTNode::lpfnFirstThat2  lpfnFirstThat2;
       // typedef typename BTNode::lpfnFirstThat3  lpfnFirstThat3;
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
       // void            ForEach( lpfnForEach2 lpfn, void *pExtra1 )
       // {               m_Root.ForEach(lpfn, 0, pExtra1);              }
       // void            ForEach( lpfnForEach3 lpfn, void *pExtra1, void *pExtra2)
       // {               m_Root.ForEach(lpfn, 0, pExtra1, pExtra2);     }


       //Read y Write
       std::ostream&  Write(ostream &os);
       std::istream&  Read(istream &is);


       std::ostream&  WriteBinaryTreeFormat(std::ostream& os);
       std::istream&  ReadBinaryTreeFormat(std::istream& is);
       

       template <typename Function>
       void ForEach( Function fn ){
              m_Root.ForEach(fn, 0);
       }

       template <typename Function>
       ObjectInfo* FirstThat( Function fn ){
              return m_Root.FirstThat(fn, 0);
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
std::ostream& BTree<Trait>::Write(std::ostream &os) {
    // Cabecera del arbol
    os << "BTree " << m_Order << " " << m_Height << " " << m_NumKeys << " " << m_Unique << "\n";
    
    // Escribimos la raiz
    m_Root.Write(os);
    
    return os;
}

template <typename Trait>
std::istream& BTree<Trait>::Read(std::istream &is) {
    std::string tag;
    is >> tag; //leemos la cabecera 
    
    if(tag == "BTree") {
       is >> m_Order >> m_Height >> m_NumKeys >> m_Unique;
        
        // Leemos la raiz
       m_Root.Read(is);
    }
    
    return is;
}


// Write alternativo usando ForEach
template <typename Trait>
std::ostream& BTree<Trait>::WriteBinaryTreeFormat(std::ostream& os) {
    os << "BTreeSimple " << m_NumKeys << " elements: ";
    
    // aca vamos a usar el ForEach como una alternativa a un formato mas simple
    ForEach([&os](auto& info, size_t level) {
        os << info.key << " ";
    });
    
    return os;
}

//implementacion para leer el archivo BT.txt
template <typename Trait>
std::istream& BTree<Trait>::ReadBinaryTreeFormat(std::istream& is) {
    std::string line;
    std::getline(is, line);
    
    m_Root.Reset();
    m_NumKeys = 0;
    m_Height = 1;
    
    //se va a convertir cada caracter en una clave e insertar en el arbol
    for (char c : line) {
        if (std::isdigit(c)) {
            int key = c - '0';  // Convertir char a int
            Insert(key, key);
        }
    }
    
    return is;
}

#endif