#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <mutex>
#include <shared_mutex>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5;

/**
 * @brief Trait structure for BTree key and object ID types
 * @tparam _keyType Type of the key
 * @tparam _ObjIDType Type of the object ID
 */
template <typename _keyType, typename _ObjIDType>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;

       /**
        * @brief Compares two keys for equality
        * @param a First key
        * @param b Second key
        * @return true if keys are equal, false otherwise
        */
       static bool isEqual(const keyType& a, const keyType& b) {
              return a == b;
       }
};

/**
 * @brief Thread-safe B-Tree implementation
 * @tparam Trait Trait type defining key and object ID types
 *
 * This implementation provides:
 * - Concurrent read operations using shared locks
 * - Exclusive write operations using unique locks
 * - Forward and backward iterators
 * - Move semantics support
 */
template <typename Trait>
class BTree
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;
       
       typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
       typedef typename BTNode::ObjectInfo      ObjectInfo;

       /**
        * @brief Iterator class for traversing BTree elements
        *
        * Supports both forward and backward iteration through the tree.
        */
       class Iterator {
       private:
              std::vector<ObjectInfo*> items;
              size_t index;
              bool reverse;

       public:
              Iterator(const std::vector<ObjectInfo*>& vec, size_t idx, bool rev = false)
                     : items(vec), index(idx), reverse(rev) {}

              ObjectInfo& operator*() { return *items[index]; }
              ObjectInfo* operator->() { return items[index]; }

              Iterator& operator++() {
                     if (reverse) index--;
                     else index++;
                     return *this;
              }

              Iterator operator++(int) {
                     Iterator tmp = *this;
                     ++(*this);
                     return tmp;
              }

              bool operator==(const Iterator& other) const {
                     return index == other.index;
              }

              bool operator!=(const Iterator& other) const {
                     return index != other.index;
              }
       };

public:
       /**
        * @brief Constructs a new BTree
        * @param order Order of the BTree (default: DEFAULT_BTREE_ORDER)
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
        * @brief Move constructor
        * @param other BTree to move from
        */
       BTree(BTree&& other) noexcept
              : m_Root(std::move(other.m_Root)),
                m_Height(other.m_Height),
                m_Order(other.m_Order),
                m_NumKeys(other.m_NumKeys),
                m_Unique(other.m_Unique)
       {
              other.m_Height = 1;
              other.m_NumKeys = 0;
       }

       ~BTree() {}

       /**
        * @brief Inserts a key-value pair into the tree
        * @param key Key to insert
        * @param ObjID Object ID associated with the key
        * @return true if insertion successful, false if duplicate and unique mode enabled
        * @note Thread-safe: uses unique lock for exclusive write access
        */
       bool            Insert (const keyType key, const long ObjID);

       /**
        * @brief Removes a key-value pair from the tree
        * @param key Key to remove
        * @param ObjID Object ID associated with the key
        * @return true if removal successful, false otherwise
        * @note Thread-safe: uses unique lock for exclusive write access
        */
       bool            Remove (const keyType key, const long ObjID);

       /**
        * @brief Searches for a key in the tree
        * @param key Key to search for
        * @return Object ID if found, -1 otherwise
        * @note Thread-safe: uses shared lock for concurrent read access
        */
       ObjIDType       Search (const keyType key)
       {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              ObjIDType ObjID = -1;
              m_Root.Search(key, ObjID);
              return ObjID;
       }

       /**
        * @brief Returns the number of keys in the tree
        * @return Number of keys
        * @note Thread-safe: uses shared lock for concurrent read access
        */
       size_t            size()  {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              return m_NumKeys;
       }

       /**
        * @brief Returns the height of the tree
        * @return Height of the tree
        * @note Thread-safe: uses shared lock for concurrent read access
        */
       size_t            height() {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              return m_Height;
       }

       /**
        * @brief Returns the order of the tree
        * @return Order of the tree
        * @note Thread-safe: uses shared lock for concurrent read access
        */
       size_t            GetOrder() {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              return m_Order;
       }

       /**
        * @brief Prints the tree structure to an output stream
        * @param os Output stream
        * @note Thread-safe: uses shared lock for concurrent read access
        */
       void            Print (ostream &os)
       {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              m_Root.Print(os);
       }

       /**
        * @brief Applies a function to each element in the tree
        * @tparam Function Function type
        * @tparam Args Variadic template for additional arguments
        * @param func Function to apply to each element
        * @param args Additional arguments to pass to the function
        * @note Thread-safe: uses shared lock for concurrent read access
        */
       template<typename Function, typename... Args>
       void ForEach(Function func, Args const&... args)
       {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              m_Root.ForEach(func, 0, args...);
       }

       /**
        * @brief Finds the first element that satisfies a predicate
        * @tparam Predicate Predicate function type
        * @tparam Args Variadic template for additional arguments
        * @param pred Predicate function
        * @param args Additional arguments to pass to the predicate
        * @return Pointer to ObjectInfo if found, nullptr otherwise
        * @note Thread-safe: uses shared lock for concurrent read access
        */
       template<typename Predicate, typename... Args>
       ObjectInfo* FirstThat(Predicate pred, Args const&... args)
       {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              return m_Root.FirstThat(pred, 0, args...);
       }

       /**
        * @brief Returns an iterator to the beginning of the tree (forward)
        * @return Forward iterator to the first element
        * @note Thread-safe: uses shared lock for concurrent read access
        */
       Iterator begin() {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              collectItems.clear();
              m_Root.ForEach([](ObjectInfo& obj, size_t level, std::vector<ObjectInfo*>* vec) {
                     vec->push_back(&obj);
              }, 0, &collectItems);
              return Iterator(collectItems, 0, false);
       }

       /**
        * @brief Returns an iterator to the end of the tree (forward)
        * @return Forward iterator past the last element
        */
       Iterator end() {
              return Iterator(collectItems, collectItems.size(), false);
       }

       /**
        * @brief Returns a reverse iterator to the beginning (backward)
        * @return Backward iterator to the last element
        * @note Thread-safe: uses shared lock for concurrent read access
        */
       Iterator rbegin() {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              collectItems.clear();
              m_Root.ForEach([](ObjectInfo& obj, size_t level, std::vector<ObjectInfo*>* vec) {
                     vec->push_back(&obj);
              }, 0, &collectItems);
              return Iterator(collectItems, collectItems.size() - 1, true);
       }

       /**
        * @brief Returns a reverse iterator to the end (backward)
        * @return Backward iterator before the first element
        */
       Iterator rend() {
              return Iterator(collectItems, (size_t)-1, true);
       }

protected:
       std::vector<ObjectInfo*> collectItems;
       BTNode          m_Root;
       size_t          m_Height;
       size_t          m_Order;
       size_t          m_NumKeys;
       bool            m_Unique;
       mutable std::shared_mutex m_Mutex;
};

/**
 * @brief Inserts a key-value pair into the tree
 * @param key Key to insert
 * @param ObjID Object ID associated with the key
 * @return true if insertion successful, false if duplicate and unique mode enabled
 */
template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const long ObjID){
       std::unique_lock<std::shared_mutex> lock(m_Mutex);
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

/**
 * @brief Removes a key-value pair from the tree
 * @param key Key to remove
 * @param ObjID Object ID associated with the key
 * @return true if removal successful, false otherwise
 */
template <typename Trait>
bool BTree<Trait>::Remove (const keyType key, const long ObjID)
{
       std::unique_lock<std::shared_mutex> lock(m_Mutex);
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if( error == bt_duplicate || error == bt_nofound )
               return false;
       m_NumKeys--;

       if( error == bt_rootmerged )
               m_Height--;
       return true;
}

/**
 * @brief Overloaded stream insertion operator for BTree
 * @tparam Trait Trait type defining key and object ID types
 * @param os Output stream
 * @param bt BTree to print
 * @return Reference to the output stream
 */
template <typename Trait>
std::ostream& operator<<(std::ostream& os, BTree<Trait>& bt)
{
       bt.Print(os);
       return os;
}

#endif