/**
 * @file btreepage.h
 * @brief B-Tree node (page) implementation
 * 
 * This file contains the implementation of B-Tree nodes (pages) including
 * key storage, child pointers, insertion/removal operations, and utility functions.
 */

#ifndef __CBTreePage_H__
#define __CBTreePage_H__

#include <vector>
#include <assert.h>
#include <functional>
#include <utility>
#include <ostream>

// TODO: #1 Crear una function para agregarla al demo.cpp ( no trivial )
// TODO: #2 Agregarle un Trait (prueba git) ( no trivial )
// TODO: #3 crear un iterator ( no trivial )
//       Sugerencia: Tarea1 cada pagina debe tener un puntero al padre primero ( no trivial )
// TODO: #4 integrarlo al recorrer ( no trivial )


template <typename Trait>
class BTree;

template <typename Trait>
class BTreeIterator;

using namespace std;

/**
 * @brief Error codes for B-Tree operations
 */
enum bt_ErrorCode {bt_ok, bt_overflow, bt_underflow, bt_duplicate, bt_nofound, bt_rootmerged};

/**
 * @brief Binary search in a container with customizable comparison function
 * 
 * Performs binary search on a sorted container using a provided comparator.
 * 
 * @tparam Container Container type (e.g., vector)
 * @tparam ObjType Type of objects being compared
 * @tparam CompareF Comparison function type (defaults to std::less<ObjType>)
 * @param container Container to search in
 * @param first First index of search range
 * @param last Last index of search range (exclusive)
 * @param object Object to search for
 * @param comp Comparison function object
 * @return Index where object is found or should be inserted
 */
template <typename Container, typename ObjType, typename CompareF = std::less<ObjType>> //std:less by default
size_t binary_search(Container& container, size_t first, size_t last, ObjType &object, CompareF comp = CompareF() )
{
       if( first >= last )
               return first;
       while( first < last )
       {
               size_t mid = (first+last)/2;
               if( !comp(object, container[mid]) && !comp(container[mid], object) ) // By default:  object == (ObjType)container[mid]   !x<y, !y<x -> x=y
                       return mid;
               if( comp(container[mid],object)  ) // By default: object > (ObjType)container[mid ]
                       first = mid+1;
               else
                       last  = mid;
       }
       if( object <= (ObjType)container[first] )
               return first;
       return last;
}

/**
 * @brief Insert an object at a specific position in a container
 * 
 * Shifts elements to the right and inserts the object at the specified position.
 * 
 * @tparam Container Container type
 * @tparam ObjType Object type
 * @param container Container to insert into
 * @param object Object to insert
 * @param pos Position to insert at
 */
// Error al poner size_t
// Posible motivo: El i está disminuyendo
template <typename Container, typename ObjType>
void insert_at(Container& container, ObjType object, int pos)
{
        // TODO: #5 replace int, long by types such as size_t
       size_t size = container.size();
       for(int i = size-2 ; i >= pos ; i--)
               container[i+1] = container[i];
       container[pos] =  object;	
}

/**
 * @brief Remove an element at a specific position from a container
 * 
 * Shifts elements to the left to fill the gap left by removal.
 * 
 * @tparam Container Container type
 * @param container Container to remove from
 * @param pos Position to remove from
 */
template <typename Container>
void remove(Container& container, size_t pos)
{
       size_t size = container.size();
       for(auto i = pos+1 ; i < size ; i++)
           container[i-1] = container[i];
}

/**
 * @brief Structure holding key-ObjID pair information
 * 
 * @tparam keyType Type of the key
 * @tparam ObjIDType Type of the object identifier
 */
template <typename keyType, typename ObjIDType>
struct tagObjectInfo
{
       keyType                 key;        /**< The key value */
       ObjIDType               ObjID;      /**< Associated object identifier */
       size_t                  UseCounter; /**< Usage counter */
       
       /**
        * @brief Construct from key and ObjID
        * @param _key Key value
        * @param _ObjID Object identifier
        */
       tagObjectInfo(const keyType     &_key, ObjIDType _ObjID)
               : key(_key), ObjID(_ObjID), UseCounter(0) {}
       
       /**
        * @brief Copy constructor
        * @param objInfo Source ObjectInfo
        */
       tagObjectInfo(const tagObjectInfo &objInfo)
               : key(objInfo.key), ObjID(objInfo.ObjID), UseCounter(0) {}
       
       /**
        * @brief Default constructor
        */
       tagObjectInfo()                          {}
       
       /**
        * @brief Implicit conversion to keyType
        * @return Key value
        */
       operator keyType                         ()     { return key; }
       
       /**
        * @brief Get use counter value
        * @return Use counter
        */
       size_t                    GetUseCounter() { return UseCounter;    }
};

/**
 * @brief B-Tree page (node) class
 * 
 * Represents a single node in the B-Tree, containing keys and child pointers.
 * Supports insertion, removal, search, and tree rebalancing operations.
 * 
 * @tparam Trait Type trait containing keyType, ObjIDType, and CompareF definitions
 */
template <typename Trait>
class CBTreePage //: public SimpleIndex <keyType>
// this is the in-memory version of the CBTreePage
{
       friend class BTree<Trait>;
       friend class BTreeIterator<Trait>; // Forward iterator
       typedef typename Trait::keyType  keyType;   /**< Key type from trait */
       typedef typename Trait::ObjIDType  ObjIDType; /**< Object ID type from trait */
       typedef typename Trait::CompareF CompareF;   /**< Comparison function type from trait */

       typedef CBTreePage<Trait>    BTPage;         /**< Alias for this class type */
       typedef tagObjectInfo<keyType, ObjIDType> ObjectInfo; /**< Alias for ObjectInfo type */

 public:
       /**
        * @brief Construct a new CBTreePage
        * @param maxKeys Maximum number of keys this page can hold
        * @param unique If true, duplicate keys are not allowed
        */
       CBTreePage(size_t maxKeys, bool unique = true);
       
       /**
        * @brief Destructor
        */
       virtual ~CBTreePage();

       /**
        * @brief Move constructor - transfers ownership of page resources
        * @param btree Source page to move from
        */
       CBTreePage(CBTreePage&& btree)              
       {
                
                // transfering page resources 
                m_MinKeys = btree.m_MinKeys;
                m_MaxKeys = btree.m_MaxKeys;
                m_MaxKeysForChilds = btree.m_MaxKeysForChilds;
                m_Unique = btree.m_Unique;
                m_isRoot = btree.m_isRoot;
                m_Keys = std::move(btree.m_Keys);
                m_SubPages = std::move(btree.m_SubPages);
                compare = std::move(btree.compare);
                m_KeyCount = btree.m_KeyCount;
                m_Parent = std::move(btree.m_Parent);

                // Leave source in a valid empty state
                btree.m_KeyCount = 0;
                btree.m_Keys.clear();
                btree.m_SubPages.clear();
                btree.m_MinKeys = 0;
                btree.m_MaxKeys = 0;
                btree.m_MaxKeysForChilds = 0;
                btree.m_Unique = false;
                btree.m_isRoot = false;
       }
       
       /**
        * @brief Move assignment operator
        * @param btree Source page to move from
        * @return Reference to this object
        */
       CBTreePage& operator=(CBTreePage&& btree) noexcept 
       {
              if (this != &btree)
              {
                     Reset(); // Clean up current resources first

                     m_MinKeys = btree.m_MinKeys;
                     m_MaxKeys = btree.m_MaxKeys;
                     m_MaxKeysForChilds = btree.m_MaxKeysForChilds;
                     m_Unique = btree.m_Unique;
                     m_isRoot = btree.m_isRoot;
                     m_Keys = std::move(btree.m_Keys);
                     m_SubPages = std::move(btree.m_SubPages);
                     compare = std::move(btree.compare);
                     m_KeyCount = btree.m_KeyCount;
                     m_Parent = std::exchange(btree.m_Parent, nullptr);

                     btree.m_KeyCount = 0; // Leave source in valid state
              }
              return *this;
       }

       /**
        * @brief Insert a key-ObjID pair into this page
        * @param key Key to insert
        * @param ObjID Object identifier associated with the key
        * @return Error code indicating success or failure (bt_ok, bt_overflow, bt_duplicate)
        */
       bt_ErrorCode    Insert (const keyType &key, const ObjIDType ObjID);
       
       /**
        * @brief Remove a key-ObjID pair from this page
        * @param key Key to remove
        * @param ObjID Object identifier to remove
        * @return Error code indicating success or failure
        */
       bt_ErrorCode    Remove (const keyType &key, const ObjIDType ObjID);
       
       /**
        * @brief Search for a key in this page and its children
        * @param key Key to search for
        * @param ObjID Output parameter for found object identifier
        * @return true if found, false otherwise
        */
       bool            Search (const keyType &key, ObjIDType &ObjID);
       
       /**
        * @brief Print page structure to output stream
        * @param os Output stream
        */
       void            Print  (ostream &os);

       // TODO: #6 change by Invoke
       // TODO: #7 ForEach must be a template inside this template
       /**
        * @brief Apply a function to each key-ObjID pair in this page and its children
        * 
        * @tparam Func Function type (callable object)
        * @tparam Args Variadic additional argument types
        * @param func Function to apply to each element
        * @param level Current tree level (for hierarchical operations)
        * @param args Additional arguments to forward to func
        */
       template <typename Func, typename... Args>
       void            ForEach(Func&& func, size_t level, Args&&... args);

       // TODO: #8 You may reduce these two function by using Invoke
       /**
        * @brief Find first element in this page matching a predicate
        * 
        * @tparam Func Predicate function type
        * @tparam Args Variadic additional argument types
        * @param func Predicate function to test each element
        * @param level Current tree level
        * @param args Additional arguments to forward to func
        * @return Pointer to first matching ObjectInfo, or nullptr if none found
        */
       template <typename Func, typename... Args>
       ObjectInfo*     FirstThat(Func&& func, size_t level, Args&&... args);

protected:
       // TODO: #9 change by size_t
       size_t  m_MinKeys;  /**< Minimum number of keys in a node */
       size_t  m_MaxKeys,  /**< Maximum number of keys in a node */
                m_MaxKeysForChilds; /**< Just to distinguish the root */
       bool m_Unique;      /**< Accept elements only once? */
       bool m_isRoot;      /**< Is this the root node? */
       //size_t           NextNode; // address of next node at same level
       //size_t RecAddr; // address of this node in the BTree file
       vector<ObjectInfo> m_Keys;     /**< Keys stored in this page */
       vector<BTPage *>m_SubPages;    /**< Child page pointers */
       CompareF compare;              /**< Comparison function object */
       BTPage* m_Parent;
       
       // TODO: #10 size_t
       size_t  m_KeyCount;  /**< Current number of keys in this page */
       
       /**
        * @brief Initialize page data structures
        */
       void  Create();
       
       /**
        * @brief Clean up page resources
        */
       void  Reset ();
       
       /**
        * @brief Destroy this page and delete it
        */
       void  Destroy () {   Reset(); delete this;}
       
       /**
        * @brief Clear all keys from this page
        */
       void  clear ();

       /**
        * @brief Attempt to redistribute keys with one brother
        * @param pos Position of child experiencing underflow
        * @return true if redistribution succeeded
        */
       bool  RedistributeWith1Brother   (size_t &pos);
       
       /**
        * @brief Attempt to redistribute keys with two brothers
        * @param pos Position of child experiencing underflow
        * @return true if redistribution succeeded
        */
       bool  RedistributeWith2Brothers   (size_t pos);
       
       /**
        * @brief Redistribute keys from right to left sibling
        * @param pos Position of right sibling
        */
       void  RedistributeR2L (size_t pos);
       
       /**
        * @brief Redistribute keys from left to right sibling
        * @param pos Position of left sibling
        */
       void  RedistributeL2R (size_t pos);

       /**
        * @brief Handle underflow condition in a child
        * @param pos Position of child with underflow
        * @return true if underflow was resolved
        */
       bool    TreatUnderflow  (size_t &pos)
       {       return RedistributeWith1Brother(pos) || RedistributeWith2Brothers(pos);}

       /**
        * @brief Merge a child with its sibling
        * @param pos Position of child to merge
        * @return Error code indicating merge result
        */
       bt_ErrorCode    Merge  (size_t pos);
       
       /**
        * @brief Merge root with its children
        * @return bt_rootmerged if successful
        */
       bt_ErrorCode    MergeRoot ();
       
       /**
        * @brief Split a child page that has overflowed
        * @param pos Position of child to split
        */
       void  SplitChild (size_t pos);

       /**
        * @brief Get first ObjectInfo in this page
        * @return Reference to first ObjectInfo
        */
       ObjectInfo &GetFirstObjectInfo();

       /**
        * @brief Check if page has overflowed
        * @return true if overflow condition exists
        */
       bool Overflow()  { return m_KeyCount > m_MaxKeys; }
       
       /**
        * @brief Check if page has underflowed
        * @return true if underflow condition exists
        */
       bool Underflow() { return m_KeyCount < MinNumberOfKeys(); }
       
       /**
        * @brief Check if page is full
        * @return true if page is full
        */
       bool IsFull()    { return m_KeyCount >= m_MaxKeys; }

       /**
        * @brief Get minimum number of keys required for this page
        * @return Minimum key count
        */
       size_t  MinNumberOfKeys()  { return 2*m_MaxKeys/3.0; }
       
       /**
        * @brief Get number of free cells in this page
        * @return Number of available key slots
        */
       size_t  GetFreeCells()  { return m_MaxKeys - m_KeyCount; }
       
       /**
        * @brief Get reference to key count
        * @return Reference to m_KeyCount
        */
       size_t& NumberOfKeys()  { return m_KeyCount; }
       
       /**
        * @brief Get current number of keys
        * @return Key count
        */
       size_t  GetNumberOfKeys()  { return m_KeyCount; }
       
       /**
        * @brief Check if this page is the root
        * @return true if root
        */
       bool IsRoot()  { return m_MaxKeysForChilds != m_MaxKeys; }
       
       /**
        * @brief Set maximum keys for child pages
        * @param orderforchilds Order value for child pages
        */
       void SetMaxKeysForChilds(size_t orderforchilds)
       {        m_MaxKeysForChilds = orderforchilds;       }
       
       /**
        * @brief Get free cells available in left sibling
        * @param pos Position of current child
        * @return Number of free cells in left sibling
        */
       size_t GetFreeCellsOnLeft(size_t pos)
       {        if( pos > 0 )                                   // there is some page on left ?
                        return m_SubPages[pos-1]->GetFreeCells();
                return 0;
       }
       
       /**
        * @brief Get free cells available in right sibling
        * @param pos Position of current child
        * @return Number of free cells in right sibling
        */
       size_t GetFreeCellsOnRight(size_t pos)
       {    if( pos < GetNumberOfKeys() )   // there is some page on right ?
                return m_SubPages[pos+1]->GetFreeCells();
            return 0;
       }

private:
       /**
        * @brief Split root into multiple pages
        * @return true if split succeeded
        */
       bool SplitRoot();
       
       /**
        * @brief Split a page into three child pages
        * @param tmpKeys Temporary key storage
        * @param SubPages Temporary subpage storage
        * @param pChild1 First resulting child
        * @param pChild2 Second resulting child
        * @param pChild3 Third resulting child
        * @param oi1 First ObjectInfo to promote
        * @param oi2 Second ObjectInfo to promote
        */
       void SplitPageInto3(vector<ObjectInfo>   & tmpKeys,
                                               vector<BTPage *>  & SubPages,
                                               BTPage           *& pChild1,
                                               BTPage           *& pChild2,
                                               BTPage           *& pChild3,
                                               ObjectInfo        & oi1,
                                               ObjectInfo        & oi2);
       
       /**
        * @brief Move page contents to temporary storage
        * @param pChildPage Page to move
        * @param tmpKeys Temporary key storage
        * @param tmpSubPages Temporary subpage storage
        */
       void MovePage(BTPage *  pChildPage,vector<ObjectInfo> & tmpKeys,vector<BTPage *> & tmpSubPages);
};

template <typename Trait>
CBTreePage<Trait>:: CBTreePage(size_t maxKeys, bool unique)
                               : m_MaxKeys(maxKeys), m_Unique(unique),m_Parent(nullptr), m_KeyCount(0)
{
       Create();
       SetMaxKeysForChilds(m_MaxKeys);
}

template <typename Trait>
CBTreePage<Trait>::~CBTreePage()
{
       Reset();
}

template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::Insert(const keyType& key, const ObjIDType ObjID){
       size_t pos = binary_search(m_Keys, 0, m_KeyCount, key, compare);
       bt_ErrorCode error = bt_ok;

       if( pos < m_KeyCount && (keyType)m_Keys[pos] == key && m_Unique)
               return bt_duplicate; // this key is duplicate

       if( !m_SubPages[pos] ){ // this is a leave
               ::insert_at(m_Keys, ObjectInfo(key, ObjID), pos);
               m_KeyCount++;
               if( Overflow() )
                       return bt_overflow;
               return bt_ok;
       }
       else{
               // recursive insertion
               error = m_SubPages[pos]->Insert(key, ObjID);
               if( error == bt_overflow ){
                       if( !RedistributeWith1Brother(pos) )
                               SplitChild(pos);
                       if( Overflow() )  // Propagate overflow
                               return bt_overflow;
                       return bt_ok;
               }
       }
       if( Overflow() ) // node overflow
               return bt_overflow;
       return bt_ok;
}

template <typename Trait>
bool CBTreePage<Trait>::RedistributeWith1Brother(size_t &pos)
{
       if( m_SubPages[pos]->Underflow() )
       {
               size_t NumberOfKeyOnLeft = 0,
                       NumberOfKeyOnRight = 0;
               // is this the first element or there are more elements on right brother
               if( pos > 0 )
                       NumberOfKeyOnLeft = m_SubPages[pos-1]->NumberOfKeys();
               if( pos < NumberOfKeys() )
                       NumberOfKeyOnRight = m_SubPages[pos+1]->NumberOfKeys();

               if( NumberOfKeyOnLeft > NumberOfKeyOnRight )
                       if( m_SubPages[pos-1]->NumberOfKeys() > m_SubPages[pos-1]->MinNumberOfKeys() )
                               RedistributeL2R(pos-1); // bring elements from left brother
                       else
                               if( pos == NumberOfKeys() )
                                       return (--pos, false);
                               else
                                       return false;
               else //NumberOfKeyOnLeft < NumberOfKeyOnRight )
                       if( m_SubPages[pos+1]->NumberOfKeys() > m_SubPages[pos+1]->MinNumberOfKeys() )
                               RedistributeR2L(pos+1); // bring elements from right brother
                       else
                               if( pos == 0 )
                                       return (++pos, false);
                               else
                                       return false;
       }
       else // it is due to overflow
       {
               size_t FreeCellsOnLeft = GetFreeCellsOnLeft(pos),   // Free Cells On Left
               fcor = GetFreeCellsOnRight(pos);  // Free Cells On Right

               if( !FreeCellsOnLeft && !fcor && m_SubPages[pos]->IsFull() )
                       return false;
               if( FreeCellsOnLeft > fcor ) // There is more space on left
                       RedistributeR2L(pos);
               else
                       RedistributeL2R(pos);

       }
       return true;
}

/** RedistributeWith2Brothers function
   it considers two brothers m_SubPages[pos-1] && m_SubPages[pos+1]
   if it fails the only way is merge !
**/
template <typename Trait>
bool CBTreePage<Trait>::RedistributeWith2Brothers(size_t pos)
{
       assert( pos > 0 && pos < NumberOfKeys()  );
       assert( m_SubPages[pos-1] != 0 && m_SubPages[pos] != 0 && m_SubPages[pos+1] != 0 );
       assert( m_SubPages[pos-1]->Underflow() ||
                       m_SubPages[ pos ]->Underflow() ||
                       m_SubPages[pos+1]->Underflow() );

       if( m_SubPages[pos-1]->Underflow() )
       {       // Rotate R2L
               RedistributeR2L(pos+1);
               RedistributeR2L(pos);
               if( m_SubPages[pos-1]->Underflow() )
                       return false;
       }
       else if( m_SubPages[pos+1]->Underflow() )
       {       // Rotate L2R
               RedistributeL2R(pos-1);
               RedistributeL2R(pos);
               if( m_SubPages[pos+1]->Underflow() )
                       return false;
       }
       else // The problem is exactly at pos !
       {
               // Rotate L2R
               RedistributeL2R(pos-1);
               RedistributeR2L(pos+1);
               if( m_SubPages[pos]->Underflow() )
                       return false;
       }
       return true;
}

template <typename Trait>
void CBTreePage<Trait>::RedistributeR2L(size_t pos)  
{
       BTPage  *pSource = m_SubPages[ pos ],
               *pTarget = m_SubPages[pos-1];

       while(pSource->GetNumberOfKeys() > pSource->MinNumberOfKeys() &&
                 pTarget->GetNumberOfKeys() < pSource->GetNumberOfKeys() )
       {
               // Move from this page to the down-left page \/
               ::insert_at(pTarget->m_Keys, m_Keys[pos-1], pTarget->NumberOfKeys()++);
               // Move the pointer leftest pointer to the rightest position
               ::insert_at(pTarget->m_SubPages, pSource->m_SubPages[0], pTarget->NumberOfKeys());

               // Move the leftest element to the root
               m_Keys[pos-1] = pSource->m_Keys[0];

               // Remove the leftest element from rigth page
               ::remove(pSource->m_Keys    , 0);
               ::remove(pSource->m_SubPages, 0);
               pSource->NumberOfKeys()--;
       }
}

template <typename Trait>
void CBTreePage<Trait>::RedistributeL2R(size_t pos)
{
       BTPage  *pSource = m_SubPages[pos],
                       *pTarget = m_SubPages[pos+1];
       while(pSource->GetNumberOfKeys() > pSource->MinNumberOfKeys() &&
                 pTarget->GetNumberOfKeys() < pSource->GetNumberOfKeys() )
       {
               // Move from this page to the down-RIGHT page \/
               ::insert_at(pTarget->m_Keys, m_Keys[pos], 0);
               // Move the pointer rightest pointer to the leftest position
               ::insert_at(pTarget->m_SubPages, pSource->m_SubPages[pSource->NumberOfKeys()], 0);
               pTarget->NumberOfKeys()++;

               // Move the rightest element to the root
               m_Keys[pos] = pSource->m_Keys[pSource->NumberOfKeys()-1];

               // Remove the leftest element from rigth page
               // it is not necessary erase because m_KeyCount controls
               pSource->NumberOfKeys()--;
       }
}

template <typename Trait>
void CBTreePage<Trait>::SplitChild(size_t pos)
{
       // FIRST: deciding the second page to split
       BTPage  *pChild1 = 0, *pChild2 = 0;
       if( pos > 0 )                                   // is left page full ?
               if( m_SubPages[pos-1]->IsFull() )
               {
                       pChild1 = m_SubPages[pos-1];
                       pChild2 = m_SubPages[pos--];
               }
       if( pos < GetNumberOfKeys() )   // is right page full ?
               if( m_SubPages[pos+1]->IsFull() )
               {
                       pChild1 = m_SubPages[pos];
                       pChild2 = m_SubPages[pos+1];
               }
       size_t nKeys = pChild1->GetNumberOfKeys() + pChild2->GetNumberOfKeys() + 1;

       // SECOND: copy both pages to a temporal one
       // Create two tmp vector
       vector<ObjectInfo> tmpKeys;
       //tmpKeys.resize(nKeys);
       vector<BTPage *>   tmpSubPages;
       //tmpKeys.resize(nKeys+1);

       // copy from left child
       MovePage(pChild1, tmpKeys, tmpSubPages);
       // copy a key from parent
       tmpKeys    .push_back(m_Keys[pos]);

       // copy from right child
       MovePage(pChild2, tmpKeys, tmpSubPages);

       BTPage *pChild3 = 0;
       ObjectInfo oi1, oi2;
       SplitPageInto3(tmpKeys, tmpSubPages, pChild1, pChild2, pChild3, oi1, oi2);

       // copy the first element to the root
       m_Keys    [pos] = oi1;
       m_SubPages[pos] = pChild1;
       pChild1->m_Parent = this; 

       // copy the second element to the root
       ::insert_at(m_Keys, oi2, pos+1);
       ::insert_at(m_SubPages, pChild2, pos+1);
       NumberOfKeys()++;

       m_SubPages[pos+2] = pChild3;
       pChild2->m_Parent = this;
       pChild3->m_Parent = this;
}

// Ddivide a large page into 3 pages (2m/3 each one)
template <typename Trait>
void CBTreePage<Trait>::SplitPageInto3(vector<ObjectInfo>& tmpKeys,
                                                vector<BTPage *>  & tmpSubPages,
                                                BTPage*                   &     pChild1,
                                                BTPage*                   &     pChild2,
                                                BTPage*                   &     pChild3,
                                                ObjectInfo                & oi1,
                                                ObjectInfo                & oi2)
{
       assert(tmpKeys.size() >= 8);
       assert(tmpSubPages.size() >= 9);
       if( !pChild1 )
               pChild1 = new BTPage(m_MaxKeysForChilds, m_Unique);

       // Split tmpKeys page into 3 pages
       // copy 1/3 elements to the first child
       pChild1->clear();
       size_t nKeys = (tmpKeys.size()-2)/3;
       size_t i = 0;
       for(; i < nKeys; i++ )
       {
               pChild1->m_Keys    [i] = tmpKeys    [i];
               pChild1->m_SubPages[i] = tmpSubPages[i];
               if (tmpSubPages[i]) // Update parent for moved children
                        tmpSubPages[i]->m_Parent = pChild1;
               pChild1->NumberOfKeys()++;
       }
       pChild1->m_SubPages[i] = tmpSubPages[i];
       if (tmpSubPages[i])  // Update parent for last child
               tmpSubPages[i]->m_Parent = pChild1;

       // first element to go up !
       oi1 = tmpKeys[i++];

       if( !pChild2 )
               pChild2 = new BTPage(m_MaxKeysForChilds, m_Unique);
       pChild2->clear();
       // copy 1/3 to the second child
       nKeys += (tmpKeys.size()-2)/3 + 1;
       size_t j = 0;
       for(; i < nKeys; i++, j++ )
       {
               pChild2->m_Keys    [j] = tmpKeys    [i];
               pChild2->m_SubPages[j] = tmpSubPages[i];
               if (tmpSubPages[i])  // Update parent for moved children
                       tmpSubPages[i]->m_Parent = pChild2;
               pChild2->NumberOfKeys()++;
       }
       pChild2->m_SubPages[j] = tmpSubPages[i];
       if (tmpSubPages[i])  // Update parent for last child
               tmpSubPages[i]->m_Parent = pChild2;

       // copy the second element to the root
       oi2 = tmpKeys[i++];

       // copy 1/3 to the third child
       if( !pChild3 )
               pChild3 = new BTPage(m_MaxKeysForChilds, m_Unique);
       pChild3->clear();
       nKeys = tmpKeys.size();
       for(j = 0; i < nKeys; i++, j++)
       {
               pChild3->m_Keys    [j] = tmpKeys    [i];
               pChild3->m_SubPages[j] = tmpSubPages[i];
               if (tmpSubPages[i])  // Update parent for moved children
                       tmpSubPages[i]->m_Parent = pChild3;
               pChild3->NumberOfKeys()++;
       }
       pChild3->m_SubPages[j] = tmpSubPages[i];
       if (tmpSubPages[i])  // Update parent for last child
               tmpSubPages[i]->m_Parent = pChild3;
}

template <typename Trait>
bool CBTreePage<Trait>::SplitRoot(){
       BTPage  *pChild1 = 0, *pChild2 = 0, *pChild3 = 0;
       ObjectInfo oi1, oi2;
       SplitPageInto3( m_Keys,m_SubPages,pChild1, pChild2, pChild3, oi1, oi2);
       clear();

       // copy the first element to the root
       m_Keys    [0] = oi1;
       m_SubPages[0] = pChild1;
       pChild1->m_Parent = this;  // Update parent pointer
       NumberOfKeys()++;

       // copy the second element to the root
       m_Keys    [1] = oi2;
       m_SubPages[1] = pChild2;
       pChild2->m_Parent = this;  // Update parent pointer
       NumberOfKeys()++;

       m_SubPages[2] = pChild3;
       pChild3->m_Parent = this;  // Update parent pointer
       return true;
}

template <typename Trait>
bool CBTreePage<Trait>::Search(const keyType &key, ObjIDType &ObjID)
{
       size_t pos = binary_search(m_Keys, 0, m_KeyCount, key, compare);
       if( pos >= m_KeyCount )
       {    if( m_SubPages[pos] )
                return m_SubPages[pos]->Search(key, ObjID);
            else
                return false;
       }
       if( key == m_Keys[pos].key )
       {
               ObjID = m_Keys[pos].ObjID;
               m_Keys[pos].UseCounter++;
               return true;
       }
       if( key < m_Keys[pos].key )
               if( m_SubPages[pos] )
                       return m_SubPages[pos]->Search(key, ObjID);
       return false;
}

/*template <typename keyType, typename ObjIDType>
void CBTreePage<keyType, ObjIDType>::ForEachReverse(lpfnForEach2 lpfn, size_t level, void *pExtra1)
{
       if( m_SubPages[m_KeyCount] )
               m_SubPages[m_KeyCount]->ForEach(lpfn, level+1, pExtra1);
       for(size_t i = m_KeyCount-1 ; i >= 0  ; i--)
       {
               lpfn(m_Keys[i], level, pExtra1);
               if( m_SubPages[i] )
                       m_SubPages[i]->ForEach(lpfn, level+1, pExtra1);
       }
}*/

// Generalize ForEach with variadic templatess
template <typename Trait>
template <typename Func, typename... Args>
void CBTreePage<Trait>::ForEach(Func&& func, size_t level, Args&&... args)
{
       for(size_t i = 0 ; i < m_KeyCount ; i++)
       {
               if( m_SubPages[i] )
                       m_SubPages[i]->ForEach(std::forward<Func>(func), level+1, std::forward<Args>(args)...);
               func(m_Keys[i], level, std::forward<Args>(args)...);
       }
       if( m_SubPages[m_KeyCount] )
               m_SubPages[m_KeyCount]->ForEach(std::forward<Func>(func), level+1, std::forward<Args>(args)...);
}


// Apicar una funcion hasta encontrar el 1er elemento
// aque que retorne true ante esta funcion
// Generalize FirsThat with variadicc templates
template <typename Trait>
template <typename Func, typename... Args>
typename CBTreePage<Trait>::ObjectInfo *
CBTreePage<Trait>::FirstThat(Func&& func, size_t level, Args&&... args)
{
       ObjectInfo *pTmp;
       for(size_t i = 0 ; i < m_KeyCount ; i++)
       {
               if( m_SubPages[i] )
                       if( (pTmp = m_SubPages[i]->FirstThat(std::forward<Func>(func), level+1, std::forward<Args>(args)...)) )
                               return pTmp;
               if( func(m_Keys[i], level,std::forward<Args>(args)...) )
                       return &m_Keys[i];
       }
       if( m_SubPages[m_KeyCount] )
               if( (pTmp = m_SubPages[m_KeyCount]->FirstThat(std::forward<Func>(func), level+1, std::forward<Args>(args)...)) )
                       return pTmp;
       return nullptr;
}

template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::Remove(const keyType &key, const ObjIDType ObjID)
{
       bt_ErrorCode error = bt_ok;
       size_t pos = binary_search(m_Keys, 0, m_KeyCount, key, compare);
       if( pos < NumberOfKeys() && key == m_Keys[pos].key /*&& m_Keys[pos].m_ObjID == ObjID*/) // We found it !
       {
               // This is a leave: First
               if( !m_SubPages[pos+1] )  // This is a leave ? FIRST CASE !
               {
                       ::remove(m_Keys, pos);
                       NumberOfKeys()--;
                       if( Underflow() )
                               return bt_underflow;
                       return bt_ok;
               }

               // We FOUND IT BUT it is NOT a leave ? SECOND CASE !
               {
                       // Get the first element from right branch
                       ObjectInfo &rFirstFromRight = m_SubPages[pos+1]->GetFirstObjectInfo();
                       // change with a leave
                       swap(m_Keys[pos], rFirstFromRight);
                       // Remove it from this leave

                       //Print(cout);
                       error = m_SubPages[++pos]->Remove(key, ObjID);
               }
       }
       else if( pos == NumberOfKeys() ) // it is not here, go by the last branch
               error = m_SubPages[pos]->Remove(key, ObjID);
       else if( key <= m_Keys[pos].key ) // = is because identical keys are inserted on left (see Insert)
       {        if( m_SubPages[pos] )
                       error = m_SubPages[pos]->Remove(key, ObjID);
               else
                       return bt_nofound;
       }
       if( error == bt_underflow )
       {
               // THIRD CASE: After removing the element we have an underflow
               // Print(cout);
               if( TreatUnderflow(pos) )
                       return bt_ok;
               // FOURTH CASE: it was not possible to redistribute -> Merge
               if( IsRoot() && NumberOfKeys() == 2 )
                       return MergeRoot();
               return Merge(pos);
       }
       if( error == bt_nofound )
               return bt_nofound;
       return bt_ok;
}


template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::Merge(size_t pos)
{
       assert( m_SubPages[pos-1]->NumberOfKeys() +
                       m_SubPages[ pos ]->NumberOfKeys() +
                       m_SubPages[pos+1]->NumberOfKeys() ==
                       3*m_SubPages[ pos ]->MinNumberOfKeys() - 1);

       // FIRST: Put all the elements into a vector
       vector<ObjectInfo> tmpKeys;
       //tmpKeys.resize(nKeys);
       vector<BTPage *>   tmpSubPages;

       BTPage  *pChild1 = m_SubPages[pos-1],
                       *pChild2 = m_SubPages[ pos ],
                       *pChild3 = m_SubPages[pos+1];
       MovePage(pChild1, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[pos-1]);
       MovePage(pChild2, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[ pos ]);
       MovePage(pChild3, tmpKeys, tmpSubPages);
       pChild3->Destroy();;

       // Move 1/2 elements to pChild1
       size_t nKeys = pChild1->GetFreeCells();
       size_t i = 0;
       for(; i < nKeys ; i++ )
       {
               pChild1->m_Keys    [i] = tmpKeys    [i];
               pChild1->m_SubPages[i] = tmpSubPages[i];
               if (tmpSubPages[i])  // Update parent for moved children
                       tmpSubPages[i]->m_Parent = pChild1;
               pChild1->NumberOfKeys()++;
       }
       pChild1->m_SubPages[i] = tmpSubPages[i];
       if (tmpSubPages[i])  // Update parent for last child
               tmpSubPages[i]->m_Parent = pChild1;

       m_Keys    [pos-1] = tmpKeys[i];
       m_SubPages[pos-1] = pChild1;
       pChild1->m_Parent = this;  // Update parent pointer

       ::remove(m_Keys    , pos);
       ::remove(m_SubPages, pos);
       NumberOfKeys()--;

       nKeys = pChild2->GetFreeCells();

       // TODO: #32 change int by size_t
       size_t j = ++i;
       for(i = 0 ; i < nKeys ; i++, j++ )
       {
               pChild2->m_Keys    [i] = tmpKeys    [j];
               pChild2->m_SubPages[i] = tmpSubPages[j];
               if (tmpSubPages[j])  // Update parent for moved children
                       tmpSubPages[j]->m_Parent = pChild2;
               pChild2->NumberOfKeys()++;
       }
       pChild2->m_SubPages[i] = tmpSubPages[j];
       if (tmpSubPages[j])  // Update parent for last child
               tmpSubPages[j]->m_Parent = pChild2;
       m_SubPages[ pos ]          = pChild2;
       pChild2->m_Parent = this;  // Update parent pointer

       if( Underflow() )
               return bt_underflow;
       return bt_ok;
}

template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::MergeRoot()
{
        // TODO: #33 change int by size_t
       size_t pos = 1;
       assert( m_SubPages[pos-1]->NumberOfKeys() +
                       m_SubPages[ pos ]->NumberOfKeys() +
                       m_SubPages[pos+1]->NumberOfKeys() ==
                       3*m_SubPages[ pos ]->MinNumberOfKeys() - 1);

       BTPage  *pChild1 = m_SubPages[pos-1], *pChild2 = m_SubPages[ pos ], *pChild3 = m_SubPages[pos+1];
       // TODO: #34 change int by size_t
       size_t nKeys = pChild1->NumberOfKeys() + pChild2->NumberOfKeys() + pChild3->NumberOfKeys() + 2;

       // FIRST: Put all the elements into a vector
       vector<ObjectInfo> tmpKeys;
       //tmpKeys.resize(nKeys);
       vector<BTPage *>   tmpSubPages;

       MovePage(pChild1, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[pos-1]);
       MovePage(pChild2, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[ pos ]);
       MovePage(pChild3, tmpKeys, tmpSubPages);

       clear();
       size_t i = 0;
       for( ; i < nKeys ; i++ )
       {
               m_Keys    [i] = tmpKeys    [i];
               m_SubPages[i] = tmpSubPages[i];
               if (tmpSubPages[i])  // Update parent for moved children
                       tmpSubPages[i]->m_Parent = this;
               NumberOfKeys()++;
       }
       m_SubPages[i] = tmpSubPages[i];
       if (tmpSubPages[i])  // Update parent for last child
               tmpSubPages[i]->m_Parent = this;

       //Print(cout);
       pChild1->Destroy();
       pChild2->Destroy();
       pChild3->Destroy();

       return bt_rootmerged;
}

template <typename Trait>
typename CBTreePage<Trait>::ObjectInfo &
CBTreePage<Trait>::GetFirstObjectInfo()
{
        if( m_SubPages[0] )
                return m_SubPages[0]->GetFirstObjectInfo();
        return m_Keys[0];
}

template <typename keyType, typename ObjIDType>
void Print(tagObjectInfo<keyType, ObjIDType> &info, size_t level, void *pExtra)
{
       ostream &os = *(ostream *)pExtra;
       for(size_t i = 0; i < level ; i++)
               os << "\t";
       os << info.key << "->" << info.ObjID << "\n";
}

template <typename Trait>
void CBTreePage<Trait>::Print(ostream & os)
{
      // Using the new  ForEach 
        ForEach([](ObjectInfo& info, size_t level, ostream &os) {
                for(size_t i = 0; i < level; i++)
                        os << "\t";
                os << info.key << "-->" << info.ObjID << "\n";
        }, 0, os);
}

template <typename Trait>
void CBTreePage<Trait>::Create()
{
       Reset();
       m_Keys.resize(m_MaxKeys+1);
       m_SubPages.resize(m_MaxKeys+2, NULL);
       m_KeyCount = 0;
       m_MinKeys  = 2 * m_MaxKeys/3;
}

template <typename Trait>
void CBTreePage<Trait>::Reset()
{
        // TODO: #35 change int by size_t
       for( size_t i = 0 ; i < m_KeyCount ; i++ )
               delete m_SubPages[i];
       clear();
}

template <typename Trait>
void CBTreePage<Trait>::clear()
{
       //m_Keys.clear();
       //m_SubPages.clear();
       m_KeyCount = 0;
}

template <typename Trait>
CBTreePage<Trait> * CreateBTreeNode (size_t maxKeys, bool unique)
{
       return new CBTreePage<Trait> (maxKeys, unique);
}

template <typename Trait>
void CBTreePage<Trait>::MovePage(BTPage *pChildPage, vector<ObjectInfo> &tmpKeys,vector<BTPage *> &tmpSubPages)
{
        // TODO: #37 change int by size_t
       size_t nKeys = pChildPage->GetNumberOfKeys();
       size_t i = 0;
       for(i = 0; i < nKeys; i++ )
       {
                tmpKeys    .push_back(pChildPage->m_Keys[i]);
                tmpSubPages.push_back(pChildPage->m_SubPages[i]);
       }
       tmpSubPages.push_back(pChildPage->m_SubPages[i]);
       pChildPage->clear();
}

#endif