#ifndef __BINARY_TREE_H__  
#define __BINARY_TREE_H__ 
#include <iostream>
//#include <algorithm>
#include <cassert>
#include <fstream>
#include <vector>
#include <utility>
#include "types.h"
#include "generalIterator.h"
//#include "util.h"
using namespace std;

template <typename Traits>
class CBinaryTree;

template <typename Container>
class binary_tree_iterator;

template <typename Container>
class binary_tree_riterator;

template <typename Traits>
class CBinaryTreeNode{
friend class CBinaryTree<Traits>;
friend class binary_tree_iterator<CBinaryTree<Traits>>;
friend class binary_tree_riterator<CBinaryTree<Traits>>;
public:
	using value_type = typename Traits::T;
	using Node = CBinaryTreeNode<Traits>;

protected:
	value_type     m_data;
	Node          *m_pParent = nullptr;
	Ref            m_ref;
	vector<Node *> m_pChild  = {nullptr, nullptr}; // 2 hijos inicializados en nullptr

public:
	CBinaryTreeNode(Node*& pParent, value_type data, Ref ref, Node* p0 = nullptr, Node* p1 = nullptr)
		: m_pParent(pParent), m_data(data), m_ref(ref)
	{
		m_pChild[0] = p0;
		m_pChild[1] = p1;
	}
	~CBinaryTreeNode(){
		m_pChild[0] = nullptr;
		m_pChild[1] = nullptr;
	}

	value_type  getData()                { return m_data; }
	value_type &getDataRef()             { return m_data; }
	Ref getRef() { return m_ref; }

private:
	// 1 next to the right
	// 0 next to the left
	Node* getpNext(int direction){
		Node* current = this;

		if (current->m_pChild[direction]) {
			current = current->m_pChild[direction];
			while (current->m_pChild[direction ? 0 : 1])
				current = current->m_pChild[direction ? 0 : 1];
			return current;
		}

		Node* parent = current->m_pParent;
		while (parent && current == parent->m_pChild[direction]) {
			current = parent;
			parent = parent->m_pParent;
		}
		return parent;
	}
	
	void      setChild(Node *pChild, size_t pos)  {   m_pChild[pos] = pChild;  }
	Node    * getChild(size_t branch){ return m_pChild[branch];  }
	Node    *&getChildRef(size_t branch){ return m_pChild[branch];  }
	Node    * getParent() { return m_pParent;   }
};

template <typename Container>
class binary_tree_iterator : public general_iterator<Container,  class binary_tree_iterator<Container>> 
{
public:
	using Parent    = class general_iterator<Container, binary_tree_iterator<Container> >;;
	using Node      = typename Container::Node;

public:
	binary_tree_iterator(Container *pContainer, Node *pNode) : Parent (pContainer,pNode) {}
	binary_tree_iterator(Container &other)  : Parent (other) {}
	binary_tree_iterator(Container &&other) : Parent(other) {} // Move constructor C++11 en adelante

public:
	binary_tree_iterator operator++() {
		Parent::m_pNode = Parent::m_pNode ? (Node*)Parent::m_pNode->getpNext(1) : nullptr;
		return *this;
	}
};

template <typename Container>
class binary_tree_riterator : public general_iterator<Container,  class binary_tree_iterator<Container>> 
{
public:
	using Parent    = class general_iterator<Container, binary_tree_iterator<Container> >;;
	using Node      = typename Container::Node;

public:
	binary_tree_riterator(Container *pContainer, Node *pNode) : Parent (pContainer,pNode) {}
	binary_tree_riterator(Container &other)  : Parent (other) {}
	binary_tree_riterator(Container &&other) : Parent(other) {} // Move constructor C++11 en adelante

public:
	binary_tree_riterator operator++() {
		Parent::m_pNode = Parent::m_pNode ? (Node*)Parent::m_pNode->getpNext(0) : nullptr;
		return *this;
	}
};

template <typename _T>
struct BinaryTreeAscTraits{
	using  T         = _T;
	using  CompareFn = less<T>;
};

template <typename _T>
struct BinaryTreeDescTraits
{
	using  T         = _T;
	using  CompareFn = greater<T>;
};

template <typename Traits>
class CBinaryTree{
public:
	using value_type    = typename Traits::T;
	using Node          = CBinaryTreeNode<Traits>;
	
	using CompareFn     = typename Traits::CompareFn;
	using Container     = CBinaryTree<Traits>;
	using iterator      = binary_tree_iterator<Container>;
	using riterator 	= binary_tree_riterator<Container>;

	friend class CBinaryTreeNode<Traits>;
protected:
	Node    *m_pRoot = nullptr;
	size_t   m_size  = 0;
	CompareFn Compfn;
public: 
	size_t  size()  const       { return m_size;       }
	bool    empty() const       { return size() == 0;  }

	void insert(value_type elem, Ref ref) {
		internal_insert(elem, ref, nullptr, m_pRoot);
	}

	Node* getExtremeNode(Node* startNode, int direction) const {
		if (!startNode) return nullptr;
		
		Node* pNode = startNode;
		while (pNode->getChild(direction)) {
			pNode = pNode->getChild(direction);
		}
		return pNode;
	}

protected:
	Node* CreateNode(Node* pParent, value_type elem, Ref ref) {
		return new Node(pParent, elem, ref);
	}
	
	virtual Node* internal_insert(
		value_type elem,
		Ref ref,
		Node* pParent,
		Node*& rpOrigin)
	{
		if (!rpOrigin) {
			++m_size;
			rpOrigin = CreateNode(pParent, elem, ref);
			return rpOrigin;
		}
		size_t branch = Compfn(elem, rpOrigin->getDataRef()) ? 0 : 1;
		return internal_insert(elem, ref, rpOrigin, rpOrigin->getChildRef(branch));
	}

public:
	CBinaryTree(){} // Empty tree
	
	CBinaryTree(CBinaryTree<Traits> &other)
		:m_size(m_size), Compfn(other.Compfn) {
		
		m_pRoot = CopyConstructorAux(other.m_pRoot, nullptr);
	}

	Node* CopyConstructorAux(Node* otherNode, Node* parentNode){
		if(otherNode == nullptr) return nullptr;

		Node* newNode = CreateNode(parentNode, otherNode->getDataRef(), otherNode->getRef());
		newNode->setChild(CopyConstructorAux(otherNode->getChild(0), newNode), 0);
		newNode->setChild(CopyConstructorAux(otherNode->getChild(1), newNode), 1);
		return newNode;
	}
	
	CBinaryTree(CBinaryTree<Traits> &&other)
		: m_pRoot(std::exchange(other.m_pRoot, nullptr)), 
		m_size (std::exchange(other.m_size, 0)), 
		Compfn (std::exchange(other.Compfn, nullptr))
	{ }

	void DestroySubtree(Node* pNode) {
		if(!pNode) return;
		DestroySubtree(pNode->m_pChild[0]);
		DestroySubtree(pNode->m_pChild[1]);
		delete pNode;
	}
	virtual ~CBinaryTree(){ 
		DestroySubtree(m_pRoot);
		m_pRoot = nullptr;
		m_size = 0;
	} 
	
	iterator begin() { 
		if (!m_pRoot) return end();
		return iterator(this, getExtremeNode(m_pRoot, 0));
	}

	iterator end()   { return iterator(this, nullptr); }

	// TODO: begin debe comenzar el el nodo mas a la derecha (1)
	riterator rbegin(){ 
		if (!m_pRoot) return rend();
		return riterator(this, getExtremeNode(m_pRoot, 1));
	}
	riterator rend()  { return riterator(this, nullptr); }

	// TODO: Generalizar estos recorridos para recibir cualquier funcion
	// con una cantidad flexible de parametros con variadic templates
	// Google: C++ parameter packs cplusplus
	void inorder  (ostream &os)    {   inorder  (m_pRoot, 0, os);  }
	
	// TODO: 
	void inorder(Node  *pNode, size_t level, ostream &os){
		if( pNode ){
			//Node *pParent = pNode->getParent();
			inorder(pNode->getChild(0), level+1, os);
			os << " --> " << pNode->getDataRef();
			inorder(pNode->getChild(1), level+1, os);
		}
	}

	// TODO: Generalize this function by using iterators and apply any function
	void inorder(Node  *pNode, void (*visit) (value_type& item)){
		if( pNode ){   
			inorder(pNode->getChild(0), *visit);
			(*visit)(pNode->getDataRef());
			inorder(pNode->getChild(1), *visit);
		}
	}

	// Variadic templates (See foreach.h)
	template <typename Function, typename... Args>
	void postorder(Function func, Args const&... args)
	{    postorder(m_pRoot, 0, func, args...);}

	template <typename Function,typename... Args>
	void postorder(
		Node* pNode, 
		size_t level, 
		Function func, 
		Args const&... args) {

		if (pNode) {
			postorder(pNode->getChild(0), level + 1, func, args...);
			postorder(pNode->getChild(1), level + 1, func, args...);
			func(pNode, level); 
		}
	}
	
	// TODO: generalize this function to apply any function
	void postorder(Node  *pNode, size_t level, ostream &os){
		if( pNode ){   
			postorder(pNode->getChild(0), level+1, os);
			postorder(pNode->getChild(1), level+1, os);
			os << " --> " << pNode->getDataRef();
		}
	}

	// TODO: Generalize this function to apply any function
	void preorder (ostream &os)    {   preorder (m_pRoot, 0, os);  }
	
	// TODO: Generalize this function to apply any function
	void preorder(Node  *pNode, size_t level, ostream &os){
		if( pNode ){   
			os << " --> " << pNode->getDataRef();
			preorder(pNode->getChild(0), level+1, os);
			preorder(pNode->getChild(1), level+1, os);            
		}
	}

	void print    (ostream &os)    {   print    (m_pRoot, 0, os);  }
	// TODO: generalize this function to apply any function
	// Google: C++ parameter packs cplusplus
	void print(Node  *pNode, size_t level, ostream &os){
		if( pNode ){
			Node *pParent = pNode->getParent();
			print(pNode->getChild(1), level+1, os);
			for(size_t i = 0; i < level; ++i){
				os << string(" | ");
			}
			os << pNode->getDataRef() << "(" << (pParent?to_string(pParent->getData()):"Root") << ")" << endl;
			print(pNode->getChild(0), level + 1, os);
		}
	}

	// TODO: Tip: recorrer el arbol en preorden
	void Write(ostream &os) {
		preorder(os);
	}

	// TODO: Leer en el arbol desde un stream asumiendo que esta en preorden
	void Read(istream &is)  { }
};

// TODO: este operator << debe seguir estando fuera de la clase
template <typename Traits>
ostream & operator<<(std::ostream &os, CBinaryTree<Traits> &obj){
	os << "CBinaryTree with " << obj.size() << " elements.";
	obj.inorder(os);
	return os;
}

template <typename Traits>
istream & operator>>(istream &is, CBinaryTree<Traits> &obj){
	// Leer el arbol
	return is;
}

void DemoBinaryTree();

#endif // __BINARY_TREE_H__