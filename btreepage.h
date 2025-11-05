#ifndef __CBTreePage_H__
#define __CBTreePage_H__

#include <vector>
#include <assert.h>
#include <functional>
#include <utility>
#include <fstream>
#include <cstring>

// TODO: #1 Crear una function para agregarla al demo.cpp ( no trivial )
// TODO: #2 Agregarle un Trait (prueba git) ( no trivial )
// TODO: #3 crear un iterator ( no trivial )
//       Sugerencia: Tarea1 cada pagina debe tener un puntero al padre primero ( no trivial )
// TODO: #4 integrarlo al recorrer ( no trivial )


template <typename Trait>
class BTree;

using namespace std;
enum bt_ErrorCode {bt_ok, bt_overflow, bt_underflow, bt_duplicate, bt_nofound, bt_rootmerged};

template <typename Container, typename ObjType>
size_t binary_search(Container& container, size_t first, size_t last, ObjType &object)
{
       if( first >= last )
               return first;
       while( first < last )
       {
               size_t mid = (first+last)/2;
               if( object == (ObjType)container[mid ] )
                       return mid;
               if( object > (ObjType)container[mid ] )
                       first = mid+1;
               else
                       last  = mid;
       }
       if( object <= (ObjType)container[first] )
               return first;
       return last;
}

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

template <typename Container>
void remove(Container& container, size_t pos)
{
       size_t size = container.size();
       for(auto i = pos+1 ; i < size ; i++)
           container[i-1] = container[i];
}

template <typename keyType, typename ObjIDType>
struct tagObjectInfo
{
       keyType                 key;
       ObjIDType               ObjID;
       size_t                    UseCounter;
       tagObjectInfo(const keyType     &_key, ObjIDType _ObjID)
               : key(_key), ObjID(_ObjID), UseCounter(0) {}
       tagObjectInfo(const tagObjectInfo &objInfo)
               : key(objInfo.key), ObjID(objInfo.ObjID), UseCounter(0) {}
       tagObjectInfo()                          {}
       operator keyType                         ()     { return key; }
       size_t                    GetUseCounter() { return UseCounter;    }
};

template <typename Trait>
class CBTreePage //: public SimpleIndex <keyType>
// this is the in-memory version of the CBTreePage
{
       friend class BTree<Trait>;
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType  ObjIDType;
       typedef typename Trait::CompareFn  CompareFn;

       typedef CBTreePage<Trait>    BTPage;         // useful shorthand
       typedef tagObjectInfo<keyType, ObjIDType> ObjectInfo;

 public:
       CBTreePage(size_t maxKeys, bool unique, CompareFn compare);
       virtual ~CBTreePage();

       // Rule of Five: Move semantics
       CBTreePage(CBTreePage&& other) noexcept;
       CBTreePage& operator=(CBTreePage&& other) noexcept;

       // Rule of Five: Copy semantics (deleted to prevent accidental copies)
       CBTreePage(const CBTreePage& other) = delete;
       CBTreePage& operator=(const CBTreePage& other) = delete;

       bt_ErrorCode    Insert (const keyType &key, const ObjIDType ObjID);
       bt_ErrorCode    Remove (const keyType &key, const ObjIDType ObjID);
       bool            Search (const keyType &key, ObjIDType &ObjID);
       void            Print  (ostream &os);
       void            PrintInternal(ostream &os, size_t level);

       // Generic ForEach using variadic templates and std::invoke
       template <typename Function, typename... Args>
       void ForEach(Function&& func, size_t level, Args&&... args) const;

       // Generic FirstThat using variadic templates and std::invoke
       // Returns pointer to first ObjectInfo that satisfies the predicate
       template <typename Predicate, typename... Args>
       ObjectInfo* FirstThat(Predicate&& pred, size_t level, Args&&... args);

       // FindAll - Finds all elements that satisfy the predicate
       // Returns vector of pointers to matching ObjectInfo
       template <typename Predicate, typename... Args>
       void FindAll(vector<ObjectInfo*>& results, Predicate&& pred, size_t level, Args&&... args);

       // CountIf - Counts elements that satisfy the predicate
       template <typename Predicate, typename... Args>
       size_t CountIf(Predicate&& pred, size_t level, Args&&... args);

       // AnyOf - Returns true if at least one element satisfies the predicate
       template <typename Predicate, typename... Args>
       bool AnyOf(Predicate&& pred, size_t level, Args&&... args);

       // AllOf - Returns true if all elements satisfy the predicate
       template <typename Predicate, typename... Args>
       bool AllOf(Predicate&& pred, size_t level, Args&&... args);

       // NoneOf - Returns true if no element satisfies the predicate
       template <typename Predicate, typename... Args>
       bool NoneOf(Predicate&& pred, size_t level, Args&&... args);

       // Accumulate - Accumulates a value by applying a binary operation
       template <typename T, typename BinaryOp, typename... Args>
       T Accumulate(T init, BinaryOp&& op, size_t level, Args&&... args);

       // Transform - Transforms all elements and stores results
       template <typename OutputContainer, typename UnaryOp, typename... Args>
       void Transform(OutputContainer& output, UnaryOp&& op, size_t level, Args&&... args);

       // Serialization: Write/Read to/from binary stream
       void WriteToDisk(std::ofstream& out) const;
       void ReadFromDisk(std::ifstream& in);

protected:
       // TODO: #9 change by size_t
       size_t  m_MinKeys; // minimum number of keys in a node
       size_t  m_MaxKeys, // maximum number of keys in a node

                m_MaxKeysForChilds; // just to distinguish the root
       bool m_Unique;
       bool m_isRoot;
       CompareFn m_Compare; // Comparison function object
       //size_t           NextNode; // address of next node at same level
       //size_t RecAddr; // address of this node in the BTree file
       vector<ObjectInfo> m_Keys;
       vector<BTPage *>m_SubPages;

       // TODO: #10 size_t
       size_t  m_KeyCount;
       void  Create();
       void  Reset ();
       void  Destroy () {   Reset(); delete this;}
       void  clear ();

       // Internal binary search using comparison function
       size_t InternalBinarySearch(size_t first, size_t last, const keyType &key);

       bool  RedistributeWith1Brother   (size_t &pos);
       bool  RedistributeWith2Brothers   (size_t pos);
       void  RedistributeR2L (size_t pos);
       void  RedistributeL2R (size_t pos);

       bool    TreatUnderflow  (size_t &pos)
       {       return RedistributeWith1Brother(pos) || RedistributeWith2Brothers(pos);}

       bt_ErrorCode    Merge  (size_t pos);
       bt_ErrorCode    MergeRoot ();
       void  SplitChild (size_t pos);

       ObjectInfo &GetFirstObjectInfo();

       bool Overflow()  { return m_KeyCount > m_MaxKeys; }
       bool Underflow() { return m_KeyCount < MinNumberOfKeys(); }
       bool IsFull()    { return m_KeyCount >= m_MaxKeys; }


       size_t  MinNumberOfKeys()  { return 2*m_MaxKeys/3.0; }
       size_t  GetFreeCells()  { return m_MaxKeys - m_KeyCount; }
       size_t& NumberOfKeys()  { return m_KeyCount; }
       size_t  GetNumberOfKeys()  { return m_KeyCount; }
       bool IsRoot()  { return m_MaxKeysForChilds != m_MaxKeys; }
       void SetMaxKeysForChilds(size_t orderforchilds)
       {        m_MaxKeysForChilds = orderforchilds;       }
       size_t GetFreeCellsOnLeft(size_t pos)
       {        if( pos > 0 )                                   // there is some page on left ?
                        return m_SubPages[pos-1]->GetFreeCells();
                return 0;
       }
       size_t GetFreeCellsOnRight(size_t pos)
       {    if( pos < GetNumberOfKeys() )   // there is some page on right ?
                return m_SubPages[pos+1]->GetFreeCells();
            return 0;
       }

private:
       bool SplitRoot();
       void SplitPageInto3(vector<ObjectInfo>   & tmpKeys,
                                               vector<BTPage *>  & SubPages,
                                               BTPage           *& pChild1,
                                               BTPage           *& pChild2,
                                               BTPage           *& pChild3,
                                               ObjectInfo        & oi1,
                                               ObjectInfo        & oi2);
       void MovePage(BTPage *  pChildPage,vector<ObjectInfo> & tmpKeys,vector<BTPage *> & tmpSubPages);
};

template <typename Trait>
CBTreePage<Trait>:: CBTreePage(size_t maxKeys, bool unique, CompareFn compare)
                               : m_MaxKeys(maxKeys), m_Unique(unique), m_Compare(compare), m_KeyCount(0)
{
       Create();
       SetMaxKeysForChilds(m_MaxKeys);
}

template <typename Trait>
CBTreePage<Trait>::~CBTreePage()
{
       Reset();
}

// Move Constructor - transfers ownership of resources
template <typename Trait>
CBTreePage<Trait>::CBTreePage(CBTreePage&& other) noexcept
       : m_MinKeys(other.m_MinKeys),
         m_MaxKeys(other.m_MaxKeys),
         m_MaxKeysForChilds(other.m_MaxKeysForChilds),
         m_Unique(other.m_Unique),
         m_isRoot(other.m_isRoot),
         m_Compare(std::move(other.m_Compare)),
         m_Keys(std::move(other.m_Keys)),
         m_SubPages(std::move(other.m_SubPages)),
         m_KeyCount(other.m_KeyCount)
{
       // Leave other in a valid but empty state
       // Reinitialize the moved-from object to make it usable again
       other.m_KeyCount = 0;
       other.m_MinKeys = 2 * other.m_MaxKeys / 3;
       // Reinitialize vectors to proper size (empty but allocated)
       other.m_Keys.clear();
       other.m_Keys.resize(other.m_MaxKeys + 1);
       other.m_SubPages.clear();
       other.m_SubPages.resize(other.m_MaxKeys + 2, nullptr);
}

// Move Assignment Operator - transfers ownership with proper cleanup
template <typename Trait>
CBTreePage<Trait>& CBTreePage<Trait>::operator=(CBTreePage&& other) noexcept
{
       if(this != &other) {
               // First, release current resources
               Reset();

               // Transfer ownership of resources
               m_MinKeys = other.m_MinKeys;
               m_MaxKeys = other.m_MaxKeys;
               m_MaxKeysForChilds = other.m_MaxKeysForChilds;
               m_Unique = other.m_Unique;
               m_isRoot = other.m_isRoot;
               m_Compare = std::move(other.m_Compare);
               m_Keys = std::move(other.m_Keys);
               m_SubPages = std::move(other.m_SubPages);
               m_KeyCount = other.m_KeyCount;

               // Leave other in a valid but empty state
               // Reinitialize the moved-from object to make it usable again
               other.m_KeyCount = 0;
               other.m_MinKeys = 2 * other.m_MaxKeys / 3;
               // Reinitialize vectors to proper size (empty but allocated)
               other.m_Keys.clear();
               other.m_Keys.resize(other.m_MaxKeys + 1);
               other.m_SubPages.clear();
               other.m_SubPages.resize(other.m_MaxKeys + 2, nullptr);
       }
       return *this;
}

template <typename Trait>
size_t CBTreePage<Trait>::InternalBinarySearch(size_t first, size_t last, const keyType &key)
{
       if( first >= last )
               return first;
       while( first < last )
       {
               size_t mid = (first+last)/2;
               keyType midKey = (keyType)m_Keys[mid];

               // key == midKey (neither key < midKey nor midKey < key)
               if( !m_Compare(key, midKey) && !m_Compare(midKey, key) )
                       return mid;
               // key > midKey (midKey < key, ascending order needs key to go right)
               if( m_Compare(midKey, key) )
                       first = mid+1;
               else
                       last  = mid;
       }
       keyType firstKey = (keyType)m_Keys[first];
       // key <= firstKey (key < firstKey or key == firstKey)
       if( m_Compare(key, firstKey) || (!m_Compare(key, firstKey) && !m_Compare(firstKey, key)) )
               return first;
       return last;
}

template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::Insert(const keyType& key, const ObjIDType ObjID){
       size_t pos = InternalBinarySearch(0, m_KeyCount, key);
       bt_ErrorCode error = bt_ok;

       // Check for duplicate using comparison function
       if( pos < m_KeyCount && !m_Compare(key, m_Keys[pos].key) &&
           !m_Compare(m_Keys[pos].key, key) && m_Unique)
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

       // copy the second element to the root
       ::insert_at(m_Keys, oi2, pos+1);
       ::insert_at(m_SubPages, pChild2, pos+1);
       NumberOfKeys()++;

       m_SubPages[pos+2] = pChild3;
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
               pChild1 = new BTPage(m_MaxKeysForChilds, m_Unique, m_Compare);

       // Split tmpKeys page into 3 pages
       // copy 1/3 elements to the first child
       pChild1->clear();
       size_t nKeys = (tmpKeys.size()-2)/3;
       size_t i = 0;
       for(; i < nKeys; i++ )
       {
               pChild1->m_Keys    [i] = tmpKeys    [i];
               pChild1->m_SubPages[i] = tmpSubPages[i];
               pChild1->NumberOfKeys()++;
       }
       pChild1->m_SubPages[i] = tmpSubPages[i];

       // first element to go up !
       oi1 = tmpKeys[i++];

       if( !pChild2 )
               pChild2 = new BTPage(m_MaxKeysForChilds, m_Unique, m_Compare);
       pChild2->clear();
       // copy 1/3 to the second child
       nKeys += (tmpKeys.size()-2)/3 + 1;
       size_t j = 0;
       for(; i < nKeys; i++, j++ )
       {
               pChild2->m_Keys    [j] = tmpKeys    [i];
               pChild2->m_SubPages[j] = tmpSubPages[i];
               pChild2->NumberOfKeys()++;
       }
       pChild2->m_SubPages[j] = tmpSubPages[i];

       // copy the second element to the root
       oi2 = tmpKeys[i++];

       // copy 1/3 to the third child
       if( !pChild3 )
               pChild3 = new BTPage(m_MaxKeysForChilds, m_Unique, m_Compare);
       pChild3->clear();
       nKeys = tmpKeys.size();
       for(j = 0; i < nKeys; i++, j++)
       {
               pChild3->m_Keys    [j] = tmpKeys    [i];
               pChild3->m_SubPages[j] = tmpSubPages[i];
               pChild3->NumberOfKeys()++;
       }
       pChild3->m_SubPages[j] = tmpSubPages[i];
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
       NumberOfKeys()++;

       // copy the second element to the root
       m_Keys    [1] = oi2;
       m_SubPages[1] = pChild2;
       NumberOfKeys()++;

       m_SubPages[2] = pChild3;
       return true;
}

template <typename Trait>
bool CBTreePage<Trait>::Search(const keyType &key, ObjIDType &ObjID)
{
       size_t pos = InternalBinarySearch(0, m_KeyCount, key);
       if( pos >= m_KeyCount )
       {    if( m_SubPages[pos] )
                return m_SubPages[pos]->Search(key, ObjID);
            else
                return false;
       }
       // key == m_Keys[pos].key using comparison function
       if( !m_Compare(key, m_Keys[pos].key) && !m_Compare(m_Keys[pos].key, key) )
       {
               ObjID = m_Keys[pos].ObjID;
               m_Keys[pos].UseCounter++;
               return true;
       }
       // key < m_Keys[pos].key
       if( m_Compare(key, m_Keys[pos].key) )
               if( m_SubPages[pos] )
                       return m_SubPages[pos]->Search(key, ObjID);
       return false;
}

// Generic ForEach implementation using variadic templates and perfect forwarding
// Traverses the BTree in-order, applying the function to each element
template <typename Trait>
template <typename Function, typename... Args>
void CBTreePage<Trait>::ForEach(Function&& func, size_t level, Args&&... args) const
{
       for(size_t i = 0 ; i < m_KeyCount ; i++)
       {
               // Recursively traverse left subtree
               if( m_SubPages[i] )
                       m_SubPages[i]->ForEach(std::forward<Function>(func), level+1, std::forward<Args>(args)...);

               // Apply function to current element using std::invoke for maximum flexibility
               std::invoke(std::forward<Function>(func), m_Keys[i], level, std::forward<Args>(args)...);
       }
       // Traverse rightmost subtree
       if( m_SubPages[m_KeyCount] )
               m_SubPages[m_KeyCount]->ForEach(std::forward<Function>(func), level+1, std::forward<Args>(args)...);
}

// Generic FirstThat implementation using variadic templates and perfect forwarding
// Finds the first element that satisfies the predicate (returns true)
// Traverses the BTree in-order and returns pointer to first matching ObjectInfo
template <typename Trait>
template <typename Predicate, typename... Args>
typename CBTreePage<Trait>::ObjectInfo *
CBTreePage<Trait>::FirstThat(Predicate&& pred, size_t level, Args&&... args)
{
       ObjectInfo *pTmp;
       for(size_t i = 0 ; i < m_KeyCount ; i++)
       {
               // Search in left subtree
               if( m_SubPages[i] )
                       if( (pTmp = m_SubPages[i]->FirstThat(std::forward<Predicate>(pred), level+1, std::forward<Args>(args)...)) )
                               return pTmp;

               // Check current element using std::invoke
               // Predicate should return bool (or convertible to bool)
               if( std::invoke(std::forward<Predicate>(pred), m_Keys[i], level, std::forward<Args>(args)...) )
                       return &m_Keys[i];
       }
       // Search in rightmost subtree
       if( m_SubPages[m_KeyCount] )
               if( (pTmp = m_SubPages[m_KeyCount]->FirstThat(std::forward<Predicate>(pred), level+1, std::forward<Args>(args)...)) )
                       return pTmp;
       return nullptr;
}

// FindAll - Finds ALL elements that satisfy the predicate
// Unlike FirstThat, this continues searching through the entire tree
// Results are accumulated in the provided vector
template <typename Trait>
template <typename Predicate, typename... Args>
void CBTreePage<Trait>::FindAll(vector<ObjectInfo*>& results, Predicate&& pred, size_t level, Args&&... args)
{
       for(size_t i = 0 ; i < m_KeyCount ; i++)
       {
               // Search in left subtree
               if( m_SubPages[i] )
                       m_SubPages[i]->FindAll(results, std::forward<Predicate>(pred), level+1, std::forward<Args>(args)...);

               // Check current element
               if( std::invoke(std::forward<Predicate>(pred), m_Keys[i], level, std::forward<Args>(args)...) )
                       results.push_back(&m_Keys[i]);
       }
       // Search in rightmost subtree
       if( m_SubPages[m_KeyCount] )
               m_SubPages[m_KeyCount]->FindAll(results, std::forward<Predicate>(pred), level+1, std::forward<Args>(args)...);
}

// CountIf - Counts elements that satisfy the predicate
// Returns the total count of matching elements
template <typename Trait>
template <typename Predicate, typename... Args>
size_t CBTreePage<Trait>::CountIf(Predicate&& pred, size_t level, Args&&... args)
{
       size_t count = 0;
       for(size_t i = 0 ; i < m_KeyCount ; i++)
       {
               // Count in left subtree
               if( m_SubPages[i] )
                       count += m_SubPages[i]->CountIf(std::forward<Predicate>(pred), level+1, std::forward<Args>(args)...);

               // Check current element
               if( std::invoke(std::forward<Predicate>(pred), m_Keys[i], level, std::forward<Args>(args)...) )
                       count++;
       }
       // Count in rightmost subtree
       if( m_SubPages[m_KeyCount] )
               count += m_SubPages[m_KeyCount]->CountIf(std::forward<Predicate>(pred), level+1, std::forward<Args>(args)...);

       return count;
}

// AnyOf - Returns true if at least one element satisfies the predicate
// Short-circuits on first match (more efficient than CountIf > 0)
template <typename Trait>
template <typename Predicate, typename... Args>
bool CBTreePage<Trait>::AnyOf(Predicate&& pred, size_t level, Args&&... args)
{
       for(size_t i = 0 ; i < m_KeyCount ; i++)
       {
               // Check left subtree
               if( m_SubPages[i] )
                       if( m_SubPages[i]->AnyOf(std::forward<Predicate>(pred), level+1, std::forward<Args>(args)...) )
                               return true;  // Early termination

               // Check current element
               if( std::invoke(std::forward<Predicate>(pred), m_Keys[i], level, std::forward<Args>(args)...) )
                       return true;  // Early termination
       }
       // Check rightmost subtree
       if( m_SubPages[m_KeyCount] )
               return m_SubPages[m_KeyCount]->AnyOf(std::forward<Predicate>(pred), level+1, std::forward<Args>(args)...);

       return false;
}

// AllOf - Returns true if all elements satisfy the predicate
// Short-circuits on first non-match
template <typename Trait>
template <typename Predicate, typename... Args>
bool CBTreePage<Trait>::AllOf(Predicate&& pred, size_t level, Args&&... args)
{
       for(size_t i = 0 ; i < m_KeyCount ; i++)
       {
               // Check left subtree
               if( m_SubPages[i] )
                       if( !m_SubPages[i]->AllOf(std::forward<Predicate>(pred), level+1, std::forward<Args>(args)...) )
                               return false;  // Early termination

               // Check current element
               if( !std::invoke(std::forward<Predicate>(pred), m_Keys[i], level, std::forward<Args>(args)...) )
                       return false;  // Early termination
       }
       // Check rightmost subtree
       if( m_SubPages[m_KeyCount] )
               return m_SubPages[m_KeyCount]->AllOf(std::forward<Predicate>(pred), level+1, std::forward<Args>(args)...);

       return true;
}

// NoneOf - Returns true if no element satisfies the predicate
// Equivalent to !AnyOf, but provided for semantic clarity
template <typename Trait>
template <typename Predicate, typename... Args>
bool CBTreePage<Trait>::NoneOf(Predicate&& pred, size_t level, Args&&... args)
{
       return !AnyOf(std::forward<Predicate>(pred), level, std::forward<Args>(args)...);
}

// Accumulate - Accumulates values using a binary operation
// Similar to std::accumulate but for tree traversal
// init: initial value, op: binary operation (accumulator, element) -> new accumulator
template <typename Trait>
template <typename T, typename BinaryOp, typename... Args>
T CBTreePage<Trait>::Accumulate(T init, BinaryOp&& op, size_t level, Args&&... args)
{
       T result = init;
       for(size_t i = 0 ; i < m_KeyCount ; i++)
       {
               // Accumulate from left subtree
               if( m_SubPages[i] )
                       result = m_SubPages[i]->Accumulate(result, std::forward<BinaryOp>(op), level+1, std::forward<Args>(args)...);

               // Apply operation to current element
               result = std::invoke(std::forward<BinaryOp>(op), result, m_Keys[i], level, std::forward<Args>(args)...);
       }
       // Accumulate from rightmost subtree
       if( m_SubPages[m_KeyCount] )
               result = m_SubPages[m_KeyCount]->Accumulate(result, std::forward<BinaryOp>(op), level+1, std::forward<Args>(args)...);

       return result;
}

// Transform - Transforms all elements and stores results
// Similar to std::transform but for tree traversal
// Applies unary operation to each element and stores in output container
template <typename Trait>
template <typename OutputContainer, typename UnaryOp, typename... Args>
void CBTreePage<Trait>::Transform(OutputContainer& output, UnaryOp&& op, size_t level, Args&&... args)
{
       for(size_t i = 0 ; i < m_KeyCount ; i++)
       {
               // Transform left subtree
               if( m_SubPages[i] )
                       m_SubPages[i]->Transform(output, std::forward<UnaryOp>(op), level+1, std::forward<Args>(args)...);

               // Apply operation to current element and store result
               output.push_back(std::invoke(std::forward<UnaryOp>(op), m_Keys[i], level, std::forward<Args>(args)...));
       }
       // Transform rightmost subtree
       if( m_SubPages[m_KeyCount] )
               m_SubPages[m_KeyCount]->Transform(output, std::forward<UnaryOp>(op), level+1, std::forward<Args>(args)...);
}

// =============================================================================
// Serialization: WriteToDisk - Writes page recursively in preorder
// =============================================================================
template <typename Trait>
void CBTreePage<Trait>::WriteToDisk(std::ofstream& out) const
{
       if(!out.good()) {
               throw std::runtime_error("WriteToDisk: Output stream is not valid");
       }

       // Write node metadata
       out.write(reinterpret_cast<const char*>(&m_KeyCount), sizeof(m_KeyCount));
       out.write(reinterpret_cast<const char*>(&m_MinKeys), sizeof(m_MinKeys));
       out.write(reinterpret_cast<const char*>(&m_MaxKeys), sizeof(m_MaxKeys));
       out.write(reinterpret_cast<const char*>(&m_MaxKeysForChilds), sizeof(m_MaxKeysForChilds));
       out.write(reinterpret_cast<const char*>(&m_Unique), sizeof(m_Unique));
       out.write(reinterpret_cast<const char*>(&m_isRoot), sizeof(m_isRoot));

       // Write keys data
       for(size_t i = 0; i < m_KeyCount; i++) {
               // Write key
               out.write(reinterpret_cast<const char*>(&m_Keys[i].key), sizeof(m_Keys[i].key));
               // Write ObjID
               out.write(reinterpret_cast<const char*>(&m_Keys[i].ObjID), sizeof(m_Keys[i].ObjID));
               // Write UseCounter
               out.write(reinterpret_cast<const char*>(&m_Keys[i].UseCounter), sizeof(m_Keys[i].UseCounter));
       }

       // Write child pointers information (flags indicating if child exists)
       for(size_t i = 0; i <= m_KeyCount; i++) {
               bool hasChild = (m_SubPages[i] != nullptr);
               out.write(reinterpret_cast<const char*>(&hasChild), sizeof(hasChild));
       }

       // Recursively write children in preorder
       for(size_t i = 0; i <= m_KeyCount; i++) {
               if(m_SubPages[i] != nullptr) {
                       m_SubPages[i]->WriteToDisk(out);
               }
       }

       if(!out.good()) {
               throw std::runtime_error("WriteToDisk: Error writing to stream");
       }
}

// =============================================================================
// Serialization: ReadFromDisk - Reads page recursively in preorder
// =============================================================================
template <typename Trait>
void CBTreePage<Trait>::ReadFromDisk(std::ifstream& in)
{
       if(!in.good()) {
               throw std::runtime_error("ReadFromDisk: Input stream is not valid");
       }

       // First, clean up existing data
       Reset();

       // Read node metadata
       in.read(reinterpret_cast<char*>(&m_KeyCount), sizeof(m_KeyCount));
       in.read(reinterpret_cast<char*>(&m_MinKeys), sizeof(m_MinKeys));
       in.read(reinterpret_cast<char*>(&m_MaxKeys), sizeof(m_MaxKeys));
       in.read(reinterpret_cast<char*>(&m_MaxKeysForChilds), sizeof(m_MaxKeysForChilds));
       in.read(reinterpret_cast<char*>(&m_Unique), sizeof(m_Unique));
       in.read(reinterpret_cast<char*>(&m_isRoot), sizeof(m_isRoot));

       // Resize vectors to proper size
       m_Keys.resize(m_MaxKeys + 1);
       m_SubPages.resize(m_MaxKeys + 2, nullptr);

       // Read keys data
       for(size_t i = 0; i < m_KeyCount; i++) {
               // Read key
               in.read(reinterpret_cast<char*>(&m_Keys[i].key), sizeof(m_Keys[i].key));
               // Read ObjID
               in.read(reinterpret_cast<char*>(&m_Keys[i].ObjID), sizeof(m_Keys[i].ObjID));
               // Read UseCounter
               in.read(reinterpret_cast<char*>(&m_Keys[i].UseCounter), sizeof(m_Keys[i].UseCounter));
       }

       // Read child pointers flags
       // Note: Using vector<char> instead of vector<bool> because vector<bool>
       // is a special case that doesn't allow taking address of elements
       vector<char> hasChildren(m_KeyCount + 1);
       for(size_t i = 0; i <= m_KeyCount; i++) {
               bool flag;
               in.read(reinterpret_cast<char*>(&flag), sizeof(bool));
               hasChildren[i] = flag ? 1 : 0;
       }

       // Recursively read children in preorder
       for(size_t i = 0; i <= m_KeyCount; i++) {
               if(hasChildren[i]) {
                       m_SubPages[i] = new BTPage(m_MaxKeys, m_Unique, m_Compare);
                       m_SubPages[i]->ReadFromDisk(in);
               } else {
                       m_SubPages[i] = nullptr;
               }
       }

       if(!in.good()) {
               throw std::runtime_error("ReadFromDisk: Error reading from stream");
       }
}

template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::Remove(const keyType &key, const ObjIDType ObjID)
{
       bt_ErrorCode error = bt_ok;
       size_t pos = InternalBinarySearch(0, m_KeyCount, key);
       // key == m_Keys[pos].key using comparison function
       if( pos < NumberOfKeys() && !m_Compare(key, m_Keys[pos].key) &&
           !m_Compare(m_Keys[pos].key, key) /*&& m_Keys[pos].m_ObjID == ObjID*/) // We found it !
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
       // key <= m_Keys[pos].key using comparison function
       // (key < m_Keys[pos].key or key == m_Keys[pos].key)
       else if( m_Compare(key, m_Keys[pos].key) ||
                (!m_Compare(key, m_Keys[pos].key) && !m_Compare(m_Keys[pos].key, key)) )
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
               pChild1->NumberOfKeys()++;
       }
       pChild1->m_SubPages[i] = tmpSubPages[i];

       m_Keys    [pos-1] = tmpKeys[i];
       m_SubPages[pos-1] = pChild1;

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
               pChild2->NumberOfKeys()++;
       }
       pChild2->m_SubPages[i] = tmpSubPages[j];
       m_SubPages[ pos ]          = pChild2;

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
               NumberOfKeys()++;
       }
       m_SubPages[i] = tmpSubPages[i];

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

template <typename Trait>
void CBTreePage<Trait>::PrintInternal(ostream & os, size_t level)
{
       for(size_t i = 0 ; i < m_KeyCount ; i++)
       {
               if( m_SubPages[i] )
                       m_SubPages[i]->PrintInternal(os, level+1);

               for(size_t j = 0; j < level ; j++)
                       os << "\t";
               os << m_Keys[i].key << "->" << m_Keys[i].ObjID << "\n";
       }
       if( m_SubPages[m_KeyCount] )
               m_SubPages[m_KeyCount]->PrintInternal(os, level+1);
}

template <typename Trait>
void CBTreePage<Trait>::Print(ostream & os)
{
       PrintInternal(os, 0);
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