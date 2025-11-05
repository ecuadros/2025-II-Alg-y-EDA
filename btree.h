#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <mutex>
#include <shared_mutex>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

template <typename _keyType, typename _ObjIDType, typename _CompareFn>
struct BTreeTrait
{
	using keyType = _keyType;
	using ObjIDType = _ObjIDType;
	using CompareFn = _CompareFn;
	// TODO: agregar funcion de comparacion
};

template <typename Trait>
class BTree // this is the full version of the BTree
{
	typedef typename Trait::keyType	keyType;
	typedef typename Trait::ObjIDType	ObjIDType;
	typedef typename Trait::CompareFn	CompareFn;
	
	typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
	//typedef ObjectInfo iterator;
	typedef typename BTNode::lpfnForEach2    lpfnForEach2;
	typedef typename BTNode::lpfnForEach3    lpfnForEach3;
	typedef typename BTNode::lpfnFirstThat2  lpfnFirstThat2;
	typedef typename BTNode::lpfnFirstThat3  lpfnFirstThat3;
	typedef typename BTNode::ObjectInfo      ObjectInfo;

protected:
	BTNode          m_Root;
	size_t          m_Height;  // height of tree
	size_t          m_Order;   // order of tree
	size_t          m_NumKeys; // number of keys
	bool            m_Unique;  // Accept the elements only once ?
	CompareFn		m_Compfn;
private:
	std::shared_mutex m_Mutex;

public:
	/// @brief Constructs a new BTree with a given order and uniqueness policy.
	/// @param order The maximum number of keys per node. Defaults to DEFAULT_BTREE_ORDER.
	/// @param unique If true, prevents duplicate keys from being inserted.
	BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
		: m_Order(order),
		  m_Root(2 * order  + 1, unique),
		  m_Unique(unique),
		  m_NumKeys(0)
	{
		m_Root.SetMaxKeysForChilds(order);
		m_Height = 1;
	}
	
	/// @brief Move constructor.
	///	 Transfers ownership of the resources from another BTree.
	/// @param other The BTree instance to move from.
	BTree(BTree &&other) {
		std::lock_guard<std::shared_mutex> lock(other.m_Mutex);
		m_Root = std::move(other.m_Root);
		m_Height = std::move(other.m_Height);
		m_Order = std::move(other.m_Order);
		m_NumKeys = std::move(other.m_NumKeys);
		m_Unique = std::move(other.m_Unique);
		m_Compfn = std::move(other.m_Compfn);
	}
	
	/// @brief Destructor.
	~BTree() {}
	//int           Open (char * name, int mode);
	//int           Create (char * name, int mode);
	//int           Close ();

	/// @brief Inserts a key and associated object ID into the tree.
	/// @param key The key to insert.
	/// @param ObjID The identifier associated with the key.
	/// @return true if the insertion was successful, if not, returs false.
	bool            Insert (const keyType key, const long ObjID);

	/// @brief Removes a key and its associated object ID from the tree.
	/// @param key The key to remove.
	/// @param ObjID The identifier associated with the key.
	/// @return true  if the key was successfully removed, if not, returns false.
	bool            Remove (const keyType key, const long ObjID);

	/// @brief Searches for a key in the tree.
	/// @param key The key to search for.
	/// @return The object ID associated with the key, or -1 if not found.
	ObjIDType       Search (const keyType key)
	{      
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		ObjIDType ObjID = -1;
		m_Root.Search(key, ObjID);
		return ObjID;
	}

	size_t size()  { 
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		return m_NumKeys; 
	}

	size_t height() { 
		std::shared_lock<std::shared_mutex> lock(m_Mutex); 
		return m_Height; 
	}

	size_t GetOrder() { 
		std::shared_lock<std::shared_mutex> lock(m_Mutex); 
		return m_Order; 
	}

	/// @brief Prints the structure of the tree to an output stream.
	/// @param os The output stream where the tree will be printed.
	void Print (ostream &os){ 
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		m_Root.Print(os); 
	}
	
	void ForEach( lpfnForEach2 lpfn, void *pExtra1 ){ 
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		m_Root.ForEach(lpfn, 0, pExtra1); 
	}
	void ForEach( lpfnForEach3 lpfn, void *pExtra1, void *pExtra2){
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		m_Root.ForEach(lpfn, 0, pExtra1, pExtra2);
	}

	ObjectInfo* FirstThat( lpfnFirstThat2 lpfn, void *pExtra1 ){
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		return m_Root.FirstThat(lpfn, 0, pExtra1);
	}
	
	ObjectInfo* FirstThat( lpfnFirstThat3 lpfn, void *pExtra1, void *pExtra2){ 
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		return m_Root.FirstThat(lpfn, 0, pExtra1, pExtra2);
	}
	//typedef               ObjectInfo iterator;

	void Write(ostream &os) { 
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		os << *this; 
	}

	friend std::ostream& operator<<(std::ostream &os, BTree<Trait> &obj);

};     

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const long ObjID){
	std::shared_lock<std::shared_mutex> lock(m_Mutex);
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
	std::shared_lock<std::shared_mutex> lock(m_Mutex);
	bt_ErrorCode error = m_Root.Remove(key, ObjID);
	if( error == bt_duplicate || error == bt_nofound )
		 return false;
	m_NumKeys--;

	if( error == bt_rootmerged )
		 m_Height--;
	return true;
}

template <typename Trait>
std::ostream& operator<<(std::ostream &os, BTree<Trait> &obj) {
	os << obj.m_Root.Print();
}

#endif