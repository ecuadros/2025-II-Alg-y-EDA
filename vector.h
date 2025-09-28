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