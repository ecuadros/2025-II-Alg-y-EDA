#ifndef __GENERAL_ITERATOR_H__
#define __GENERAL_ITERATOR_H__

template <typename Container, typename Iterator>
class general_iterator {
protected:
    using Node = typename Container::Node;
    Container *m_pContainer = nullptr;
    Node      *m_pNode      = nullptr;
public:
    general_iterator(Container *pContainer = nullptr, Node *pNode = nullptr)
        : m_pContainer(pContainer), m_pNode(pNode) {}
    general_iterator(general_iterator &other);
    general_iterator& operator=(const general_iterator &other);

    bool operator==(const general_iterator& other) const {
        return m_pContainer == other.m_pContainer && m_pNode == other.m_pNode;
    }
    bool operator!=(const general_iterator& other) const { return !(*this == other); }
};
#endif // __GENERAL_ITERATOR_H__