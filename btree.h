#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <iterator>
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
    // Forward iterator
    class iterator {
        friend class BTree;
    private:
        BTNode* m_Node;
        size_t m_Index;
        BTree* m_Tree;  // Puntero al árbol para navegar desde end()
        
        // Encuentra el nodo más a la izquierda desde un nodo dado
        void goToLeftmost(BTNode* node) {
            m_Node = node;
            if(m_Node) {
                while(m_Node->m_SubPages[0]) {
                    m_Node = m_Node->m_SubPages[0];
                }
                m_Index = 0;
            }
        }
        
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = ObjectInfo;
        using difference_type = std::ptrdiff_t;
        using pointer = ObjectInfo*;
        using reference = ObjectInfo&;
        
        iterator(BTNode* node = nullptr, size_t index = 0, BTree* tree = nullptr) 
            : m_Node(node), m_Index(index), m_Tree(tree) {}
        
        reference operator*() { return m_Node->m_Keys[m_Index]; }
        pointer operator->() { return &m_Node->m_Keys[m_Index]; }
        
        // Pre-incremento (avanzamos)
        iterator& operator++() {
            if(!m_Node) return *this;
            
            // Si hay hijo derecho, ir al más izquierdo del hijo derecho
            if(m_Node->m_SubPages[m_Index + 1]) {
                m_Node = m_Node->m_SubPages[m_Index + 1];
                while(m_Node->m_SubPages[0]) {
                    m_Node = m_Node->m_SubPages[0];
                }
                m_Index = 0;
            }
            // Si no hay hijo derecho, siguiente key en el mismo nodo
            else if(m_Index + 1 < m_Node->m_KeyCount) {
                ++m_Index;
            }
            // Subir al padre
            else {
                BTNode* child = m_Node;
                m_Node = m_Node->m_Parent;
                
                while(m_Node) {
                    // Buscar índice del hijo en el padre
                    for(size_t i = 0; i <= m_Node->m_KeyCount; ++i) {
                        if(m_Node->m_SubPages[i] == child) {
                            if(i < m_Node->m_KeyCount) {
                                m_Index = i;
                                return *this;
                            }
                            break;
                        }
                    }
                    child = m_Node;
                    m_Node = m_Node->m_Parent;
                }
                // Llegamos al final
                m_Node = nullptr;
            }
            return *this;
        }
        
        // Post-incremento
        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }
        
        // Pre-decremento (retrocedemos)
        iterator& operator--() {
            // Si estamos en end(), ir al último elemento
            if(!m_Node && m_Tree) {
                m_Node = &m_Tree->m_Root;
                // Ir al nodo más a la derecha
                while(m_Node->m_SubPages[m_Node->m_KeyCount]) {
                    m_Node = m_Node->m_SubPages[m_Node->m_KeyCount];
                }
                m_Index = m_Node->m_KeyCount > 0 ? m_Node->m_KeyCount - 1 : 0;
                return *this;
            }
            
            if(!m_Node) return *this;
            
            // Si hay hijo izquierdo, ir al más derecho del hijo izquierdo
            if(m_Node->m_SubPages[m_Index]) {
                m_Node = m_Node->m_SubPages[m_Index];
                while(m_Node->m_SubPages[m_Node->m_KeyCount]) {
                    m_Node = m_Node->m_SubPages[m_Node->m_KeyCount];
                }
                m_Index = m_Node->m_KeyCount - 1;
            }
            // Si no hay hijo izquierdo, key anterior en el mismo nodo
            else if(m_Index > 0) {
                --m_Index;
            }
            // Subir al padre
            else {
                BTNode* child = m_Node;
                m_Node = m_Node->m_Parent;
                
                while(m_Node) {
                    // Buscar índice del hijo en el padre
                    for(size_t i = 0; i <= m_Node->m_KeyCount; ++i) {
                        if(m_Node->m_SubPages[i] == child) {
                            if(i > 0) {
                                m_Index = i - 1;
                                return *this;
                            }
                            break;
                        }
                    }
                    child = m_Node;
                    m_Node = m_Node->m_Parent;
                }
                // Llegamos al inicio
                m_Node = nullptr;
            }
            return *this;
        }
        
        // Post-decremento
        iterator operator--(int) {
            iterator temp = *this;
            --(*this);
            return temp;
        }
        
        bool operator==(const iterator& other) const {
            return m_Node == other.m_Node && 
                   (m_Node == nullptr || m_Index == other.m_Index);
        }
        
        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }
    };
    
    // Métodos begin/end
    iterator begin() {
        BTNode* node = &m_Root;
        // Ir al nodo más a la izquierda
        while(node->m_SubPages[0]) {
            node = node->m_SubPages[0];
        }
        return iterator(node, 0, this);
    }
    
    iterator end() {
        return iterator(nullptr, 0, this);
    }
    
    // Backward iterator

    using reverse_iterator = std::reverse_iterator<iterator>;
    
    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }
    
    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

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