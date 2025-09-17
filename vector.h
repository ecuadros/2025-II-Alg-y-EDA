/**
 * @file vector.h
 * @author Jhan Carlos Ccanto Quispe (jhan.ccanto.q@uni.pe)
 * @brief Implementation of a dynamic thread-safe vector with advanced features
 * @version 0.1
 * @date 2025-09-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */

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

/**
 * @brief traits for vector data types
 * @tparam T data type that will store the vector
 * @note traits are used to optimize operations based ont he characteristic of the type
 */

// TODO (Nivel 2): Agregar Traits
template <typename T>
struct vector_traits
{
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    static constexpr bool is_trivially_copyable = std::is_trivially_copyable_v<T>;
    static constexpr bool is_default_constructible = std::is_default_constructible_v<T>;
    static constexpr bool is_move_constructible = std::is_move_constructible_v<T>;
    static constexpr bool is_copy_constructible = std::is_copy_constructible_v<T>;
    static constexpr bool is_arithmetic = std::is_arithmetic_v<T>;
    static constexpr bool is_pointer = std::is_pointer_v<T>;
};

// TODO (Nivel 2): Agregar Iterators (forward, backward)

// TODO (Nivel 1): Agregar Documentacion para generar con doxygen

/**
 * @brief Thread-safe dynamic vector with advanced features
 * @tparam T type of elements stored in the vector
 */

// TODO  (Nivel 2): Agregar control de concurrencia en todo el vector
template <typename T>
class CVector
{

    T *m_pVect = nullptr;
    size_t m_count = 0;
    size_t m_max = 0;
    double m_growth_factor = 1.5;
    mutable std::shared_mutex m_mutex;

public:
    using traits_type = vector_traits<T>;
    using value_type = typename traits_type::value_type;
    using reference = typename traits_type::reference;
    using const_reference = typename traits_type::const_reference;
    using pointer = typename traits_type::pointer;
    using const_pointer = typename traits_type::const_pointer;
    using size_type = typename traits_type::size_type;
    using difference_type = typename traits_type::difference_type;

    /**
     * @brief Random access iterator for CVector
     * Implements all operator required for random access iterator
     * Compataible with STL algorithms
     */
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

        /**
         * @brief Iterator constructor
         * @param p Pinter to the initial element
         */
        iterator(T* p = nullptr) : ptr(p) {}

        /**
         * @brief Dereference operator
         * @return Reference to the pointed element
         */
        reference operator*() const { return *ptr; }

        /**
         * @brief Member access operator
         * @return Pointer to the pointed element
         */
        pointer operator->() const { return ptr; }

        /**
         * @brief Pre-increment operator
         * @return reference to the incremented iterator
         */
        iterator& operator++() { ++ptr; return *this; }

        /**
         * @brief Post-increment operator
         * @return Copy of the iterator before incrementing
         */
        iterator operator++(int) { iterator temp = *this; ++ptr; return temp; }

        /**
         * @brief Pre-decrement operator
         * @return Reference to the decremented iterator
         */
        iterator& operator--() { --ptr; return *this; }

        /**
         * @brief Post-decrement operator
         * @return Copy of the iterator before decrementing
         */
        iterator operator--(int) { iterator temp = *this; --ptr; return temp; }

        /**
         * @brief Adds a displacement to the iterator
         * @param n Number of positons to advance
         * @return New displaced iterator
         */
        iterator operator+(difference_type n) const { return iterator(ptr + n); }

        /**
         * @brief Subtract a displacement from the iterator
         * @param n Number of positions to move back
         * @return New displaced iterator
         */
        iterator operator-(difference_type n) const { return iterator(ptr - n); }

        /**
         * @brief Addtion and assigment operator
         * @param n Number of positions to advance
         * @return Reference to the modified iterator
         */
        iterator& operator+=(difference_type n) { ptr += n; return *this; }

        /**
         * @brief Substraction and assignment operator
         * @param n Number of positons to move back
         * @return Reference to the modified iterator.
         */
        iterator& operator-=(difference_type n) { ptr -= n; return *this; }

        /**
         * @brief Calculate the distance between iterators
         * @param other Another iterator
         * @return Distance between iterators
         */
        difference_type operator-(const iterator& other) const { return ptr - other.ptr; }

        /**
         * @brief Indexed access operator
         * @param n Relative index
         * @return Reference to the element al position n
         */
        reference operator[](difference_type n) const { return ptr[n]; }

        bool operator==(const iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const iterator& other) const { return ptr != other.ptr; }
        bool operator<(const iterator& other) const { return ptr < other.ptr; }
        bool operator<=(const iterator& other) const { return ptr <= other.ptr; }
        bool operator>(const iterator& other) const { return ptr > other.ptr; }
        bool operator>=(const iterator& other) const { return ptr >= other.ptr; }
    };

    /**
     * @brief Constant random access iterator for CVector
     * Similar to the normal iterator but for read-only access
     */
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

        /**
         * @brief Constant iterator constructor
         * @param p Pointer to the initial element
         */
        const_iterator(const T* p = nullptr) : ptr(p) {}

        /**
         * @brief Conversion constructor from normal iterator
         * @param it Normal iterator
         */
        const_iterator(const iterator& it) : ptr(&(*it)) {}

        /**
         * @brief Dereference operator
         * @return Constant reference to the pointed element
         */
        reference operator*() const { return *ptr; }

        /**
         * @brief Memeber access operator
         * @return Constant pointer to the pointed element
         */
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

    /**
     * @brief Get an iterator at the beginning of the vector
     * @return Iterator to the first element
     */
    iterator begin();

    /**
     * @brief Gets an iterator to the end of the vector 
     * @return Iterator past the last element
     */
    iterator end();
    
    /**
     * @brief Gets a constant iterator to the start of the vector
     * @return Constant iterator to the first element
     */
    const_iterator begin() const;

    /**
     * @brief Gets a constant iterator to the end of the vector
     * @return Constant iterator past thye last element
     */
    const_iterator end() const;
    
    /**
     * @brief Gets a constant iterator to the start of the vector
     * @return Constant iterator tot he first element
     */
    const_iterator cbegin() const;

    /**
     * @brief Gets a constant iterator to the end of the vector
     * @return Constant iterator past the last element
     */
    const_iterator cend() const;
    
    /**
     * @brief Gets a reverse iterator tothe start (last element)
     * @return Reverse iterator to the last element
     */
    reverse_iterator rbegin();

    /**
     * @brief Gets a reverse iterator to the end (before the first element)
     * @return Reverse iterator before the first element 
     */
    reverse_iterator rend();
    
    /**
     * @brief Gets a constant reverse iterator to the start
     * @return Constant reverse iterator to he last element
     */
    const_reverse_iterator rbegin() const;

    /**
     * @brief Gets a constant reverse iterator at the end
     * @return Cosntant reverse iterator before the first element
     */
    const_reverse_iterator rend() const;
    
    /**
     * @brief Gets a constant reverse iterator to the start
     * @return Constant reverse iterator to the last element
     */
    const_reverse_iterator crbegin() const;

    /**
     * @brief Gets a constant reverse iterator at the end
     * @return Constant reverse iterator before the first element
     */
    const_reverse_iterator crend() const;

    /**
     * @brief Default constructor
     * Initializes an empty vector with a growth factor of 1.5
     */
    // Constructor por defecto
    CVector() : m_pVect(nullptr), m_count(0), m_max(0), m_growth_factor(1.5) {}
    
    // TODO  (Nivel 1) Agregar un constructor por copia
    /**
     * @brief Copy constructor
     * @param v Vector to copy 
     */
    CVector(const CVector &v);

    /**
     * @brief Constructor with a specified size
     * @param n Initial capacity of the vector
     */
    CVector(size_t n);
    // TODO  (Nivel 2): Agregar un move constructor
    /**
     * @brief Move constructor
     * @param v Vector to move (rvalue reference)
     */
    CVector(CVector &&v) noexcept;

    /**
     * @brief Copy assignment operator
     * @param other Vector to copy
     * @return Reference to this vector after copying
     */
    CVector& operator=(const CVector& other);

    /**
     * @brief Move assignment operator
     * @param other vector to move
     * @return Reference to this vector
     */
    CVector& operator=(CVector&& other) noexcept;

    // TODO: (Nivel 1) implementar el destructor de forma segura
    /**
     * @brief Destroy the CVector object
     * Safely frees memory
     * Sets the pointer to nullptr after freeing
     */
    virtual ~CVector();

    /**
     * @brief Insert an element at the end of the vector
     * @param elem Element to insert by reference
     * @note This operation is thread-safe and acquires an exclusive lock
     */
    void insert(T &elem);

    /**
     * @brief Resizes the vector when it reuns out of capacity
     */
    void resize();

    /**
     * @brief Indexed access operator
     * @param index Infdex of the element to access
     * @return Reference to the element at position index
     * @note Thread-safe for concurrent reading
     */
    T &operator[](size_t index);

    /**
     * @brief Constant indexed access operator
     * @param index Index of the element to access
     * @return Constant reference to the element at postion index
     * @note Thread-safe for concurrent reading
     */
    const T &operator[](size_t index) const;

    /**
     * @brief Sets the vector's growth factor
     * @param factor New growth factor
     * @note if factor <= 1.0, the operation is ignored
     * @note Thread-safe
     */
    void set_growth_factor(double factor);
    
    /**
     * @brief Checks if the stored type is arithmetic
     * @return true if T is an arithmetic ype, false otherwise
     * @note Constexptr function evaluated at compile time
     */
    constexpr bool is_arithmetic_type() const noexcept {
        return traits_type::is_arithmetic;
    }
    
    /**
     * @brief Checks is the stored type is trivially copyable
     * @return true if T is trivially copyable, false otherwise
     * @note Constexptr function evaluated at complie time
     */
    constexpr bool is_trivially_copyable_type() const noexcept {
        return traits_type::is_trivially_copyable;
    }
    
    /**
     * @brief Checks if the stored type is a pointer
     * @return true if T is a pointer type, false otherwise
     * @note Constexptr funcstion evaluated at compile time
     */
    constexpr bool is_pointer_type() const noexcept {
        return traits_type::is_pointer;
    }
    
    /**
     * @brief Gets the number of elements in the vector
     * @return Current number of elements
     * @note Thread-safe
     */
    size_type size() const noexcept;

    /**
     * @brief Gets the current capacity of the vector
     * @return Maximun capacity without reallocation
     * @note Trhead-safe
     */
    size_type capacity() const noexcept;

    /**
     * @brief Checks if the vector is empty
     * @return true if the vector contains no elements false otherwise
     * @note Thread-safe
     */
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
/**
 * @brief Output operator to print the vector
 * @tparam T Type of vector elements
 * @param os  Oputput stream
 * @param vec Vector to print
 * @return Reference to the output stream
 */
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
CVector<T>& CVector<T>::operator=(const CVector& other)
{
    if (this != &other)
    {
        std::unique_lock<std::shared_mutex> lock1(m_mutex, std::defer_lock);
        std::shared_lock<std::shared_mutex> lock2(other.m_mutex, std::defer_lock);
        std::lock(lock1, lock2);
        
        delete[] m_pVect;
        m_pVect = nullptr;
        m_count = 0;
        m_max = 0;
        
        if (other.m_count > 0)
        {
            m_max = other.m_count;
            m_pVect = new T[m_max];
            m_count = other.m_count;
            
            if constexpr (traits_type::is_trivially_copyable)
            {
                std::memcpy(m_pVect, other.m_pVect, m_count * sizeof(T));
            }
            else
            {
                for (size_t i = 0; i < m_count; ++i)
                {
                    m_pVect[i] = other.m_pVect[i];
                }
            }
        }
        
        m_growth_factor = other.m_growth_factor;
    }
    return *this;
}

template <typename T>
CVector<T>& CVector<T>::operator=(CVector&& other) noexcept
{
    if (this != &other)
    {
        std::unique_lock<std::shared_mutex> lock1(m_mutex, std::defer_lock);
        std::unique_lock<std::shared_mutex> lock2(other.m_mutex, std::defer_lock);
        std::lock(lock1, lock2);
        
        delete[] m_pVect;
        
        m_pVect = other.m_pVect;
        m_count = other.m_count;
        m_max = other.m_max;
        m_growth_factor = other.m_growth_factor;
        
        other.m_pVect = nullptr;
        other.m_count = 0;
        other.m_max = 0;
        other.m_growth_factor = 1.5;
    }
    return *this;
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