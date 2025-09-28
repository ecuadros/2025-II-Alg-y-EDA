#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <mutex>
#include <stdexcept>
#include <iostream>

// PC1: deben hacer:
//      2 problemas de nivel 2
//      3 problemas de nivel 1
// Cada solucion enviarla como un Pull request

template <typename T>
class CVector{

    T      *m_pVect = nullptr;
    size_t  m_count = 0; // How many elements we have now?
    size_t  m_max   = 0; // Max capacity

    mutable std::mutex m_mutex;

public:
    class iterator {
    private:
        T* ptr;
        const CVector<T>* container;
        mutable std::mutex* mutex_ref;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        iterator(T* p, const CVector<T>* c, std::mutex* m) : ptr(p), container(c), mutex_ref(m) {}

        reference operator*() { return *ptr; }
        pointer operator->() { return ptr; }

        iterator& operator++() {
            ++ptr;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++ptr;
            return tmp;
        }

        bool operator==(const iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const iterator& other) const { return ptr != other.ptr; }
    };

    class const_iterator {
    private:
        const T* ptr;
        const CVector<T>* container;
        mutable std::mutex* mutex_ref;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        const_iterator(const T* p, const CVector<T>* c, std::mutex* m) : ptr(p), container(c), mutex_ref(m) {}

        reference operator*() const { return *ptr; }
        pointer operator->() const { return ptr; }

        const_iterator& operator++() {
            ++ptr;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++ptr;
            return tmp;
        }

        bool operator==(const const_iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const const_iterator& other) const { return ptr != other.ptr; }
    };

    class reverse_iterator {
    private:
        T* ptr;
        const CVector<T>* container;
        mutable std::mutex* mutex_ref;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        reverse_iterator(T* p, const CVector<T>* c, std::mutex* m) : ptr(p), container(c), mutex_ref(m) {}

        reference operator*() { return *ptr; }
        pointer operator->() { return ptr; }

        reverse_iterator& operator++() {
            --ptr;
            return *this;
        }

        reverse_iterator operator++(int) {
            reverse_iterator tmp = *this;
            --ptr;
            return tmp;
        }

        bool operator==(const reverse_iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const reverse_iterator& other) const { return ptr != other.ptr; }
    };

    class const_reverse_iterator {
    private:
        const T* ptr;
        const CVector<T>* container;
        mutable std::mutex* mutex_ref;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        const_reverse_iterator(const T* p, const CVector<T>* c, std::mutex* m) : ptr(p), container(c), mutex_ref(m) {}

        reference operator*() const { return *ptr; }
        pointer operator->() const { return ptr; }

        const_reverse_iterator& operator++() {
            --ptr;
            return *this;
        }

        const_reverse_iterator operator++(int) {
            const_reverse_iterator tmp = *this;
            --ptr;
            return tmp;
        }

        bool operator==(const const_reverse_iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const const_reverse_iterator& other) const { return ptr != other.ptr; }
    };
    CVector(size_t n);

    // TODO  (Nivel 1) Agregar un constructor por copia
    CVector(const CVector &v);

    // TODO  (Nivel 2): Agregar un move constructor
    CVector(CVector &&v) noexcept;

    CVector& operator=(const CVector& v);
    CVector& operator=(CVector&& v) noexcept;

    // TODO: (Nivel 1) implementar el destructor de forma segura
    ~CVector();

    void insert(const T &elem);
    void resize();
    T& operator[](size_t index);
    const T& operator[](size_t index) const;
    size_t size() const;

    T at(size_t index) const;

    std::vector<T> to_vector() const;

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

    template<typename U>
    friend std::ostream& operator<<(std::ostream& os, const CVector<U>& vec);
};

template <typename T>
CVector<T>::CVector(size_t n) : m_max(n) {
    if (n > 0) {
        m_pVect = new T[n];
    }
}

template <typename T>
CVector<T>::CVector(const CVector &v) {
    std::lock_guard<std::mutex> lock(v.m_mutex);

    m_max = v.m_max;
    m_count = v.m_count;

    if (m_max > 0) {
        m_pVect = new T[m_max];
        for (size_t i = 0; i < m_count; ++i) {
            m_pVect[i] = v.m_pVect[i];
        }
    } else {
        m_pVect = nullptr;
    }
}

template <typename T>
CVector<T>::CVector(CVector &&v) noexcept {
    std::lock_guard<std::mutex> lock(v.m_mutex);

    m_pVect = v.m_pVect;
    m_max = v.m_max;
    m_count = v.m_count;

    v.m_pVect = nullptr;
    v.m_max = 0;
    v.m_count = 0;
}

template <typename T>
CVector<T>& CVector<T>::operator=(const CVector& v) {
    if (this != &v) {
        std::mutex* first = &m_mutex;
        std::mutex* second = &v.m_mutex;

        if (this > &v) {
            std::swap(first, second);
        }

        std::lock_guard<std::mutex> lock1(*first);
        std::lock_guard<std::mutex> lock2(*second);

        delete[] m_pVect;

        m_max = v.m_max;
        m_count = v.m_count;

        if (m_max > 0) {
            m_pVect = new T[m_max];
            for (size_t i = 0; i < m_count; ++i) {
                m_pVect[i] = v.m_pVect[i];
            }
        } else {
            m_pVect = nullptr;
        }
    }
    return *this;
}

template <typename T>
CVector<T>& CVector<T>::operator=(CVector&& v) noexcept {
    if (this != &v) {
        std::mutex* first = &m_mutex;
        std::mutex* second = &v.m_mutex;

        if (this > &v) {
            std::swap(first, second);
        }

        std::lock_guard<std::mutex> lock1(*first);
        std::lock_guard<std::mutex> lock2(*second);

        delete[] m_pVect;

        m_pVect = v.m_pVect;
        m_max = v.m_max;
        m_count = v.m_count;

        v.m_pVect = nullptr;
        v.m_max = 0;
        v.m_count = 0;
    }
    return *this;
}

template <typename T>
CVector<T>::~CVector(){
    delete[] m_pVect;
    m_pVect = nullptr;
    m_count = 0;
    m_max = 0;
}

// TODO (Nivel 1): hacer dinamico el delta de crecimiento
template <typename T>
void CVector<T>::resize(){
    size_t new_capacity = (m_max == 0) ? 1 : m_max * 2;
    T *pTmp = new T[new_capacity];

    for(size_t i = 0; i < m_count; ++i) {
        pTmp[i] = m_pVect[i];
    }

    delete[] m_pVect;
    m_max = new_capacity;
    m_pVect = pTmp;
}

template <typename T>
void CVector<T>::insert(const T &elem){
    std::lock_guard<std::mutex> lock(m_mutex);

    if(m_count == m_max) {
        resize();
    }

    m_pVect[m_count++] = elem;
}

template <typename T>
T& CVector<T>::operator[](size_t index) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (index >= m_count) {
        throw std::out_of_range("Index out of range");
    }

    return m_pVect[index];
}

template <typename T>
const T& CVector<T>::operator[](size_t index) const {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (index >= m_count) {
        throw std::out_of_range("Index out of range");
    }

    return m_pVect[index];
}

template <typename T>
size_t CVector<T>::size() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_count;
}

template <typename T>
T CVector<T>::at(size_t index) const {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (index >= m_count) {
        throw std::out_of_range("Index out of range");
    }

    return m_pVect[index];
}

template <typename T>
std::vector<T> CVector<T>::to_vector() const {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<T> result;
    result.reserve(m_count);

    for (size_t i = 0; i < m_count; ++i) {
        result.push_back(m_pVect[i]);
    }

    return result;
}

template <typename T>
typename CVector<T>::iterator CVector<T>::begin() {
    return iterator(m_pVect, this, &m_mutex);
}

template <typename T>
typename CVector<T>::iterator CVector<T>::end() {
    return iterator(m_pVect + m_count, this, &m_mutex);
}

template <typename T>
typename CVector<T>::const_iterator CVector<T>::begin() const {
    return const_iterator(m_pVect, this, &m_mutex);
}

template <typename T>
typename CVector<T>::const_iterator CVector<T>::end() const {
    return const_iterator(m_pVect + m_count, this, &m_mutex);
}

template <typename T>
typename CVector<T>::const_iterator CVector<T>::cbegin() const {
    return const_iterator(m_pVect, this, &m_mutex);
}

template <typename T>
typename CVector<T>::const_iterator CVector<T>::cend() const {
    return const_iterator(m_pVect + m_count, this, &m_mutex);
}

template <typename T>
typename CVector<T>::reverse_iterator CVector<T>::rbegin() {
    return reverse_iterator(m_pVect + m_count - 1, this, &m_mutex);
}

template <typename T>
typename CVector<T>::reverse_iterator CVector<T>::rend() {
    return reverse_iterator(m_pVect - 1, this, &m_mutex);
}

template <typename T>
typename CVector<T>::const_reverse_iterator CVector<T>::rbegin() const {
    return const_reverse_iterator(m_pVect + m_count - 1, this, &m_mutex);
}

template <typename T>
typename CVector<T>::const_reverse_iterator CVector<T>::rend() const {
    return const_reverse_iterator(m_pVect - 1, this, &m_mutex);
}

template <typename T>
typename CVector<T>::const_reverse_iterator CVector<T>::crbegin() const {
    return const_reverse_iterator(m_pVect + m_count - 1, this, &m_mutex);
}

template <typename T>
typename CVector<T>::const_reverse_iterator CVector<T>::crend() const {
    return const_reverse_iterator(m_pVect - 1, this, &m_mutex);
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const CVector<T>& vec) {
    std::lock_guard<std::mutex> lock(vec.m_mutex);

    os << "[";
    for (size_t i = 0; i < vec.m_count; ++i) {
        os << vec.m_pVect[i];
        if (i < vec.m_count - 1) {
            os << ", ";
        }
    }
    os << "]";
    return os;
}

#endif // __VECTOR_H__