#ifndef __BTREE_ITERATOR_H__
#define __BTREE_ITERATOR_H__

#include <iterator>
#include <stack>

template <typename Trait>
class BTree;
template <typename Trait>
class CBTreePage;
template <typename Trait>
class BTreeIterator
{
public:
       using iterator_category = std::bidirectional_iterator_tag;
       using value_type = typename CBTreePage<Trait>::ObjectInfo;
       using difference_type = std::ptrdiff_t;
       using pointer = value_type*;
       using reference = value_type&;
       using BTPage = CBTreePage<Trait>;
       using ObjectInfo = typename BTPage::ObjectInfo;

private:
       BTPage* m_CurrentPage;    // Current page being iterated
       size_t m_CurrentIndex;     // Current key index within the page
       std::stack<std::pair<BTPage*, size_t>> m_Stack; // Stack for traversal state

       void findLeftmost(BTPage* page) {
               while (page) {
                       // If page has children, go left
                       if (page->m_SubPages[0]) {
                               m_Stack.push({page, 0});
                               page = page->m_SubPages[0];
                       } else {
                               // Reached leaf - this is the leftmost element
                               m_CurrentPage = page;
                               m_CurrentIndex = 0;
                               return;
                       }
               }
               // Empty tree case
               m_CurrentPage = nullptr;
               m_CurrentIndex = 0;
       }

       void findRightmost(BTPage* page) {
               while (page) {
                       // If page has children, go to the rightmost child
                       if (page->m_SubPages[page->m_KeyCount]) {
                               m_Stack.push({page, page->m_KeyCount});
                               page = page->m_SubPages[page->m_KeyCount];
                       } else {
                               // Reached leaf - this is the rightmost element
                               m_CurrentPage = page;
                               m_CurrentIndex = page->m_KeyCount - 1;
                               return;
                       }
               }
               // Empty tree case
               m_CurrentPage = nullptr;
               m_CurrentIndex = 0;
       }

public:

       BTreeIterator() : m_CurrentPage(nullptr), m_CurrentIndex(0) {}

       explicit BTreeIterator(BTPage* root)
               : m_CurrentPage(nullptr), m_CurrentIndex(0) {
               if (root && root->m_KeyCount > 0) {
                       findLeftmost(root);
               }
       }

       static BTreeIterator end() { return BTreeIterator(); }
       reference operator*() const { return m_CurrentPage->m_Keys[m_CurrentIndex]; }
       pointer operator->() const { return &(m_CurrentPage->m_Keys[m_CurrentIndex]); }

       BTreeIterator& operator++() {
               if (!m_CurrentPage) {
                       return *this; // Already at end
               }

               // Check if current position has a right child (for internal nodes)
               if (m_CurrentPage->m_SubPages[m_CurrentIndex + 1]) {
                       // Go down to leftmost of right subtree
                       BTPage* rightChild = m_CurrentPage->m_SubPages[m_CurrentIndex + 1];
                       m_Stack.push({m_CurrentPage, m_CurrentIndex + 1});
                       findLeftmost(rightChild);
                       return *this;
               }

               // Try to advance to next key in current page
               m_CurrentIndex++;
               if (m_CurrentIndex < m_CurrentPage->m_KeyCount) {
                       // There are more keys in current page
                       return *this;
               }

               // No more keys in current page - go up to parent
               if (!m_Stack.empty()) {
                       auto [parentPage, parentIndex] = m_Stack.top();
                       m_Stack.pop();
                       m_CurrentPage = parentPage;
                       m_CurrentIndex = parentIndex;
                       return *this;
               }

               // Reached end of iteration
               m_CurrentPage = nullptr;
               m_CurrentIndex = 0;
               return *this;
       }

       BTreeIterator operator++(int) {
               BTreeIterator temp = *this;
               ++(*this);
               return temp;
       }

       BTreeIterator& operator--() {
               if (!m_CurrentPage) {
                       return *this; // Already at begin() or invalid
               }

               // Check if current position has a left child (for internal nodes)
               if (m_CurrentPage->m_SubPages[m_CurrentIndex]) {
                       // Go down to rightmost of left subtree
                       BTPage* leftChild = m_CurrentPage->m_SubPages[m_CurrentIndex];
                       m_Stack.push({m_CurrentPage, m_CurrentIndex});
                       findRightmost(leftChild);
                       return *this;
               }

               // Try to move to previous key in current page
               if (m_CurrentIndex > 0) {
                       m_CurrentIndex--;
                       return *this;
               }

               // No more keys in current page - go up to parent
               if (!m_Stack.empty()) {
                       auto [parentPage, parentIndex] = m_Stack.top();
                       m_Stack.pop();
                       m_CurrentPage = parentPage;
                       if (parentIndex > 0) {
                               m_CurrentIndex = parentIndex - 1;
                       } else {
                               m_CurrentIndex = 0;
                       }
                       return *this;
               }

               // Reached beginning of iteration
               m_CurrentPage = nullptr;
               m_CurrentIndex = 0;
               return *this;
       }

       BTreeIterator operator--(int) {
               BTreeIterator temp = *this;
               --(*this);
               return temp;
       }

       bool operator==(const BTreeIterator& other) const {
               return m_CurrentPage == other.m_CurrentPage &&
                      m_CurrentIndex == other.m_CurrentIndex;
       }

       bool operator!=(const BTreeIterator& other) const {
               return !(*this == other);
       }

       // Friend class declarations for access to private members
       friend class BTree<Trait>;
       friend class CBTreePage<Trait>;
};

#endif // __BTREE_ITERATOR_H__