/**
 * @file btree.h
 * @brief Thread-safe B-Tree implementation with disk persistence support
 * 
 * This file contains the implementation of a generic B-Tree data structure
 * that supports concurrent access through reader-writer locks and provides
 * binary serialization for persistent storage.
 */

#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <utility>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <fstream>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

/**
 * @brief Trait structure to configure BTree type parameters
 * 
 * @tparam _keyType The type of keys stored in the B-Tree
 * @tparam _ObjIDType The type of object identifiers associated with keys
 * @tparam _CompareF Comparison function object type for ordering keys
 */
template <typename _keyType, typename _ObjIDType, typename _CompareF>
struct BTreeTrait
{
       using keyType = _keyType;      /**< Type of keys */
       using ObjIDType = _ObjIDType;  /**< Type of object IDs */  
       using CompareF = _CompareF;    /**< Comparison function type */

};

/**
 * @brief Thread-safe B-Tree data structure with disk persistence
 * 
 * This class implements a B-Tree that supports concurrent read/write operations
 * using reader-writer locks. It provides standard B-Tree operations (Insert, Remove, Search)
 * as well as disk persistence (Write, Read) and functional programming utilities (ForEach, FirstThat).
 * 
 * @tparam Trait Type trait containing keyType, ObjIDType, and CompareF definitions
 */
template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType    keyType;   /**< Key type from trait */
       typedef typename Trait::ObjIDType  ObjIDType; /**< Object ID type from trait */
       
       typedef CBTreePage <Trait> BTNode;/**< Alias for tree node type */

       /**
        * @brief File header structure for binary serialization
        */
       struct FileHeader {
           size_t numKeys;  /**< Total number of keys in the tree */    
           size_t height;   /**< Height of the tree */     

           /**
            * @brief Write header to output stream
            * @param os Output stream
            * @return true if write succeeded, false otherwise
            */
           bool Write(std::ostream& os) const {
               os.write(reinterpret_cast<const char*>(this), sizeof(FileHeader));
               return os.good();
           }

           /**
            * @brief Read header from input stream
            * @param is Input stream
            * @return true if read succeeded, false otherwise
            */
           bool Read(std::istream& is) {
               is.read(reinterpret_cast<char*>(this), sizeof(FileHeader));
               return is.good();

           }
       };
public:
       friend class BTreeIterator<Trait>;
       typedef BTreeIterator<Trait> iterator;
       friend class BTreeReverseIterator<Trait>;
       typedef BTreeReverseIterator<Trait> reverse_iterator;


public:
       //typedef ObjectInfo iterator;
       typedef typename BTNode::ObjectInfo     ObjectInfo; /**< Type for key-ObjID pairs */
       
       /**
        * @brief Friend function for stream output operator
        * @tparam U Trait type (must differ from class template parameter)
        * @param os Output stream
        * @param tree BTree to output
        * @return Reference to output stream
        */
       template<typename U>
       friend std::ostream& operator<<(std::ostream& os, BTree<U>& tree);

public:
       /**
        * @brief Construct a new BTree
        * @param order Order of the B-Tree (maximum keys per node = 2*order + 1)
        * @param unique If true, duplicate keys are not allowed
        */
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
              : m_Order(order),
                m_Root(2 * order  + 1, unique),
                m_Unique(unique),
                m_NumKeys(0)
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Height = 1;
       }
       
       /**
        * @brief Move constructor - transfers ownership of tree resources
        * @param btree Source BTree to move from (left in valid but unspecified state)
        */
       BTree(BTree&& btree) noexcept
       {
              std::scoped_lock lock(m_mutex, btree.m_mutex);
              m_Root = std::move(btree.m_Root);
              m_Height = std::exchange(btree.m_Height,1);
              m_Order = btree.m_Order,
              m_Unique = btree.m_Unique;
              m_NumKeys = std::exchange(btree.m_NumKeys,0);
       }
       
       /**
        * @brief Move assignment operator - transfers ownership to existing object
        * @param btree Source BTree to move from
        * @return Reference to this object
        */
       BTree& operator=(BTree&& btree  ) noexcept 
       {
              if (this != &btree  )
              {
                     std::scoped_lock(m_mutex,btree.m_mutex); //Block both mutex
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
       
       /**
        * @brief Destructor
        */
       ~BTree() {}
       
       /**
        * @brief Write tree to binary file
        * @param filename Path to output file
        * @return true if write succeeded, false otherwise
        */
       bool            Write(const std::string& filename);
       
       /**
        * @brief Read tree from binary file
        * @param filename Path to input file
        * @return true if read succeeded, false otherwise
        */
       bool            Read(const std::string& filename);
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();
       
       /**
        * @brief Insert a key-ObjID pair into the tree
        * @param key Key to insert
        * @param ObjID Object identifier associated with the key
        * @return true if insertion succeeded, false if duplicate (when unique=true)
        */
       bool            Insert (const keyType key, const long ObjID);
       
       /**
        * @brief Remove a key-ObjID pair from the tree
        * @param key Key to remove
        * @param ObjID Object identifier to remove
        * @return true if removal succeeded, false if not found
        */
       bool            Remove   (const keyType key, const long ObjID);
       
       /**
        * @brief Search for a key and return its associated ObjID
        * @param key Key to search for
        * @return ObjID if found, -1 otherwise
        */
       ObjIDType       Search (const keyType key)
       {      
              std::shared_lock<std::shared_mutex> lock(m_mutex);
              ObjIDType ObjID = -1;
              m_Root.Search(key, ObjID);
              return ObjID;
       }
       
       /**
        * @brief Get total number of keys in the tree
        * @return Number of keys
        */
       size_t            size()  
       { 
              std::shared_lock<std::shared_mutex> lock(m_mutex);
              return m_NumKeys; 
       }
       
       /**
        * @brief Get height of the tree
        * @return Tree height
        */
       size_t            height() 
       { 
              std::shared_lock<std::shared_mutex> lock(m_mutex);
              return m_Height;      
       }
       
       /**
        * @brief Get order of the tree
        * @return Tree order
        */
       size_t            GetOrder() { 
              std::shared_lock<std::shared_mutex> lock(m_mutex);
              return m_Order;     
       }

       /**
        * @brief Print tree structure to output stream
        * @param os Output stream
        */
       void            Print (ostream &os)
       {               m_Root.Print(os);                              }
       
       /**
        * @brief Apply a function to each key-ObjID pair in the tree
        * 
        * Traverses the tree and invokes the provided function with each ObjectInfo
        * and any additional forwarded arguments.
        * 
        * @tparam Func Function type (callable object)
        * @tparam Args Variadic additional argument types
        * @param func Function to apply to each element
        * @param args Additional arguments to forward to func
        */
       template <typename Func, typename... Args>
       void            ForEach( Func&& func, Args&&... args )
       {               m_Root.ForEach(std::forward<Func>(func), 0, std::forward<Args>(args)...);              }

       /**
        * @brief Find first element matching a predicate
        * 
        * Traverses the tree until finding an ObjectInfo for which the predicate returns true.
        * 
        * @tparam Func Predicate function type
        * @tparam Args Variadic additional argument types
        * @param func Predicate function to test each element
        * @param args Additional arguments to forward to func
        * @return Pointer to first matching ObjectInfo, or nullptr if none found
        */
       template <typename Func, typename... Args>
       ObjectInfo*     FirstThat( Func&& func, Args&&... args )
       {               return m_Root.FirstThat(std::forward<Func>(func), 0, std::forward<Args>(args)...);     }
      
       //typedef               ObjectInfo iterator;
private:
       
       /**
        * @brief Recursively write a node and its children to output stream
        * @param os Output stream
        * @param node Node to write
        * @return true if write succeeded, false otherwise
        */
       bool WriteNode(std::ostream& os, BTNode* node) {
           if (!node) return true;

           // Write  number of keys
           size_t count = node->GetNumberOfKeys();
           os.write(reinterpret_cast<const char*>(&count), sizeof(count));
           if (!os.good()) return false;

           // write keys (Serialize key and ObjID)
           for (size_t i = 0; i < count; ++i) {
              const auto &oi = node->m_Keys[i];
              os.write(reinterpret_cast<const char*>(&oi.key), sizeof(oi.key));
              os.write(reinterpret_cast<const char*>(&oi.ObjID), sizeof(oi.ObjID));
              if (!os.good()) return false;
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

       /**
        * @brief Recursively read a node and its children from input stream
        * @param is Input stream
        * @param node Node to populate
        * @return true if read succeeded, false otherwise
        */
       bool ReadNode(std::istream& is, BTNode* node) {
           if (!node) return false;

           // Read number of keys
           size_t count;
           is.read(reinterpret_cast<char*>(&count), sizeof(count));
           if (!is.good()) return false;

           // Read keys (Deserialize key and ObjID)
           node->m_Keys.resize(count);
           for (size_t i = 0; i < count; i++) {
              keyType k;
              ObjIDType id;
              is.read(reinterpret_cast<char*>(&k), sizeof(k));
              is.read(reinterpret_cast<char*>(&id), sizeof(id));
              if (!is.good()) return false;
              node->m_Keys[i].key = k;
              node->m_Keys[i].ObjID = id;
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
       BTNode          m_Root;     /**< Root node of the tree */
       size_t          m_Height;   /**< Height of tree */
       size_t          m_Order;    /**< Order of tree */
       size_t          m_NumKeys;  /**< Total number of keys */
       bool            m_Unique;   /**< Accept the elements only once ? */
       mutable         std::shared_mutex m_mutex; /**< Mutex for thread-safe access (mutable: lockable in const funcs) */

public:
       iterator begin()
       {
               std::shared_lock<std::shared_mutex> lock(m_mutex);  // Shared lock: Multiple threads can access it at the same time (Just lecture)   
               if (m_NumKeys == 0)
                       return end();
               // Descender hasta el hijo más a la izquierda
               BTNode* page = &m_Root;
               while (!page->m_SubPages.empty() && page->m_SubPages[0] != nullptr)
                       page = page->m_SubPages[0];
               return iterator(page, 0);
       }

       iterator end()
       {
               return iterator(nullptr, 0);
       }

       reverse_iterator rbegin()
       {
               std::shared_lock<std::shared_mutex> lock(m_mutex);  // Shared lock: Multiple threads can access it at the same time (Just lecture)   

               if (m_NumKeys == 0)
                       return rend();

               // Descender hasta el hijo más a la derecha
               BTNode* page = &m_Root;
               while (!page->m_SubPages.empty()){
                     size_t idx = page->m_KeyCount;
                     page = page->m_SubPages[idx];
               }


               return reverse_iterator(page, page->m_KeyCount - 1);
       }
      
       reverse_iterator rend()
       {
               return reverse_iterator(nullptr, 0);
       }
};     

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const long ObjID){
       std::unique_lock<std::shared_mutex> _lk(m_mutex);
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
       std::unique_lock<std::shared_mutex> _lk(m_mutex);
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if( error == bt_duplicate || error == bt_nofound )
               return false;
       m_NumKeys--;

       if( error == bt_rootmerged )
               m_Height--;
       return true;
}

template <typename Trait>
bool BTree<Trait>::Write(const std::string& filename) {
       std::unique_lock<std::shared_mutex> _lk(m_mutex);
       std::ofstream file(filename, std::ios::binary);
       if (!file) return false;

       // Write Header
       FileHeader header{m_NumKeys, m_Height};
       if (!header.Write(file)) return false;
       // Write Tree recursively
       return WriteNode(file, &m_Root);
}

template <typename Trait>
bool BTree<Trait>::Read(const std::string& filename) {
       std::unique_lock<std::shared_mutex> _lk(m_mutex);
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


template <typename Trait>
std::ostream& operator<<(std::ostream& os,  BTree<Trait>& tree) {
    std::shared_lock<std::shared_mutex> _lk(tree.m_mutex);
    os << "BTree: order=" << tree.m_Order << ", height=" << tree.m_Height 
       << ", keys=" << tree.m_NumKeys << "\n";
    tree.Print(os);  
    return os;
}


template <typename Trait>
class BTreeIterator
{
       friend class BTree<Trait>;
       typedef CBTreePage<Trait> BTPage;
       typedef typename BTPage::ObjectInfo ObjectInfo;

private:
       BTPage* m_CurrentPage;
       size_t m_CurrentIndex;

       // Constructor
       BTreeIterator(BTPage* page, size_t index): m_CurrentPage(page), m_CurrentIndex(index){}

public:
       using iterator_category = std::forward_iterator_tag; // custom forward iterator
       using value_type = ObjectInfo;
       using difference_type = std::ptrdiff_t; // Large enough to represent the diff between any two pointerts to elements on the same array
       using pointer = ObjectInfo*;
       using reference = ObjectInfo&;

       // Default constructor
       BTreeIterator(): m_CurrentPage(nullptr), m_CurrentIndex(0){}

       // Reference
       ObjectInfo& operator*(){ return m_CurrentPage->m_Keys[m_CurrentIndex];}

       // Access to members
       ObjectInfo* operator->() { return &(m_CurrentPage->m_Keys[m_CurrentIndex]);}

       BTreeIterator& operator++();

       BTreeIterator operator++(int){
              BTreeIterator temp = *this;
              ++(*this);
              return temp;
       }

       bool operator==(const BTreeIterator& other) const {
              if(m_CurrentPage == nullptr && other.m_CurrentPage == nullptr)
                     return true;
              return m_CurrentPage == other.m_CurrentPage && m_CurrentIndex == other.m_CurrentIndex;
       }

       bool operator!=(const BTreeIterator& other) const{
              return !(*this == other);
       }

};

template <typename Trait>
BTreeIterator<Trait>& BTreeIterator<Trait>::operator++()
{
       if (!m_CurrentPage) return *this;

       // Si hay hijo derecho, ir al leftmost de ese subárbol
       BTPage* rightChild = m_CurrentPage->m_SubPages[m_CurrentIndex + 1];
       if (rightChild != nullptr) {
               BTPage* leftmost = rightChild;
               while (leftmost->m_SubPages[0] != nullptr) {
                       leftmost = leftmost->m_SubPages[0];
               }
               m_CurrentPage = leftmost;
               m_CurrentIndex = 0;
               return *this;
       }

       // Avanzar en el nodo actual si hay más keys a la derecha
       if (m_CurrentIndex + 1 < m_CurrentPage->m_KeyCount) {
               m_CurrentIndex++;
               return *this;
       }

       // Subir al padre hasta encontrar una key no visitada
       BTPage* child = m_CurrentPage;
       BTPage* parent = m_CurrentPage->m_Parent;

       while (parent != nullptr) {
               // Buscar posición del hijo en el arreglo de SubPages del padre
               size_t childPos = 0;
               while (childPos <= parent->m_KeyCount && parent->m_SubPages[childPos] != child) {
                       childPos++;
               }

               // Si encontramos una key válida en el padre, esa es la siguiente
               if (childPos < parent->m_KeyCount) {
                       m_CurrentPage = parent;
                       m_CurrentIndex = childPos;
                       return *this;
               }

               // Continuar subiendo en el árbol
               child = parent;
               parent = parent->m_Parent;
       }

       // No hay más elementos,  end
       m_CurrentPage = nullptr;
       m_CurrentIndex = 0;
       return *this;
}

template <typename Trait>
class BTreeReverseIterator
{
       friend class BTree<Trait>;
       typedef CBTreePage<Trait> BTPage;
       typedef typename BTPage::ObjectInfo ObjectInfo;

private:
       BTPage* m_CurrentPage;
       size_t m_CurrentIndex;

       // Constructor
       BTreeReverseIterator(BTPage* page, size_t index): m_CurrentPage(page), m_CurrentIndex(index){}

public:
       using iterator_category = std::forward_iterator_tag; // custom forward iterator
       using value_type = ObjectInfo;
       using difference_type = std::ptrdiff_t; // Large enough to represent the diff between any two pointerts to elements on the same array
       using pointer = ObjectInfo*;
       using reference = ObjectInfo&;

       // Default constructor
       BTreeReverseIterator(): m_CurrentPage(nullptr), m_CurrentIndex(0){}

       // Reference
       ObjectInfo& operator*(){ return m_CurrentPage->m_Keys[m_CurrentIndex];}

       // Access to members
       ObjectInfo* operator->() { return &(m_CurrentPage->m_Keys[m_CurrentIndex]);}

       BTreeReverseIterator& operator++();

       BTreeReverseIterator operator++(int){
              BTreeReverseIterator temp = *this;
              ++(*this);
              return temp;
       }

       bool operator==(const BTreeReverseIterator& other) const {
              if(m_CurrentPage == nullptr && other.m_CurrentPage == nullptr)
                     return true;
              return m_CurrentPage == other.m_CurrentPage && m_CurrentIndex == other.m_CurrentIndex;
       }

       bool operator!=(const BTreeReverseIterator& other) const{
              return !(*this == other);
       }

};

template <typename Trait>
BTreeReverseIterator<Trait>& BTreeReverseIterator<Trait>::operator++()
{
       if (!m_CurrentPage) return *this;

       // Si hay hijo izquierdo, ir al rightmost de ese subárbol
       BTPage* leftChild = m_CurrentPage->m_SubPages[m_CurrentIndex];
       if (leftChild != nullptr) {
               BTPage* rightmost = leftChild;
               while (rightmost->m_SubPages[rightmost->m_KeyCount] != nullptr) {
                       rightmost = rightmost->m_SubPages[rightmost->m_KeyCount];
               }
               m_CurrentPage = rightmost;
               m_CurrentIndex = rightmost->m_KeyCount - 1;
               return *this;
       }

       // Retroceder en el nodo actual si hay más keys a la izquierda
       if (m_CurrentIndex > 0) {
               m_CurrentIndex--;
               return *this;
       }

       // Subir al padre hasta encontrar una key no visitada
       BTPage* child = m_CurrentPage;
       BTPage* parent = m_CurrentPage->m_Parent;

       while (parent != nullptr) {
               // Buscar posición del hijo en el arreglo de SubPages del padre
               size_t childPos = 0;
               while (childPos <= parent->m_KeyCount && parent->m_SubPages[childPos] != child) {
                       childPos++;
               }

               // Si el hijo está en una posición > 0, la key anterior del padre es la siguiente
               if (childPos > 0) {
                       m_CurrentPage = parent;
                       m_CurrentIndex = childPos - 1;
                       return *this;
               }

               // Continuar subiendo en el árbol
               child = parent;
               parent = parent->m_Parent;
       }

       // No hay más elementos, end
       m_CurrentPage = nullptr;
       m_CurrentIndex = 0;
       return *this;
}


#endif