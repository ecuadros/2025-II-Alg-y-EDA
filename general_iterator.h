#ifndef __GENERAL_ITERATOR_H__
#define __GENERAL_ITERATOR_H__

template <typename Container, typename Derived>
class general_iterator{
public:
    using value_type = typename Container::value_type;
    using Node       = typename Container::Node;

protected:
    Container   *m_pContainer = nullptr;
    Node        *m_pNode      = nullptr;

public:
    general_iterator(Container* pContainer, Node* pNode)
        : m_pContainer(pContainer), m_pNode(pNode) {}

    general_iterator(general_iterator const &other)
        : m_pContainer(other.m_pContainer), m_pNode(other.m_pNode) {}

    // general_iterator& operator=(general_iterator& other) {
    //     if (this != &other) {
    //         m_pContainer = other.m_pContainer;
    //         m_pNode = other.m_pNode;
    //     }
    //     return *this;
    // }

    bool operator==(general_iterator other){ return m_pContainer == other.m_pContainer && m_pNode == other.m_pNode; }
    bool operator!=(general_iterator other){ return !(*this == other); }

    value_type& operator*(){     return (m_pNode->getDataRef());    }

};

#endif // __GENERAL_ITERATOR_H__