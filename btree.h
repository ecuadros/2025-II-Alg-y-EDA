#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <utility>  // Para std::move
#include <mutex>    // Para concurrencia con std::mutex
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

template <typename _keyType, typename _ObjIDType, typename _Compare = std::less<_keyType>>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       using Compare = _Compare;
       // TODO: agregar funcion de comparacion
};

template <typename Trait>
class BTree // this is the full version of the BTree
{ 
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;
       typedef typename Trait::Compare    Compare;
       
       typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
       // Typedefs de iteradores (siguiendo el patrón de doublelinkedlist.h)
       typedef forward_btree_iterator<Trait>    iterator;
       typedef backward_btree_iterator<Trait>   reverse_iterator;

       typedef typename BTNode::ObjectInfo      ObjectInfo;

       // Friend declarations para los iteradores
       friend class forward_btree_iterator<Trait>;
       friend class backward_btree_iterator<Trait>;

public:
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
              : m_Order(order),
                m_Root(2 * order  + 1, unique),
                m_Unique(unique),
                m_NumKeys(0)
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Root.SetParent(nullptr);  // Root no tiene padre
              m_Height = 1;
       }

       // Delete copy constructor and copy assignment (evitar copias accidentales)
       BTree(const BTree&) = delete;
       BTree& operator=(const BTree&) = delete;

       // Move Constructor
       BTree(BTree&& other) noexcept
              : m_Order(other.m_Order),
                m_Root(std::move(other.m_Root)),
                m_Height(other.m_Height),
                m_Unique(other.m_Unique),
                m_NumKeys(other.m_NumKeys)
       {
              // Lock both mutexes in consistent order (by address) to prevent deadlock
              if (this < &other) {
                     m_mutex.lock();
                     other.m_mutex.lock();
              } else {
                     other.m_mutex.lock();
                     m_mutex.lock();
              }
              
              // El root no debe tener padre
              m_Root.SetParent(nullptr);
              
              // Reset other to a valid but empty state
              other.m_Height = 1;
              other.m_NumKeys = 0;
              
              // Unlock in reverse order
              if (this < &other) {
                     other.m_mutex.unlock();
                     m_mutex.unlock();
              } else {
                     m_mutex.unlock();
                     other.m_mutex.unlock();
              }
       }

       // Move Assignment Operator
       BTree& operator=(BTree&& other) noexcept
       {
              if (this != &other) {
                     // Lock both mutexes in consistent order
                     if (this < &other) {
                            m_mutex.lock();
                            other.m_mutex.lock();
                     } else {
                            other.m_mutex.lock();
                            m_mutex.lock();
                     }
                     
                     // Move data from other
                     m_Order = other.m_Order;
                     m_Root = std::move(other.m_Root);
                     m_Height = other.m_Height;
                     m_Unique = other.m_Unique;
                     m_NumKeys = other.m_NumKeys;

                     // El root no debe tener padre
                     m_Root.SetParent(nullptr);

                     // Reset other to a valid but empty state
                     other.m_Height = 1;
                     other.m_NumKeys = 0;
                     
                     // Unlock in reverse order
                     if (this < &other) {
                            other.m_mutex.unlock();
                            m_mutex.unlock();
                     } else {
                            m_mutex.unlock();
                            other.m_mutex.unlock();
                     }
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
       {      
              // Lock para thread-safety
              m_mutex.lock();
              
              ObjIDType ObjID = -1;
              m_Root.Search(key, ObjID);
              
              // Unlock antes de return
              m_mutex.unlock();
              
              return ObjID;
       }
       size_t            size() const
       {
              m_mutex.lock();
              size_t result = m_NumKeys;
              m_mutex.unlock();
              return result;
       }
       
       size_t            height() const
       {
              m_mutex.lock();
              size_t result = m_Height;
              m_mutex.unlock();
              return result;
       }
       
       size_t            GetOrder() const
       {
              m_mutex.lock();
              size_t result = m_Order;
              m_mutex.unlock();
              return result;
       }

       void            Print (ostream &os)
       {
              m_mutex.lock();
              m_Root.Print(os);
              m_mutex.unlock();
       }
       
       std::ostream& Write(std::ostream& os) const
       {
               m_mutex.lock();
               
               os << m_Order << "," << (m_Unique ? "1" : "0") << "\n";
               os << m_NumKeys << "\n";
               
               BTree* non_const_this = const_cast<BTree*>(this);
               for(auto it = non_const_this->begin(); it != non_const_this->end(); ++it) {
                       os << it->key << "," << it->ObjID << "\n";
               }
               
               m_mutex.unlock();
               
               return os;
       }

       std::istream& Read(std::istream& is)
       {
               // Lock para preparar estructura
               m_mutex.lock();
               
               size_t order;
               int unique_int;
               size_t count;
               char comma;
               
               is >> order >> comma >> unique_int;  
               is >> count;
               
               m_Root.Reset();
               m_Order = order;
               m_Unique = (unique_int == 1);
               m_NumKeys = 0;
               m_Height = 1;
               
               m_Root = BTNode(2 * order + 1, m_Unique);
               m_Root.SetMaxKeysForChilds(order);
               m_Root.SetParent(nullptr);
               
               // Unlock ANTES de llamar Insert (evitar deadlock)
               m_mutex.unlock();
               
               // Insert hace su propio lock/unlock por cada elemento
               for(size_t i = 0; i < count; i++) {
                       keyType key;
                       ObjIDType objID;
                       
                       is >> key >> comma >> objID; 
                       Insert(key, objID);  
               }
               
               return is;
       }
       
       // Template versions using std::invoke (TODO #6, #7, #8 completed)
       template <typename Func, typename... Args>
       void ForEach(Func&& func, Args&&... args)
       {
              m_mutex.lock();
              m_Root.ForEach(std::forward<Func>(func), std::forward<Args>(args)...);
              m_mutex.unlock();
       }

       template <typename Func, typename... Args>
       ObjectInfo* FirstThat(Func&& func, Args&&... args)
       {
              m_mutex.lock();
              ObjectInfo* result = m_Root.FirstThat(std::forward<Func>(func), std::forward<Args>(args)...);
              m_mutex.unlock();
              return result;
       }

       // Iteradores forward (in-order traversal: orden ascendente)
       iterator begin()
       {
               if (m_NumKeys == 0)
                       return end();
               return iterator(this, &m_Root, 0);
       }

       iterator end()
       {
               return iterator(this, nullptr, 0);
       }

       // Iteradores reverse (reverse in-order: orden descendente)
       reverse_iterator rbegin()
       {
               if (m_NumKeys == 0)
                       return rend();

               // Ir al último elemento (nodo más a la derecha)
               BTNode* node = &m_Root;
               while (node->m_SubPages[node->GetNumberOfKeys()])
                       node = node->m_SubPages[node->GetNumberOfKeys()];

               return reverse_iterator(this, node, node->GetNumberOfKeys() - 1);
       }

       reverse_iterator rend()
       {
               return reverse_iterator(this, nullptr, 0);
       }

       friend std::ostream& operator<<(std::ostream& os, const BTree& tree)
       {
               return tree.Write(os);
       }

       friend std::istream& operator>>(std::istream& is, BTree& tree)
       {
               return tree.Read(is);
       }

protected:
       BTNode          m_Root;
       size_t          m_Height;  // height of tree
       size_t          m_Order;   // order of tree
       size_t          m_NumKeys; // number of keys
       bool            m_Unique;  // Accept the elements only once ?
       
       // Mutex para concurrencia (TODO #11: Thread-safety)
       mutable std::mutex m_mutex;
};     

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const long ObjID){
       // Lock para thread-safety
       m_mutex.lock();
       
       bt_ErrorCode error = m_Root.Insert(key, ObjID);
       
       // Preparar resultado sin return intermedio
       bool result = false;
       
       if( error == bt_duplicate ) {
              result = false;
       } else {
              m_NumKeys++;
              if( error == bt_overflow ){
                     m_Root.SplitRoot();
                     m_Height++;
              }
              result = true;
       }
       
       // Unlock antes de return
       m_mutex.unlock();
       
       return result;
}

template <typename Trait>
bool BTree<Trait>::Remove (const keyType key, const long ObjID)
{
       // Lock para thread-safety
       m_mutex.lock();
       
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       
       // Preparar resultado sin return intermedio
       bool result = false;
       
       if( error == bt_duplicate || error == bt_nofound ) {
              result = false;
       } else {
              m_NumKeys--;
              if( error == bt_rootmerged )
                     m_Height--;
              result = true;
       }
       
       // Unlock antes de return
       m_mutex.unlock();
       
       return result;
}

#endif