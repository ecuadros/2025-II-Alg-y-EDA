#ifndef __GENERAL_ITERATOR_H__
#define __GENERAL_ITERATOR_H__
#include <iterator>
#include <utility>

template <typename Container, typename Derived>
class general_iterator {
public:
    using value_type = typename Container::value_type;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    using Node = typename Container::Node;

protected:
    Container* m_pContainer;
    Node* m_pNode;

public:
    // Constructores
    general_iterator(Container* pContainer, Node* pNode)
        : m_pContainer(pContainer), m_pNode(pNode) {}
    
    general_iterator(const general_iterator& other)
        : m_pContainer(other.m_pContainer), m_pNode(other.m_pNode) {}
    
    general_iterator(general_iterator&& other)
        : m_pContainer(std::exchange(other.m_pContainer, nullptr)),
          m_pNode(std::exchange(other.m_pNode, nullptr)) {}

    // Operadores de asignación
    general_iterator& operator=(const general_iterator& other) {
        if (this != &other) {
            m_pContainer = other.m_pContainer;
            m_pNode = other.m_pNode;
        }
        return *this;
    }

    general_iterator& operator=(general_iterator&& other) {
        if (this != &other) {
            m_pContainer = std::exchange(other.m_pContainer, nullptr);
            m_pNode = std::exchange(other.m_pNode, nullptr);
        }
        return *this;
    }

    // Operadores de comparación
    bool operator==(const general_iterator& other) const {
        return m_pNode == other.m_pNode;
    }

    bool operator!=(const general_iterator& other) const {
        return m_pNode != other.m_pNode;
    }

    // Operadores de acceso
    reference operator*() const {
        return m_pNode->getDataRef();
    }

    pointer operator->() const {
        return &(m_pNode->getDataRef());
    }

    // Operadores de incremento (deben ser implementados por la clase derivada)
    Derived& operator++() {
        // Este método debe ser sobrescrito por la clase derivada
        static_assert(sizeof(Derived) == 0,
            "general_iterator::operator++() must be implemented by derived class");
        return *static_cast<Derived*>(this);
    }

    Derived operator++(int) {
        // Implementación por defecto que usa el pre-incremento de la clase derivada
        Derived tmp = *static_cast<Derived*>(this);
        ++(*static_cast<Derived*>(this));
        return tmp;
    }

    // Métodos de acceso para clases derivadas
    Container* getContainer() const { return m_pContainer; }
    Node* getNode() const { return m_pNode; }
    void setNode(Node* node) { m_pNode = node; }

    // Destructor virtual para permitir herencia segura
    virtual ~general_iterator() = default;
};

#endif // __GENERAL_ITERATOR_H__