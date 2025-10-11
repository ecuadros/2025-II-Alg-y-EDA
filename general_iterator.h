#ifndef __GENERAL_ITERATOR_H__
#define __GENERAL_ITERATOR_H__

template<typename Container, typename Iterator>
class general_iterator {
public:
    using Node = typename Container::Node;

protected:
    Container* m_pContainer;
    Node*      m_pNode;
public:
    general_iterator(Container* pContainer, Node* pNode)
        : m_pContainer(pContainer), m_pNode(pNode) {}

    general_iterator(const general_iterator& other) = default;
};

#endif // __GENERAL_ITERATOR_H__