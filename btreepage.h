#ifndef __CBTreePage_H__
#define __CBTreePage_H__

#include <vector>
#include <assert.h>
#include <functional>

// TODO: #1 Crear una function para agregarla al demo.cpp ( no trivial )
// TODO: #2 Agregarle un Trait (prueba git) ( no trivial )
// TODO: #3 crear un iterator ( no trivial )
//       Sugerencia: Tarea1 cada pagina debe tener un puntero al padre primero ( no trivial )
// TODO: #4 integrarlo al recorrer ( no trivial )


template <typename Trait>
class BTree;

using namespace std;
/// @brief 
enum bt_ErrorCode {bt_ok, bt_overflow, bt_underflow, bt_duplicate, bt_nofound, bt_rootmerged};

/// @brief Auxiliar binary search function to use in some operations of the BTree.
/// @tparam Container The Container Type that we are analyzing.
/// @tparam ObjType The Type of the object contained in the BTree.
/// @tparam Compare The Function Type that we will use for comparations.
/// @param container The proper container.
/// @param first The first index.
/// @param last The last index.
/// @param object The object we are searching.
/// @param comp The comparation function.
/// @return 
template <typename Container, typename ObjType, typename Compare>
size_t binary_search(Container& container, size_t first, size_t last, ObjType &object, Compare comp) {
	if( first >= last )
		return first;
	while( first < last ) {
		size_t mid = (first+last)/2;
		if(!comp(container[mid], object) && !comp(object, container[mid]))
			return mid;
		if( comp(container[mid], object) )
			first = mid+1;
		else
			last  = mid;
	}
	if(!comp(object, container[first]))
		return first;
	return last;
}

// Error al poner size_t
// Posible motivo: El i está disminuyendo
/// @brief Auxiliary function used in some process of the BTree.
/// @tparam Container The Container Type that we are analyzing.
/// @tparam ObjType The Type of the object contained in the BTree.
/// @param container The proper container.
/// @param object The object we are inserting.
/// @param pos The position of the insertion.
template <typename Container, typename ObjType>
void insert_at(Container& container, ObjType object, int pos) {
	size_t size = container.size();
	for(int i = size-2 ; i >= pos ; i--)
		container[i+1] = container[i];
	container[pos] =  object;	
}

/// @brief Auxiliary function to remove an element.
/// @tparam Container The Container Type that we are analyzing.
/// @param container The proper container.
/// @param pos The position of the removal.
template <typename Container>
void remove(Container& container, size_t pos) {
	size_t size = container.size();
	for(auto i = pos+1 ; i < size ; i++)
	container[i-1] = container[i];
}

/// @brief Auxiliary struct that stores information of an object of the BTree.
/// @tparam keyType The type of the key used.
/// @tparam ObjIDType The type of the objectID used.
template <typename keyType, typename ObjIDType>
struct tagObjectInfo {
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

/// @brief Class that encapsulates the logic behind the BTreePage.
/// @tparam Trait Encapsulation of the types that will ise the BTree.
template <typename Trait>
class CBTreePage {
	friend class BTree<Trait>;
	typedef typename Trait::keyType  keyType;
	typedef typename Trait::ObjIDType  ObjIDType;
	typedef typename Trait::CompareFn CompareFn;

	typedef CBTreePage<Trait>    BTPage;         // useful shorthand
	typedef tagObjectInfo<keyType, ObjIDType> ObjectInfo;

	
 public:
	/// @brief Constructor of the BTreePage.
	/// @param maxKeys Max keys that hold this page.
	/// @param unique Boleean to indicate if the values will be unique.
	CBTreePage(size_t maxKeys, bool unique = true);

	/// @brief Default Destructor
	virtual ~CBTreePage();

	/// @brief Add a new value recursively and balance the tree.
	/// @param key The key to add.
	/// @param ObjID The ObjectID to add.
	/// @return Returns the status of the insertion to the parent.
	bt_ErrorCode    Insert (const keyType &key, const ObjIDType ObjID);

	/// @brief Remove a new value recursively and balance the tree.
	/// @param key The key to remove.
	/// @param ObjID The ObjectID to remove.
	/// @return Returns the status of the deletion to the parent.
	bt_ErrorCode    Remove (const keyType &key, const ObjIDType ObjID);
	   
	/// @brief Searchs a value recursively.
	/// @param key The key to search.
	/// @param ObjID The ObjectID to search.
	/// @return True if its found, false if not.
	bool            Search (const keyType &key, ObjIDType &ObjID);

	/// @brief Prints the string form of the page.
	/// @param os The output stream.
	void            Print  (ostream &os);

	// TODO: #6 change by Invoke
	// TODO: #7 ForEach must be a template inside this template
	// ForEach generalizado
	template <typename Func, typename... Args>
	void ForEach(Func&& func, size_t level, Args&&... args) {
		for (size_t i = 0; i < m_KeyCount; i++) {
			if (m_SubPages[i])
				m_SubPages[i]->ForEach(std::forward<Func>(func), level + 1, 
									std::forward<Args>(args)...);
			
			std::invoke(std::forward<Func>(func), m_Keys[i], level, 
					std::forward<Args>(args)...);
		}
		
		if (m_SubPages[m_KeyCount])
			m_SubPages[m_KeyCount]->ForEach(std::forward<Func>(func), level + 1, 
										std::forward<Args>(args)...);
	}

	// FirstThat generalizado
	template <typename Pred, typename... Args>
	ObjectInfo* FirstThat(Pred&& pred, size_t level, Args&&... args) {
		ObjectInfo* pTmp = nullptr;
		
		for (size_t i = 0; i < m_KeyCount; i++) {
			if (m_SubPages[i])
				if ((pTmp = m_SubPages[i]->FirstThat(std::forward<Pred>(pred), 
													level + 1, 
													std::forward<Args>(args)...)))
					return pTmp;
			
			if (std::invoke(std::forward<Pred>(pred), m_Keys[i], level, 
						std::forward<Args>(args)...))
				return &m_Keys[i];
		}
		
		if (m_SubPages[m_KeyCount])
			if ((pTmp = m_SubPages[m_KeyCount]->FirstThat(std::forward<Pred>(pred), 
														level + 1, 
														std::forward<Args>(args)...)))
				return pTmp;
		
		return nullptr;
	}

protected:
	CompareFn	Compfn;
	size_t  m_MinKeys; // minimum number of keys in a node
	size_t  m_MaxKeys, // maximum number of keys in a node

	m_MaxKeysForChilds; // just to distinguish the root
	bool m_Unique;
	bool m_isRoot;
	//size_t           NextNode; // address of next node at same level
	//size_t RecAddr; // address of this node in the BTree file
	vector<ObjectInfo> m_Keys;
	vector<BTPage *>m_SubPages;
	
	size_t  m_KeyCount;
	/// @brief Creates a new empty page with default values.
	void  Create();

	/// @brief Resets this page and remove children and keys.
	void  Reset ();

	/// @brief Destroy this page.
	void  Destroy () {   Reset(); delete this;}

	/// @brief Clears the keys of the page.
	void  clear ();

	/// @brief Function that redistributes the keys with 1 brother to balance the tree.
	/// @param pos Position of the child that have the problem.
	/// @return true if the problem is fixed, if not, returns false.
	bool  RedistributeWith1Brother   (size_t &pos);

	/// @brief Function that redistributes the keys with 2 brother to balance the tree.
	/// @param pos Position of the child that have the problem.
	/// @return true if the problem is fixed, if not, returns false.
	bool  RedistributeWith2Brothers   (size_t pos);

	/// @brief Redistributes from right to left for balancing.
	/// @param pos 
	void  RedistributeR2L (size_t pos);

	/// @brief Redistributes from left to right for balancing.
	/// @param pos 
	void  RedistributeL2R (size_t pos);

	/// @brief Treat the problems of a position.
	/// @param pos The position to balance.
	/// @return true if the problem is fixed, if not returns false.
	bool    TreatUnderflow  (size_t &pos) {
		return RedistributeWith1Brother(pos) || RedistributeWith2Brothers(pos);
	}

	/// @brief Merges the child page at the given position with one of its siblings.
	/// @param pos Index of the child page to merge.
	/// @return Returns the error code indicating the result of the merge.
	bt_ErrorCode    Merge  (size_t pos);

	/// @brief Merges the root node when it becomes empty or has only one child after a deletion.
	/// @return Returns the error code indicating the result of the merge.
	bt_ErrorCode    MergeRoot ();

	/// @brief Splits a child node when it overflows, promoting the median key to the parent.
	/// @param pos Index of the child to split.
	void  SplitChild (size_t pos);

	/// @brief Retrieves a reference to the first key (and its associated object) 
	///        in the subtree rooted at this node.
	/// @return Reference to the first ObjectInfo in sorted order.
	ObjectInfo &GetFirstObjectInfo();

	/// @brief Checks whether the node has more keys than allowed.
	/// @return true if the node exceeds its maximum capacity, false otherwise.
	bool Overflow()  { return m_KeyCount > m_MaxKeys; }
	

	/// @brief Checks whether the node has fewer keys than the minimum allowed.
	/// @return true if the node underflows, false otherwise.
	bool Underflow() { return m_KeyCount < MinNumberOfKeys(); }

	/// @brief Checks whether the node has reached its full capacity.
	/// @return true if the node is full, false otherwise.
	bool IsFull()    { return m_KeyCount >= m_MaxKeys; }

	/// @brief Computes the minimum number of keys allowed in this node.
	/// @return Minimum number of keys before an underflow occurs.
	size_t  MinNumberOfKeys()  { return 2*m_MaxKeys/3.0; }
	
	/// @brief Calculates how many key slots are still available in this node.
	/// @return The number of free key slots.
	size_t  GetFreeCells()  { return m_MaxKeys - m_KeyCount; }

	/// @brief Returns a reference to the current number of keys.
	/// @return Reference to the internal counter of keys in this node.
	size_t& NumberOfKeys()  { return m_KeyCount; }

	/// @brief Returns the current number of keys (read-only).
	/// @return Current number of keys stored in this node.
	size_t  GetNumberOfKeys()  { return m_KeyCount; }

	/// @brief Checks if this node is the root of the B-tree.
	/// @return true if the node is the root, false otherwise.
	bool IsRoot()  { return m_MaxKeysForChilds != m_MaxKeys; }

	/// @brief Sets the maximum number of keys allowed for child nodes of this node.
	/// @param orderforchilds Maximum number of keys for the children.
	void SetMaxKeysForChilds(size_t orderforchilds) { m_MaxKeysForChilds = orderforchilds; }

	/// @brief Returns how many free key slots exist in the left sibling of a given position.
	/// @param pos Index of the current child in the parent.
	/// @return Number of free key slots in the left sibling, or 0 if none exists.
	size_t GetFreeCellsOnLeft(size_t pos) {
		if( pos > 0 )                                   // there is some page on left ?
			return m_SubPages[pos-1]->GetFreeCells();
		return 0;
	}
	
	/// @brief Returns how many free key slots exist in the right sibling of a given position.
	/// @param pos Index of the current child in the parent.
	/// @return Number of free key slots in the right sibling, or 0 if none exists.
	size_t GetFreeCellsOnRight(size_t pos){    
		if( pos < GetNumberOfKeys() )   // there is some page on right ?
			return m_SubPages[pos+1]->GetFreeCells();
		return 0;
	}

private:
	/// @brief Splits the root node when it overflows, creating a new root and two children.
	/// @return true if the split was successful, false otherwise.
	bool SplitRoot();

	/// @brief Splits a node into three parts (used when reorganizing after overflow or merging).
	/// @param tmpKeys Temporary vector of keys to redistribute.
	/// @param SubPages Temporary vector of child pointers.
	/// @param pChild1 Output pointer to the first resulting child.
	/// @param pChild2 Output pointer to the second resulting child.
	/// @param pChild3 Output pointer to the third resulting child.
	/// @param oi1 Output median key promoted to parent (between child 1 and 2).
	/// @param oi2 Output median key promoted to parent (between child 2 and 3).
	void SplitPageInto3(
		vector<ObjectInfo>   & tmpKeys,
		vector<BTPage *>  & SubPages,
		BTPage           *& pChild1,
		BTPage           *& pChild2,
		BTPage           *& pChild3,
		ObjectInfo        & oi1,
		ObjectInfo        & oi2
	);

	/// @brief Moves all keys and child pointers from one page into temporary vectors.
	/// @param pChildPage Source child page to extract from.
	/// @param tmpKeys Destination vector for the extracted keys.
	/// @param tmpSubPages Destination vector for the extracted child pointers.
	void MovePage(BTPage *  pChildPage,vector<ObjectInfo> & tmpKeys,vector<BTPage *> & tmpSubPages);
};

template <typename Trait>
CBTreePage<Trait>:: CBTreePage(size_t maxKeys, bool unique)
	: m_MaxKeys(maxKeys), m_Unique(unique), m_KeyCount(0) {
	Create();
	SetMaxKeysForChilds(m_MaxKeys);
}

template <typename Trait>
CBTreePage<Trait>::~CBTreePage() {
	Reset();
}

template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::Insert(const keyType& key, const ObjIDType ObjID){
	size_t pos = binary_search(m_Keys, 0, m_KeyCount, key, Compfn);
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
		pChild1->NumberOfKeys()++;
	}
	pChild1->m_SubPages[i] = tmpSubPages[i];

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
		pChild2->NumberOfKeys()++;
	}
	pChild2->m_SubPages[j] = tmpSubPages[i];

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
	size_t pos = binary_search(m_Keys, 0, m_KeyCount, key, Compfn);
	if( pos >= m_KeyCount ) {
		if( m_SubPages[pos] )
			return m_SubPages[pos]->Search(key, ObjID);
		else
			return false;
	}
	
	if(!Compfn(m_Keys[pos].key, key) && !Compfn(key, m_Keys[pos].key))
	{
		ObjID = m_Keys[pos].ObjID;
		m_Keys[pos].UseCounter++;
		return true;
	}
	
	if(Compfn(key, m_Keys[pos].key))
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



// Apicar una funcion hasta encontrar el 1er elemento
// aque que retorne true ante esta funcion



template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::Remove(const keyType &key, const ObjIDType ObjID)
{
	bt_ErrorCode error = bt_ok;
	size_t pos = binary_search(m_Keys, 0, m_KeyCount, key, Compfn);
	if( pos < NumberOfKeys() && !Compfn(m_Keys[pos].key, key) && !Compfn(key, m_Keys[pos].key) /*&& m_Keys[pos].m_ObjID == ObjID*/) // We found it !
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
		pChild1->NumberOfKeys()++;
	}
	pChild1->m_SubPages[i] = tmpSubPages[i];

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

template <typename keyType, typename ObjIDType>
void Print(tagObjectInfo<keyType, ObjIDType> &info, size_t level, void *pExtra)
{
	ostream &os = *(ostream *)pExtra;
	for(size_t i = 0; i < level ; i++)
		os << "\t";
	os << info.key << "->" << info.ObjID << "\n";
}

template <typename Trait>
void CBTreePage<Trait>::Print(ostream & os) {
    ForEach(::Print<keyType, ObjIDType>, 0, &os);
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
	}
	tmpSubPages.push_back(pChildPage->m_SubPages[i]);
	pChildPage->clear();
}
// 
//  FORWARD ITERATOR  (operator++ avanza hacia adelante)


	template<typename Trait>
	class BTreeForwardIterator {
	public:
		using BTPage = CBTreePage<Trait>;
		using ObjectInfo = typename BTPage::ObjectInfo;

	private:
		BTPage* current_page;
		size_t current_index;
		std::vector<std::pair<BTPage*, size_t>> stack;

	public:
		BTreeForwardIterator(BTPage* page = nullptr, size_t idx = 0)
			: current_page(page), current_index(idx) 
		{
			if (page) goToFirst();
		}

		void goToFirst() {
			while (current_page && current_page->m_SubPages[0]) {
				stack.push_back({current_page, 0});
				current_page = current_page->m_SubPages[0];
			}
			current_index = 0;
		}

		void goToLast() {
			while (current_page && current_page->m_SubPages[current_page->m_KeyCount]) {
				stack.push_back({current_page, current_page->m_KeyCount});
				current_page = current_page->m_SubPages[current_page->m_KeyCount];
			}
			if (current_page)
				current_index = current_page->m_KeyCount - 1;
		}

		ObjectInfo& operator*() { return current_page->m_Keys[current_index]; }
		ObjectInfo* operator->() { return &(current_page->m_Keys[current_index]); }

	
		BTreeForwardIterator& operator++() {
			if (!current_page) return *this;

			// Si hay hijo derecho
			if (current_page->m_SubPages[current_index + 1]) {
				stack.push_back({current_page, current_index});
				current_page = current_page->m_SubPages[current_index + 1];

				while (current_page->m_SubPages[0]) {
					stack.push_back({current_page, 0});
					current_page = current_page->m_SubPages[0];
				}
				current_index = 0;
			}
			// Avanzar en la misma página
			else if (current_index + 1 < current_page->m_KeyCount) {
				current_index++;
			}
			// Subir al padre
			else {
				if (stack.empty()) {
					current_page = nullptr;
				} else {
					auto parent = stack.back();
					stack.pop_back();
					current_page = parent.first;
					current_index = parent.second;
				}
			}

			return *this;
		}
	};


	//  BACKWARD ITERATOR (operator++ retrocede hacia atrás) 


	template<typename Trait>
	class BTreeBackwardIterator {
	public:
		using BTPage = CBTreePage<Trait>;
		using ObjectInfo = typename BTPage::ObjectInfo;

	private:
		BTPage* current_page;
		size_t current_index;
		std::vector<std::pair<BTPage*, size_t>> stack;

	public:
		BTreeBackwardIterator(BTPage* page = nullptr, size_t idx = 0)
			: current_page(page), current_index(idx)
		{
			if (page) goToLast();
		}

		void goToFirst() {
			while (current_page && current_page->m_SubPages[0]) {
				stack.push_back({current_page, 0});
				current_page = current_page->m_SubPages[0];
			}
			current_index = 0;
		}

		void goToLast() {
			while (current_page && current_page->m_SubPages[current_page->m_KeyCount]) {
				stack.push_back({current_page, current_page->m_KeyCount});
				current_page = current_page->m_SubPages[current_page->m_KeyCount];
			}
			if (current_page)
				current_index = current_page->m_KeyCount - 1;
		}

		ObjectInfo& operator*() { return current_page->m_Keys[current_index]; }
		ObjectInfo* operator->() { return &(current_page->m_Keys[current_index]); }

		
		BTreeBackwardIterator& operator++() {
			if (!current_page) return *this;

			// Si hay hijo izquierdo
			if (current_page->m_SubPages[current_index]) {
				stack.push_back({current_page, current_index});
				current_page = current_page->m_SubPages[current_index];

				while (current_page->m_SubPages[current_page->m_KeyCount]) {
					stack.push_back({current_page, current_page->m_KeyCount});
					current_page = current_page->m_SubPages[current_page->m_KeyCount];
				}

				current_index = current_page->m_KeyCount - 1;
			}
			// Retroceder en la misma página
			else if (current_index > 0) {
				current_index--;
			}
			// Subir al padre
			else {
				if (stack.empty()) {
					current_page = nullptr;
				} else {
					auto parent = stack.back();
					stack.pop_back();
					current_page = parent.first;
					current_index = parent.second;

					if (current_index > 0)
						current_index--;
				}
			}

			return *this;
		}
	};

    
    // Comparación
    bool operator==(const BTreeIterator& other) const {
        return current_page == other.current_page && 
               current_index == other.current_index;
    }
    
    bool operator!=(const BTreeIterator& other) const {
        return !(*this == other);
    }
    
    bool isValid() const {
        return current_page != nullptr;
    }
};
#endif