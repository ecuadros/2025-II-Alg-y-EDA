#ifndef __CBTreePage_H__
#define __CBTreePage_H__

#include <vector>
#include <assert.h>
#include <functional>
#include <utility>  // Para std::move

// TODO: #1 Crear una function para agregarla al demo.cpp ( no trivial )
// TODO: #2 Agregarle un Trait (prueba git) ( no trivial )
// TODO: #3 crear un iterator ( no trivial )
//       Sugerencia: Tarea1 cada pagina debe tener un puntero al padre primero ( no trivial )
// TODO: #4 integrarlo al recorrer ( no trivial )


template <typename Trait>
class BTree;

template <typename Trait>
class forward_btree_iterator;

template <typename Trait>
class backward_btree_iterator;

using namespace std;
enum bt_ErrorCode {bt_ok, bt_overflow, bt_underflow, bt_duplicate, bt_nofound, bt_rootmerged};

template <typename Container, typename ObjType, typename Compare>
size_t binary_search(Container& container, size_t first, size_t last, ObjType &object, Compare comp)
{
       if( first >= last )
               return first;
       while( first < last )
       {
               size_t mid = (first+last)/2;
               if( !comp(object, (ObjType)container[mid]) && !comp((ObjType)container[mid], object) )
                       return mid;
               if( comp((ObjType)container[mid], object) )
                       first = mid+1;
               else
                       last  = mid;
       }
       if( !comp((ObjType)container[first], object) )
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
       friend class forward_btree_iterator<Trait>;
       friend class backward_btree_iterator<Trait>;
       typedef typename Trait::keyType  keyType;
       typedef typename Trait::ObjIDType  ObjIDType;
       typedef typename Trait::Compare  Compare;

       typedef CBTreePage<Trait>    BTPage;         // useful shorthand
public:
       typedef tagObjectInfo<keyType, ObjIDType> ObjectInfo;
private:

       typedef void (*lpfnForEach2)(ObjectInfo &info, size_t level, void *pExtra1);
       typedef void (*lpfnForEach3)(ObjectInfo &info, size_t level, void *pExtra1, void *pExtra2);

       typedef ObjectInfo *(*lpfnFirstThat2)(ObjectInfo &info, size_t level, void *pExtra1);
       typedef ObjectInfo *(*lpfnFirstThat3)(ObjectInfo &info, size_t level, void *pExtra1, void *pExtra2);
 public:
       CBTreePage(size_t maxKeys, bool unique = true);
       virtual ~CBTreePage();

       // Delete copy constructor and copy assignment (evitar copias accidentales)
       CBTreePage(const CBTreePage&) = delete;
       CBTreePage& operator=(const CBTreePage&) = delete;

       // Move Constructor
       CBTreePage(CBTreePage&& other) noexcept
              : m_MaxKeys(other.m_MaxKeys),
                m_Unique(other.m_Unique),
                m_Compare(std::move(other.m_Compare)),
                m_Parent(nullptr),  // El nuevo nodo no tiene padre inicialmente
                m_KeyCount(other.m_KeyCount),
                m_MinKeys(other.m_MinKeys),
                m_MaxKeysForChilds(other.m_MaxKeysForChilds),
                m_isRoot(other.m_isRoot),
                m_Keys(std::move(other.m_Keys)),
                m_SubPages(std::move(other.m_SubPages))
       {
              // Actualizar referencias de padre en los hijos
              for (size_t i = 0; i <= m_KeyCount; i++) {
                     if (m_SubPages[i]) {
                            m_SubPages[i]->SetParent(this);
                     }
              }

              // Reset other to a valid but empty state
              other.m_Parent = nullptr;
              other.m_KeyCount = 0;
       }

       // Move Assignment Operator
       CBTreePage& operator=(CBTreePage&& other) noexcept
       {
              if (this != &other) {
                     // First, clean up current resources
                     Reset();

                     // Move data from other
                     m_MaxKeys = other.m_MaxKeys;
                     m_MinKeys = other.m_MinKeys;
                     m_MaxKeysForChilds = other.m_MaxKeysForChilds;
                     m_Unique = other.m_Unique;
                     m_isRoot = other.m_isRoot;
                     m_KeyCount = other.m_KeyCount;
                     m_Parent = nullptr;  // El nodo movido no tiene padre
                     m_Compare = std::move(other.m_Compare);
                     m_Keys = std::move(other.m_Keys);
                     m_SubPages = std::move(other.m_SubPages);

                     // Actualizar referencias de padre en los hijos
                     for (size_t i = 0; i <= m_KeyCount; i++) {
                            if (m_SubPages[i]) {
                                   m_SubPages[i]->SetParent(this);
                            }
                     }

                     // Reset other to a valid but empty state
                     other.m_Parent = nullptr;
                     other.m_KeyCount = 0;
              }
              return *this;
       }

       bt_ErrorCode    Insert (const keyType &key, const ObjIDType ObjID);
       bt_ErrorCode    Remove (const keyType &key, const ObjIDType ObjID);
       bool            Search (const keyType &key, ObjIDType &ObjID);
       void            Print  (ostream &os);

       // TODO: #6 change by Invoke
       // TODO: #7 ForEach must be a template inside this template
       void            ForEach(lpfnForEach2 lpfn, size_t level, void *pExtra1);
       void            ForEach(lpfnForEach3 lpfn, size_t level, void *pExtra1, void *pExtra2);

       // TODO: #8 You may reduce these two function by using Invoke
       ObjectInfo*     FirstThat(lpfnFirstThat2 lpfn, size_t level, void *pExtra1);
       ObjectInfo*     FirstThat(lpfnFirstThat3 lpfn, size_t level, void *pExtra1, void *pExtra2);

protected:
 
       size_t  m_MinKeys; // minimum number of keys in a node
       size_t  m_MaxKeys, // maximum number of keys in a node

                m_MaxKeysForChilds; // just to distinguish the root
       bool m_Unique;
       bool m_isRoot;
       //size_t           NextNode; // address of next node at same level
       //size_t RecAddr; // address of this node in the BTree file
       vector<ObjectInfo> m_Keys;
       vector<BTPage *>m_SubPages;
       Compare m_Compare;
       BTPage* m_Parent;

       size_t  m_KeyCount;

       bool isEqual(const keyType& a, const keyType& b) const
       { return !m_Compare(a, b) && !m_Compare(b, a); }

       bool isLess(const keyType& a, const keyType& b) const
       { return m_Compare(a, b); }

       bool isGreater(const keyType& a, const keyType& b) const
       { return m_Compare(b, a); }

       bool isLessOrEqual(const keyType& a, const keyType& b) const
       { return !m_Compare(b, a); }

       void  Create();
       void  Reset ();
       void  Destroy () {   Reset(); delete this;}
       void  clear ();

       BTPage* GetParent() const { return m_Parent; }
       void SetParent(BTPage* parent) { m_Parent = parent; }

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
CBTreePage<Trait>:: CBTreePage(size_t maxKeys, bool unique)
                               : m_MaxKeys(maxKeys), m_Unique(unique), m_Compare(Compare()), m_Parent(nullptr), m_KeyCount(0)
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
       size_t pos = binary_search(m_Keys, 0, m_KeyCount, key, m_Compare);
       bt_ErrorCode error = bt_ok;

       if( pos < m_KeyCount && isEqual((keyType)m_Keys[pos], key) && m_Unique)
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
               
               // Actualizar padre del nodo movido
               if (pSource->m_SubPages[0])
                       pSource->m_SubPages[0]->SetParent(pTarget);

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
               
               // Actualizar padre del nodo movido
               if (pSource->m_SubPages[pSource->NumberOfKeys()])
                       pSource->m_SubPages[pSource->NumberOfKeys()]->SetParent(pTarget);
               
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
       pChild1->SetParent(this);

       // copy the second element to the root
       ::insert_at(m_Keys, oi2, pos+1);
       ::insert_at(m_SubPages, pChild2, pos+1);
       NumberOfKeys()++;

       m_SubPages[pos+2] = pChild3;
       pChild2->SetParent(this);
       pChild3->SetParent(this);
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
               if( tmpSubPages[i] )
                       tmpSubPages[i]->SetParent(pChild1);
               pChild1->NumberOfKeys()++;
       }
       pChild1->m_SubPages[i] = tmpSubPages[i];
       if( tmpSubPages[i] )
               tmpSubPages[i]->SetParent(pChild1);

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
               if( tmpSubPages[i] )
                       tmpSubPages[i]->SetParent(pChild2);
               pChild2->NumberOfKeys()++;
       }
       pChild2->m_SubPages[j] = tmpSubPages[i];
       if( tmpSubPages[i] )
               tmpSubPages[i]->SetParent(pChild2);

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
               if( tmpSubPages[i] )
                       tmpSubPages[i]->SetParent(pChild3);
               pChild3->NumberOfKeys()++;
       }
       pChild3->m_SubPages[j] = tmpSubPages[i];
       if( tmpSubPages[i] )
               tmpSubPages[i]->SetParent(pChild3);
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
       pChild1->SetParent(this);
       NumberOfKeys()++;

       // copy the second element to the root
       m_Keys    [1] = oi2;
       m_SubPages[1] = pChild2;
       pChild2->SetParent(this);
       NumberOfKeys()++;

       m_SubPages[2] = pChild3;
       pChild3->SetParent(this);
       return true;
}

template <typename Trait>
bool CBTreePage<Trait>::Search(const keyType &key, ObjIDType &ObjID)
{
       size_t pos = binary_search(m_Keys, 0, m_KeyCount, key, m_Compare);
       if( pos >= m_KeyCount )
       {    if( m_SubPages[pos] )
                return m_SubPages[pos]->Search(key, ObjID);
            else
                return false;
       }
       if( isEqual(key, m_Keys[pos].key) )
       {
               ObjID = m_Keys[pos].ObjID;
               m_Keys[pos].UseCounter++;
               return true;
       }
       if( isLess(key, m_Keys[pos].key) )
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

template <typename Trait>
void CBTreePage<Trait>::ForEach(lpfnForEach2 lpfn, size_t level, void *pExtra1)
{
       for(size_t i = 0 ; i < m_KeyCount ; i++)
       {
               if( m_SubPages[i] )
                       m_SubPages[i]->ForEach(lpfn, level+1, pExtra1);
               lpfn(m_Keys[i], level, pExtra1);
       }
       if( m_SubPages[m_KeyCount] )
               m_SubPages[m_KeyCount]->ForEach(lpfn, level+1, pExtra1);
}

/*template <typename keyType, typename ObjIDType>
void CBTreePage<keyType, ObjIDType>::ForEachReverse(lpfnForEach3 lpfn,
                                                                                                       size_t level, void *pExtra1, void *pExtra2)
{
       if( m_SubPages[m_KeyCount] )
               m_SubPages[m_KeyCount]->ForEach(lpfn, level+1, pExtra1, pExtra2);
       for(size_t i = m_KeyCount-1 ; i >= 0  ; i--)
       {
               lpfn(m_Keys[i], level, pExtra1, pExtra2);
               if( m_SubPages[i] )
                       m_SubPages[i]->ForEach(lpfn, level+1, pExtra1, pExtra2);
       }
}*/

template <typename Trait>
void CBTreePage<Trait>::ForEach(lpfnForEach3 lpfn, size_t level, void *pExtra1, void *pExtra2)
{
       for(size_t i = 0 ; i < m_KeyCount ; i++)
       {
               if( m_SubPages[i] )
                       m_SubPages[i]->ForEach(lpfn, level+1, pExtra1, pExtra2);
               lpfn(m_Keys[i], level, pExtra1, pExtra2);
       }
       if( m_SubPages[m_KeyCount] )
               m_SubPages[m_KeyCount]->ForEach(lpfn, level+1, pExtra1, pExtra2);
}

// Apicar una funcion hasta encontrar el 1er elemento
// aque que retorne true ante esta funcion
template <typename Trait>
typename CBTreePage<Trait>::ObjectInfo *
CBTreePage<Trait>::FirstThat(lpfnFirstThat2 lpfn, size_t level, void *pExtra1)
{
       ObjectInfo *pTmp;
       for(size_t i = 0 ; i < m_KeyCount ; i++)
       {
               if( m_SubPages[i] )
                       if( (pTmp = m_SubPages[i]->FirstThat(lpfn, level+1, pExtra1)) )
                               return pTmp;
               if( lpfn(m_Keys[i], level, pExtra1) )
                       return &m_Keys[i];
       }
       if( m_SubPages[m_KeyCount] )
               if( (pTmp = m_SubPages[m_KeyCount]->FirstThat(lpfn, level+1, pExtra1)) )
                       return pTmp;
       return 0;
}

template <typename Trait>
typename CBTreePage<Trait>::ObjectInfo *
CBTreePage<Trait>::FirstThat(lpfnFirstThat3 lpfn,size_t level, void *pExtra1, void *pExtra2)
{
       ObjectInfo *pTmp;
       for(size_t i = 0 ; i < m_KeyCount ; i++)
       {
               if( m_SubPages[i] )
                       if( (pTmp = m_SubPages[i]->FirstThat(lpfn, level+1, pExtra1, pExtra2) ) )
                               return pTmp;
               if( lpfn(m_Keys[i], level, pExtra1, pExtra2) )
                       return &m_Keys[i];
       }
       if( m_SubPages[m_KeyCount] )
               if( (pTmp = m_SubPages[m_KeyCount]->FirstThat(lpfn, level+1, pExtra1, pExtra2) ) )
                       return pTmp;
       return 0;
}

template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::Remove(const keyType &key, const ObjIDType ObjID)
{
       bt_ErrorCode error = bt_ok;
       size_t pos = binary_search(m_Keys, 0, m_KeyCount, key, m_Compare);
       if( pos < NumberOfKeys() && isEqual(key, m_Keys[pos].key) /*&& m_Keys[pos].m_ObjID == ObjID*/) // We found it !
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
       else if( isLessOrEqual(key, m_Keys[pos].key) ) // = is because identical keys are inserted on left (see Insert)
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
               // Reestablecer padre
               if (tmpSubPages[i])
                       tmpSubPages[i]->SetParent(pChild1);
               pChild1->NumberOfKeys()++;
       }
       pChild1->m_SubPages[i] = tmpSubPages[i];
       if (tmpSubPages[i])
               tmpSubPages[i]->SetParent(pChild1);

       m_Keys    [pos-1] = tmpKeys[i];
       m_SubPages[pos-1] = pChild1;

       ::remove(m_Keys    , pos);
       ::remove(m_SubPages, pos);
       NumberOfKeys()--;

       nKeys = pChild2->GetFreeCells();


       size_t j = ++i;
       for(i = 0 ; i < nKeys ; i++, j++ )
       {
               pChild2->m_Keys    [i] = tmpKeys    [j];
               pChild2->m_SubPages[i] = tmpSubPages[j];
               // Reestablecer padre
               if (tmpSubPages[j])
                       tmpSubPages[j]->SetParent(pChild2);
               pChild2->NumberOfKeys()++;
       }
       pChild2->m_SubPages[i] = tmpSubPages[j];
       if (tmpSubPages[j])
               tmpSubPages[j]->SetParent(pChild2);
       m_SubPages[ pos ]          = pChild2;

       if( Underflow() )
               return bt_underflow;
       return bt_ok;
}

template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::MergeRoot()
{

       size_t pos = 1;
       assert( m_SubPages[pos-1]->NumberOfKeys() +
                       m_SubPages[ pos ]->NumberOfKeys() +
                       m_SubPages[pos+1]->NumberOfKeys() ==
                       3*m_SubPages[ pos ]->MinNumberOfKeys() - 1);

       BTPage  *pChild1 = m_SubPages[pos-1], *pChild2 = m_SubPages[ pos ], *pChild3 = m_SubPages[pos+1];
 
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
               // Reestablecer padre a la raíz
               if (tmpSubPages[i])
                       tmpSubPages[i]->SetParent(this);
               NumberOfKeys()++;
       }
       m_SubPages[i] = tmpSubPages[i];
       if (tmpSubPages[i])
               tmpSubPages[i]->SetParent(this);

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
       lpfnForEach2 lpfn = &::Print<keyType, ObjIDType>;
       ForEach(lpfn, 0, &os);
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
       size_t nKeys = pChildPage->GetNumberOfKeys();
       size_t i = 0;
       for(i = 0; i < nKeys; i++ )
       {
                tmpKeys    .push_back(pChildPage->m_Keys[i]);
                tmpSubPages.push_back(pChildPage->m_SubPages[i]);
                // Mantener referencias de padre durante el movimiento temporal
                if (pChildPage->m_SubPages[i])
                       pChildPage->m_SubPages[i]->SetParent(nullptr);
       }
       tmpSubPages.push_back(pChildPage->m_SubPages[i]);
       if (pChildPage->m_SubPages[i])
               pChildPage->m_SubPages[i]->SetParent(nullptr);
       pChildPage->clear();
}

// Forward iterator: recorre el árbol en orden ascendente (in-order traversal)
template <typename Trait>
class forward_btree_iterator
{
private:
       using value_type = typename BTree<Trait>::ObjectInfo;
       using BTPage     = typename BTree<Trait>::BTNode;
       using iterator   = forward_btree_iterator<Trait>;

       BTree<Trait> *m_pTree = nullptr;
       BTPage       *m_pPage = nullptr;
       size_t        m_Index = 0;

public:
       forward_btree_iterator(BTree<Trait> *pTree, BTPage *pPage, size_t index = 0)
               : m_pTree(pTree), m_pPage(pPage), m_Index(index)
       {
               // Si estamos en un nodo interno, ir a la primera hoja
               if (m_pPage && m_pPage->m_SubPages[0])
                       goToFirstLeaf();
       }

       forward_btree_iterator(const iterator &other)
               : m_pTree(other.m_pTree), m_pPage(other.m_pPage), m_Index(other.m_Index)
       {}

       bool operator==(const iterator& other) const {
               return m_pTree == other.m_pTree &&
                      m_pPage == other.m_pPage &&
                      m_Index == other.m_Index;
       }

       bool operator!=(const iterator& other) const {
               return !(*this == other);
       }

       // Operador de avance (SOLO FORWARD)
       iterator operator++() {
               if (!m_pPage)
                       return *this;

               // Si hay subárbol derecho, ir al mínimo de ese subárbol
               if (m_pPage->m_SubPages[m_Index + 1]) {
                       m_pPage = m_pPage->m_SubPages[m_Index + 1];
                       m_Index = 0;
                       goToFirstLeaf();
               }
               // Si hay más claves en este nodo, avanzar al siguiente
               else if (m_Index + 1 < m_pPage->GetNumberOfKeys()) {
                       m_Index++;
               }
               // Subir al padre
               else {
                       goToNextInParent();
               }

               return *this;
       }

       // Post-incremento
       iterator operator++(int) {
               iterator tmp = *this;
               ++(*this);
               return tmp;
       }

       value_type& operator*() const {
               return m_pPage->m_Keys[m_Index];
       }

       value_type* operator->() const {
               return &m_pPage->m_Keys[m_Index];
       }

private:
       void goToFirstLeaf() {
               while (m_pPage && m_pPage->m_SubPages[0]) {
                       m_pPage = m_pPage->m_SubPages[0];
                       m_Index = 0;
               }
       }

       void goToNextInParent() {
               BTPage* child = m_pPage;
               m_pPage = m_pPage->GetParent();

               while (m_pPage) {
                       for (size_t i = 0; i <= m_pPage->GetNumberOfKeys(); i++) {
                               if (m_pPage->m_SubPages[i] == child) {
                                       if (i < m_pPage->GetNumberOfKeys()) {
                                               m_Index = i;
                                               return;
                                       }
                                       break;
                               }
                       }
                       child = m_pPage;
                       m_pPage = m_pPage->GetParent();
               }

               m_pPage = nullptr;
               m_Index = 0;
       }
};

// Backward iterator: recorre el árbol en orden descendente (reverse in-order traversal)
template <typename Trait>
class backward_btree_iterator
{
private:
       using value_type = typename BTree<Trait>::ObjectInfo;
       using BTPage     = typename BTree<Trait>::BTNode;
       using iterator   = backward_btree_iterator<Trait>;

       BTree<Trait> *m_pTree = nullptr;
       BTPage       *m_pPage = nullptr;
       size_t        m_Index = 0;

public:
       backward_btree_iterator(BTree<Trait> *pTree, BTPage *pPage, size_t index = 0)
               : m_pTree(pTree), m_pPage(pPage), m_Index(index)
       {
               // Si estamos en un nodo interno, ir a la última hoja
               if (m_pPage && m_pPage->m_SubPages[0])
                       goToLastLeaf();
       }

       backward_btree_iterator(const iterator &other)
               : m_pTree(other.m_pTree), m_pPage(other.m_pPage), m_Index(other.m_Index)
       {}

       bool operator==(const iterator& other) const {
               return m_pTree == other.m_pTree &&
                      m_pPage == other.m_pPage &&
                      m_Index == other.m_Index;
       }

       bool operator!=(const iterator& other) const {
               return !(*this == other);
       }

       // Operador de avance (avanza BACKWARD en el árbol)
       iterator operator++() {
               if (!m_pPage)
                       return *this;

               // Si hay subárbol izquierdo, ir al máximo de ese subárbol
               if (m_pPage->m_SubPages[m_Index]) {
                       m_pPage = m_pPage->m_SubPages[m_Index];
                       goToLastLeaf();
                       m_Index = m_pPage->GetNumberOfKeys() - 1;
               }
               // Si hay más claves a la izquierda en este nodo
               else if (m_Index > 0) {
                       m_Index--;
               }
               // Subir al padre
               else {
                       goToPrevInParent();
               }

               return *this;
       }

       // Post-incremento
       iterator operator++(int) {
               iterator tmp = *this;
               ++(*this);
               return tmp;
       }

       value_type& operator*() const {
               return m_pPage->m_Keys[m_Index];
       }

       value_type* operator->() const {
               return &m_pPage->m_Keys[m_Index];
       }

private:
       void goToLastLeaf() {
               while (m_pPage && m_pPage->m_SubPages[m_pPage->GetNumberOfKeys()]) {
                       m_pPage = m_pPage->m_SubPages[m_pPage->GetNumberOfKeys()];
               }
       }

       void goToPrevInParent() {
               BTPage* child = m_pPage;
               m_pPage = m_pPage->GetParent();

               while (m_pPage) {
                       for (size_t i = 0; i <= m_pPage->GetNumberOfKeys(); i++) {
                               if (m_pPage->m_SubPages[i] == child) {
                                       if (i > 0) {
                                               m_Index = i - 1;
                                               return;
                                       }
                                       break;
                               }
                       }
                       child = m_pPage;
                       m_pPage = m_pPage->GetParent();
               }

               m_pPage = nullptr;
               m_Index = 0;
       }
};

#endif