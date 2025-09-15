#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <type_traits>
#include <shared_mutex>
#include <cstring>
#include <mutex>
#include <iostream>
#include <iterator>

// PC1: deben hacer:
//      2 problemas de nivel 2
//      3 problemas de nivel 1
// Cada solucion enviarla como un Pull request

// TODO (Nivel 2): Agregar Traits

// TODO (Nivel 2): Agregar Iterators (forward, backward)

// TODO (Nivel 1): Agregar Documentacion para generar con doxygen

// TODO  (Nivel 2): Agregar control de concurrencia en todo el vector
template <typename T>
class CVector
{

    T *m_pVect = nullptr;
    size_t m_count = 0; // How many elements we have now?
    size_t m_max = 0;   // Max capacity
    double m_growth_factor = 1.5;
    mutable std::shared_mutex m_mutex;

public:
    using size_type = size_t;
    using traits_type = std::conditional_t<std::is_trivially_copyable_v<T>, std::true_type, std::false_type>;

    class iterator
    {
    private:
        T* ptr;

    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;

        iterator(T* p = nullptr) : ptr(p) {}

        reference operator*() const { return *ptr; }
        pointer operator->() const { return ptr; }

        iterator& operator++() { ++ptr; return *this; }
        iterator operator++(int) { iterator temp = *this; ++ptr; return temp; }

        iterator& operator--() { --ptr; return *this; }
        iterator operator--(int) { iterator temp = *this; --ptr; return temp; }

        iterator operator+(difference_type n) const { return iterator(ptr + n); }
        iterator operator-(difference_type n) const { return iterator(ptr - n); }
        iterator& operator+=(difference_type n) { ptr += n; return *this; }
        iterator& operator-=(difference_type n) { ptr -= n; return *this; }

        difference_type operator-(const iterator& other) const { return ptr - other.ptr; }

        reference operator[](difference_type n) const { return ptr[n]; }

        bool operator==(const iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const iterator& other) const { return ptr != other.ptr; }
        bool operator<(const iterator& other) const { return ptr < other.ptr; }
        bool operator<=(const iterator& other) const { return ptr <= other.ptr; }
        bool operator>(const iterator& other) const { return ptr > other.ptr; }
        bool operator>=(const iterator& other) const { return ptr >= other.ptr; }
    };

    class const_iterator
    {
    private:
        const T* ptr;

    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;
        using iterator_category = std::random_access_iterator_tag;

        const_iterator(const T* p = nullptr) : ptr(p) {}
        const_iterator(const iterator& it) : ptr(&(*it)) {}

        reference operator*() const { return *ptr; }
        pointer operator->() const { return ptr; }

        const_iterator& operator++() { ++ptr; return *this; }
        const_iterator operator++(int) { const_iterator temp = *this; ++ptr; return temp; }

        const_iterator& operator--() { --ptr; return *this; }
        const_iterator operator--(int) { const_iterator temp = *this; --ptr; return temp; }

        const_iterator operator+(difference_type n) const { return const_iterator(ptr + n); }
        const_iterator operator-(difference_type n) const { return const_iterator(ptr - n); }
        const_iterator& operator+=(difference_type n) { ptr += n; return *this; }
        const_iterator& operator-=(difference_type n) { ptr -= n; return *this; }

        difference_type operator-(const const_iterator& other) const { return ptr - other.ptr; }

        reference operator[](difference_type n) const { return ptr[n]; }

        bool operator==(const const_iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const const_iterator& other) const { return ptr != other.ptr; }
        bool operator<(const const_iterator& other) const { return ptr < other.ptr; }
        bool operator<=(const const_iterator& other) const { return ptr <= other.ptr; }
        bool operator>(const const_iterator& other) const { return ptr > other.ptr; }
        bool operator>=(const const_iterator& other) const { return ptr >= other.ptr; }
    };

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin();
    iterator end();
    
    const_iterator begin() const;
    const_iterator end() const;
    
    const_iterator cbegin() const;
    const_iterator cend() const;
    
    reverse_iterator rbegin();
    reverse_iterator rend();
    
    const_reverse_iterator rbegin() const;
    const_reverse_iterator rend() const;
    
    const_reverse_iterator crbegin() const;
    const_reverse_iterator crend() const;

    // TODO  (Nivel 1) Agregar un constructor por copia
    CVector(const CVector &v);

    CVector(size_t n);
    // TODO  (Nivel 2): Agregar un move constructor
    CVector(CVector &&v) noexcept;

    // TODO: (Nivel 1) implementar el destructor de forma segura
    virtual ~CVector();
    void insert(T &elem);
    void resize();
    T &operator[](size_t index);
    const T &operator[](size_t index) const;
    void set_growth_factor(double factor);
    size_type size() const noexcept;
    size_type capacity() const noexcept;
    bool empty() const noexcept;
};

template <typename T>
typename CVector<T>::iterator CVector<T>::begin()
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return iterator(m_pVect);
}

template <typename T>
typename CVector<T>::iterator CVector<T>::end()
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return iterator(m_pVect + m_count);
}

template <typename T>
typename CVector<T>::const_iterator CVector<T>::begin() const
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return const_iterator(m_pVect);
}

template <typename T>
typename CVector<T>::const_iterator CVector<T>::end() const
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return const_iterator(m_pVect + m_count);
}

template <typename T>
typename CVector<T>::const_iterator CVector<T>::cbegin() const
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return const_iterator(m_pVect);
}

template <typename T>
typename CVector<T>::const_iterator CVector<T>::cend() const
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return const_iterator(m_pVect + m_count);
}

template <typename T>
typename CVector<T>::reverse_iterator CVector<T>::rbegin()
{
    return reverse_iterator(end());
}

template <typename T>
typename CVector<T>::reverse_iterator CVector<T>::rend()
{
    return reverse_iterator(begin());
}

template <typename T>
typename CVector<T>::const_reverse_iterator CVector<T>::rbegin() const
{
    return const_reverse_iterator(end());
}

template <typename T>
typename CVector<T>::const_reverse_iterator CVector<T>::rend() const
{
    return const_reverse_iterator(begin());
}

template <typename T>
typename CVector<T>::const_reverse_iterator CVector<T>::crbegin() const
{
    return const_reverse_iterator(end());
}

template <typename T>
typename CVector<T>::const_reverse_iterator CVector<T>::crend() const
{
    return const_reverse_iterator(begin());
}

template <typename T>
typename CVector<T>::size_type CVector<T>::size() const noexcept
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_count;
}

template <typename T>
typename CVector<T>::size_type CVector<T>::capacity() const noexcept
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_max;
}

template <typename T>
bool CVector<T>::empty() const noexcept
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_count == 0;
}

// Implementacion del operador de salida <<
template <typename T>
std::ostream &operator<<(std::ostream &os, const CVector<T> &vec)
{
    os << "[";

    for (size_t i = 0; i < vec.size(); ++i)
    {
        os << vec[i];

        if (i < vec.size() - 1)
        {
            os << ", ";
        }
    }

    os << "]";
    return os;
}

template <typename T>
void CVector<T>::set_growth_factor(double factor)
{
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    if (factor > 1.0)
    {
        m_growth_factor = factor;
    }
}

// Implementacion del operador []
template <typename T>
T &CVector<T>::operator[](size_t index)
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_pVect[index];
}

template <typename T>
const T &CVector<T>::operator[](size_t index) const
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_pVect[index];
}

// Implementacion de constructor move
template <typename T>
CVector<T>::CVector(CVector &&v) noexcept
    : m_pVect(v.m_pVect), m_count(v.m_count), m_max(v.m_max), m_growth_factor(v.m_growth_factor)
{
    v.m_pVect = nullptr;
    v.m_count = 0;
    v.m_max = 0;
    v.m_growth_factor = 1.5;
}

// Implementacion de constructor por copia
template <typename T>
CVector<T>::CVector(const CVector &v) : m_pVect(nullptr), m_count(0), m_max(0)
{
    std::shared_lock<std::shared_mutex> lock(v.m_mutex);

    if (v.m_count == 0)
    {
        return;
    }

    m_max = v.m_count;
    m_pVect = new T[m_max];
    m_count = v.m_count;

    if constexpr (traits_type::is_trivially_copyable)
    {
        std::memcpy(m_pVect, v.m_pVect, m_count * sizeof(T));
    }
    else
    {
        for (size_t i = 0; i < m_count; ++i)
        {
            m_pVect[i] = v.m_pVect[i];
        }
    }
}

template <typename T>
CVector<T>::CVector(size_t n) : m_pVect(nullptr), m_count(0), m_max(0)
{
    if (n > 0)
    {
        m_pVect = new T[n];
        m_max = n;
    }
}

// Implementacion del destructor de forma segura
template <typename T>
CVector<T>::~CVector()
{
    if (m_pVect != nullptr)
    {
        delete[] m_pVect;
        m_pVect = nullptr;
    }
}

// TODO (Nivel 1): hacer dinamico el delta de crecimiento
template <typename T>
void CVector<T>::resize()
{
    size_t new_size;
    if (m_max == 0)
    {
        new_size = 1;
    }
    else
    {
        new_size = static_cast<size_t>(m_max * m_growth_factor);
        if (new_size <= m_max)
        {
            new_size = m_max + 1;
        }
    }

    T *pTmp = new T[new_size];

    if constexpr (traits_type::is_trivially_copyable)
    {
        if (m_pVect != nullptr && m_count > 0)
        {
            std::memcpy(pTmp, m_pVect, m_count * sizeof(T));
        }
    }
    else
    {
        for (size_t i = 0; i < m_count; ++i)
        {
            pTmp[i] = std::move(m_pVect[i]);
        }
    }

    delete[] m_pVect;
    m_max = new_size;
    m_pVect = pTmp;
}

// TODO (ya está hecha): la funcion insert debe permitir que el vector crezca si ha desbordado
template <typename T>
void CVector<T>::insert(T &elem)
{
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    if (m_count == m_max)
        resize();
    m_pVect[m_count++] = elem;
}

#endif // __VECTOR_H__