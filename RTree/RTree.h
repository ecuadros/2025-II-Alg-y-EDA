#pragma once
#include <iostream>
#include <vector>
#include <limits>
#include <functional>
#include "types.h"

struct Rect {
	float m_minX;
	float m_minY;
	float m_maxX;
	float m_maxY;

	float Area() const {
		return (m_maxX - m_minX) * (m_maxY - m_minY);
	}

	bool Intersects(const Rect& other) const {
		return !(m_maxX < other.m_minX || m_minX > other.m_maxX ||
				 m_maxY < other.m_minY || m_minY > other.m_maxY);
	}

	static Rect Combine(const Rect& a, const Rect& b) {
		return {
			std::min(a.m_minX, b.m_minX),
			std::min(a.m_minY, b.m_minY),
			std::max(a.m_maxX, b.m_maxX),
			std::max(a.m_maxY, b.m_maxY)
		};
	}
};

struct RTreeTraits {
	using value_type = Rect;
	static constexpr size_t M = 8;
	static constexpr size_t m = 4;
};

template <typename Traits>
class RTreeNode {
private:
	using value_type = typename Traits::value_type;
	using Node = RTreeNode<Traits>;

public:
	struct Entry {
		value_type mbr;
		Node*      child = nullptr;
		Ref        ref   = 0;   // solo válido en hojas
	};

private:
	bool m_isLeaf = true;
	std::vector<Entry> m_entries;
	Node* m_parent = nullptr;

public:
	RTreeNode(bool isLeaf = true) : m_isLeaf(isLeaf) {}

	bool IsLeaf() const { return m_isLeaf; }
	size_t Size() const { return m_entries.size(); }

	std::vector<Entry>& Entries() { return m_entries; }
	Node* GetParent() { return m_parent; }
	void  SetParent(Node* p) { m_parent = p; }

	Rect ComputeMBR() const {
		Rect r = m_entries[0].mbr;
		for (size_t i = 1; i < m_entries.size(); ++i)
			r = Rect::Combine(r, m_entries[i].mbr);
		return r;
	}
};

template <typename Traits>
class CRTree {
public:
	using value_type = typename Traits::value_type;
	using Node = RTreeNode<Traits>;
	using Entry = typename Node::Entry;

private:
	Node* m_pRoot = nullptr;

public:
	CRTree() {
		m_pRoot = new Node(true);
	}

	~CRTree() {
		Destroy(m_pRoot);
	}

	void Insert(const value_type& rect, Ref ref);
	void Delete(const value_type& rect, Ref ref);
	void RangeQuery(const Rect& query, std::vector<Ref>& result) const;

	std::ostream& Write(std::ostream& os);
	std::istream& Read (std::istream& is);
	void WriteToFile();
	void ReadFromFile();

private:
	Node* ChooseLeaf(Node* node, const Rect& rect);
	void  AdjustTree(Node* node);
	void  SplitNode(Node* node);
	void  Destroy(Node* node);
};

template <typename Traits>
void CRTree<Traits>::Insert(const value_type& rect, Ref ref) {
	Node* leaf = ChooseLeaf(m_pRoot, rect);
	leaf->Entries().push_back({ rect, nullptr, ref });

	if (leaf->Size() > Traits::M)
		SplitNode(leaf);

	AdjustTree(leaf);
}

template <typename Traits>
typename CRTree<Traits>::Node*
CRTree<Traits>::ChooseLeaf(Node* node, const Rect& rect) {
	if (node->IsLeaf())
		return node;

	float bestIncrease = std::numeric_limits<float>::max();
	Node* bestChild = nullptr;

	for (auto& e : node->Entries()) {
		Rect combined = Rect::Combine(e.mbr, rect);
		float increase = combined.Area() - e.mbr.Area();
		if (increase < bestIncrease) {
			bestIncrease = increase;
			bestChild = e.child;
		}
	}
	return ChooseLeaf(bestChild, rect);
}

template <typename Traits>
void CRTree<Traits>::SplitNode(Node* node) {
	Node* sibling = new Node(node->IsLeaf());

	// Linear split (simplificado)
	sibling->Entries().assign(
		node->Entries().begin() + Traits::m,
		node->Entries().end()
	);
	node->Entries().resize(Traits::m);

	if (!node->GetParent()) {
		Node* newRoot = new Node(false);
		newRoot->Entries().push_back({ node->ComputeMBR(), node });
		newRoot->Entries().push_back({ sibling->ComputeMBR(), sibling });
		node->SetParent(newRoot);
		sibling->SetParent(newRoot);
		m_pRoot = newRoot;
	}
}

template <typename Traits>
void CRTree<Traits>::AdjustTree(Node* node) {
	while (node->GetParent()) {
		Node* parent = node->GetParent();
		for (auto& e : parent->Entries())
			if (e.child == node)
				e.mbr = node->ComputeMBR();

		if (parent->Size() > Traits::M)
			SplitNode(parent);

		node = parent;
	}
}

template <typename Traits>
void CRTree<Traits>::RangeQuery(const Rect& query, std::vector<Ref>& result) const {
	std::function<void(Node*)> visit = [&](Node* n) {
		for (auto& e : n->Entries()) {
			if (e.mbr.Intersects(query)) {
				if (n->IsLeaf())
					result.push_back(e.ref);
				else
					visit(e.child);
			}
		}
	};
	visit(m_pRoot);
}

template <typename Traits>
std::ostream& CRTree<Traits>::Write(std::ostream& os) {
	std::function<void(Node*)> dump = [&](Node* n) {
		os << n->IsLeaf() << " " << n->Size() << "\n";
		for (auto& e : n->Entries()) {
			os << e.mbr.m_minX << " " << e.mbr.m_minY << " "
			   << e.mbr.m_maxX << " " << e.mbr.m_maxY << " "
			   << e.ref << "\n";
			if (e.child) dump(e.child);
		}
	};
	dump(m_pRoot);
	return os;
}

template <typename Traits>
void CRTree<Traits>::Destroy(Node* node) {
    if (!node)
        return;

    if (!node->IsLeaf()) {
        for (auto& e : node->Entries())
            Destroy(e.child);
    }

    delete node;
}


