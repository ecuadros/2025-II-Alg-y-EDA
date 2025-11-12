#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <stack>
#include <memory>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5;

/**
 * @brief Trait structure for BTree key and object ID types
 * @tparam _keyType Type of the key
 * @tparam _ObjIDType Type of the object ID
 * @tparam _Compare Comparator type for key comparison (default: DefaultCompare)
 */
template <typename _keyType, typename _ObjIDType, typename _Compare = DefaultCompare<_keyType>>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       using Compare = _Compare;

       /**
        * @brief Compares two keys for equality
        * @param a First key
        * @param b Second key
        * @return true if keys are equal, false otherwise
        */
       static bool isEqual(const keyType& a, const keyType& b) {
              return Compare::compare(a, b) == 0;
       }

       /**
        * @brief Compares two keys
        * @param a First key
        * @param b Second key
        * @return -1 if a < b, 0 if a == b, 1 if a > b
        */
       static int compare(const keyType& a, const keyType& b) {
              return Compare::compare(a, b);
       }
};

/**
 * @brief Thread-safe B-Tree implementation
 * @tparam Trait Trait type defining key and object ID types
 * @tparam Compare Comparator type for key comparison (default: std::less)
 *
 * This implementation provides:
 * - Concurrent read operations using shared locks
 * - Exclusive write operations using unique locks
 * - Forward and backward iterators
 * - Move semantics support
 * - Custom comparison function support
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
        * Maintains a shared lock during its lifetime for thread-safety.
        */
       class Iterator {
       private:
              BTree* m_Tree;
              BTNode* m_CurrentNode;
              size_t m_CurrentIndex;
              std::stack<std::pair<BTNode*, size_t>> m_PathStack;
              std::unique_ptr<std::shared_lock<std::shared_mutex>> m_Lock;
              bool m_Reverse;

              void moveToNext();
              void moveToPrev();

       public:
              // Constructor for valid iterator
              Iterator(BTree* tree, BTNode* node, size_t index, bool reverse, std::unique_ptr<std::shared_lock<std::shared_mutex>> lock)
                     : m_Tree(tree), m_CurrentNode(node), m_CurrentIndex(index), m_Lock(std::move(lock)), m_Reverse(reverse) {}

              // Constructor for end iterator (no lock)
              Iterator(BTree* tree, bool reverse)
                     : m_Tree(tree), m_CurrentNode(nullptr), m_CurrentIndex(0), m_Lock(nullptr), m_Reverse(reverse) {}

              // Copy constructor (shares the iteration state but creates new lock)
              Iterator(const Iterator& other) = delete;

              // Move constructor
              Iterator(Iterator&& other) noexcept = default;

              ObjectInfo& operator*() { return m_CurrentNode->m_Keys[m_CurrentIndex]; }
              ObjectInfo* operator->() { return &m_CurrentNode->m_Keys[m_CurrentIndex]; }

              Iterator& operator++() {
                     if (m_Reverse) moveToPrev();
                     else moveToNext();
                     return *this;
              }

              Iterator operator++(int) {
                     Iterator tmp = std::move(*this);
                     ++(*this);
                     return tmp;
              }

              bool operator==(const Iterator& other) const {
                     return m_CurrentNode == other.m_CurrentNode && m_CurrentIndex == other.m_CurrentIndex;
              }

              bool operator!=(const Iterator& other) const {
                     return !(*this == other);
              }
       };

public:
       /**
        * @brief Constructs a new BTree
        * @param order Order of the BTree (default: DEFAULT_BTREE_ORDER)
        * @param unique If true, duplicate keys are not allowed
        */
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
              : m_Root(2 * order  + 1, unique),
                m_Height(1),
                m_Order(order),
                m_NumKeys(0),
                m_Unique(unique)
       {
              m_Root.SetMaxKeysForChilds(order);
       }

       /**
        * @brief Move constructor
        * @param other BTree to move from
        */
       BTree(BTree&& other) noexcept
              : m_Root(2 * other.m_Order + 1, other.m_Unique),
                m_Height(1),
                m_Order(other.m_Order),
                m_NumKeys(0),
                m_Unique(other.m_Unique)
       {
              std::lock_guard<std::shared_mutex> lock(other.m_Mutex);
              m_Root.SetMaxKeysForChilds(other.m_Order);

              // Swap the roots
              std::swap(m_Root, other.m_Root);

              m_Height = other.m_Height;
              m_NumKeys = other.m_NumKeys;

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
       void            Print (ostream &os) const
       {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              m_Root.Print(os);
       }

       /**
        * @brief Writes the tree structure to an output stream
        * @param os Output stream
        * @return Reference to the output stream
        * @note Thread-safe: uses shared lock for concurrent read access
        */
       std::ostream&   Write (std::ostream &os);

       /**
        * @brief Reads the tree structure from an input stream
        * @param is Input stream
        * @return Reference to the input stream
        * @note Thread-safe: uses unique lock for exclusive write access
        */
       std::istream&   Read (std::istream &is);

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
        * @note Thread-safe: maintains shared lock during iteration lifetime
        */
       Iterator begin() {
              auto lock = std::make_unique<std::shared_lock<std::shared_mutex>>(m_Mutex);
              BTNode* pNode = &m_Root;

              if (pNode->m_KeyCount == 0) {
                     return Iterator(this, false); // Empty tree, return end()
              }

              // Navigate to the leftmost (minimum) node
              while (pNode->m_SubPages[0] != nullptr) {
                     pNode = pNode->m_SubPages[0];
              }

              return Iterator(this, pNode, 0, false, std::move(lock));
       }

       /**
        * @brief Returns an iterator to the end of the tree (forward)
        * @return Forward iterator past the last element
        */
       Iterator end() {
              return Iterator(this, false);
       }

       /**
        * @brief Returns a reverse iterator to the beginning (backward)
        * @return Backward iterator to the last element
        * @note Thread-safe: maintains shared lock during iteration lifetime
        */
       Iterator rbegin() {
              auto lock = std::make_unique<std::shared_lock<std::shared_mutex>>(m_Mutex);
              BTNode* pNode = &m_Root;

              if (pNode->m_KeyCount == 0) {
                     return Iterator(this, true); // Empty tree, return rend()
              }

              // Navigate to the rightmost (maximum) node
              while (pNode->m_SubPages[pNode->m_KeyCount] != nullptr) {
                     pNode = pNode->m_SubPages[pNode->m_KeyCount];
              }

              return Iterator(this, pNode, pNode->m_KeyCount - 1, true, std::move(lock));
       }

       /**
        * @brief Returns a reverse iterator to the end (backward)
        * @return Backward iterator before the first element
        */
       Iterator rend() {
              return Iterator(this, true);
       }

protected:
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
 * @brief Writes the tree structure to an output stream
 * @param os Output stream
 * @return Reference to the output stream
 */
template <typename Trait>
std::ostream& BTree<Trait>::Write(std::ostream& os)
{
       std::shared_lock<std::shared_mutex> lock(m_Mutex);
       os << m_Order << " " << m_Unique << " " << m_Height << " " << m_NumKeys << "\n";
       m_Root.Write(os);
       return os;
}

/**
 * @brief Reads the tree structure from an input stream
 * @param is Input stream
 * @return Reference to the input stream
 */
template <typename Trait>
std::istream& BTree<Trait>::Read(std::istream& is)
{
       std::unique_lock<std::shared_mutex> lock(m_Mutex);

       size_t order;
       bool unique;
       is >> order >> unique >> m_Height >> m_NumKeys;

       // Note: order and unique should match the tree's configuration
       // Reading them but not changing the tree structure

       m_Root.Read(is);
       return is;
}

/**
 * @brief Moves iterator to next element in in-order traversal
 */
template <typename Trait>
void BTree<Trait>::Iterator::moveToNext()
{
       if (!m_CurrentNode) return; // Already at end

       // If there's a right child after current key, go to its minimum
       if (m_CurrentNode->m_SubPages[m_CurrentIndex + 1] != nullptr) {
              m_PathStack.push({m_CurrentNode, m_CurrentIndex});
              m_CurrentNode = m_CurrentNode->m_SubPages[m_CurrentIndex + 1];

              // Navigate to leftmost node
              while (m_CurrentNode->m_SubPages[0] != nullptr) {
                     m_PathStack.push({m_CurrentNode, 0});
                     m_CurrentNode = m_CurrentNode->m_SubPages[0];
              }
              m_CurrentIndex = 0;
       }
       // Otherwise, move to next key in current node
       else if (m_CurrentIndex + 1 < m_CurrentNode->m_KeyCount) {
              m_CurrentIndex++;
       }
       // Otherwise, go up to parent
       else {
              if (m_PathStack.empty()) {
                     // Reached end
                     m_CurrentNode = nullptr;
                     m_CurrentIndex = 0;
              } else {
                     auto parent = m_PathStack.top();
                     m_PathStack.pop();
                     m_CurrentNode = parent.first;
                     m_CurrentIndex = parent.second;
              }
       }
}

/**
 * @brief Moves iterator to previous element in reverse in-order traversal
 */
template <typename Trait>
void BTree<Trait>::Iterator::moveToPrev()
{
       if (!m_CurrentNode) return; // Already at rend

       // If there's a left child before current key, go to its maximum
       if (m_CurrentNode->m_SubPages[m_CurrentIndex] != nullptr) {
              m_PathStack.push({m_CurrentNode, m_CurrentIndex});
              m_CurrentNode = m_CurrentNode->m_SubPages[m_CurrentIndex];

              // Navigate to rightmost node
              while (m_CurrentNode->m_SubPages[m_CurrentNode->m_KeyCount] != nullptr) {
                     m_PathStack.push({m_CurrentNode, m_CurrentNode->m_KeyCount});
                     m_CurrentNode = m_CurrentNode->m_SubPages[m_CurrentNode->m_KeyCount];
              }
              m_CurrentIndex = m_CurrentNode->m_KeyCount - 1;
       }
       // Otherwise, move to previous key in current node
       else if (m_CurrentIndex > 0) {
              m_CurrentIndex--;
       }
       // Otherwise, go up to parent
       else {
              if (m_PathStack.empty()) {
                     // Reached rend
                     m_CurrentNode = nullptr;
                     m_CurrentIndex = 0;
              } else {
                     auto parent = m_PathStack.top();
                     m_PathStack.pop();
                     m_CurrentNode = parent.first;
                     m_CurrentIndex = parent.second;
              }
       }
}

/**
 * @brief Overloaded stream insertion operator for BTree
 * @tparam Trait Trait type defining key and object ID types
 * @param os Output stream
 * @param bt BTree to print
 * @return Reference to the output stream
 */
template <typename Trait>
std::ostream& operator<<(std::ostream& os, const BTree<Trait>& bt)
{
       bt.Print(os);
       return os;
}

#endif