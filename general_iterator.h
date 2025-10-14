#ifndef __GENERAL_ITERATOR_H__
#define __GENERAL_ITERATOR_H__

#include <iterator>

template <typename Container, typename Derived>
class general_iterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type        = typename Container::value_type;
    using difference_type   = std::ptrdiff_t;
    using pointer           = value_type*;
    using reference         = value_type&;
    using Node              = typename Container::Node;

protected:
    Container  *m_pContainer;
    Node       *m_pNode;

public:
    general_iterator(Container *pContainer, Node *pNode)
        : m_pContainer(pContainer), m_pNode(pNode) {}

    general_iterator(const general_iterator &other) = default;

    general_iterator(general_iterator &&other) noexcept = default;

    ~general_iterator() = default;

    general_iterator& operator=(const general_iterator &other) = default;

    general_iterator& operator=(general_iterator &&other) noexcept = default;

    reference operator*() const {
        return m_pNode->getDataRef();
    }

    pointer operator->() const {
        return &(m_pNode->getDataRef());
    }

    bool operator==(const general_iterator &other) const {
        return m_pNode == other.m_pNode;
    }

    bool operator!=(const general_iterator &other) const {
        return m_pNode != other.m_pNode;
    }
};

#endif // __GENERAL_ITERATOR_H__
