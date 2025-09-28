#ifndef __VECTOR_H__
#define __VECTOR_H__

// PC1: deben hacer:
//      2 problemas de nivel 2
//      3 problemas de nivel 1
// Cada solucion enviarla como un Pull request

// TODO (Nivel 2): Agregar Traits

// TODO (Nivel 2): Agregar Iterators (forward, backward)

// TODO (Nivel 1): Agregar Documentacion para generar con doxygen

// TODO  (Nivel 2): Agregar control de concurrencia en todo el vector
template <typename T>
class CVector{
   
    T      *m_pVect = nullptr;
    size_t  m_count = 0; // How many elements we have now?
    size_t  m_max   = 0; // Max capacity
public:
    CVector(size_t n);

    // TODO  (Nivel 1) Agregar un constructor por copia
    CVector(CVector &v);

    // TODO  (Nivel 2): Agregar un move constructor
    CVector(CVector &&v) noexcept;

    // TODO: (Nivel 1) implementar el destructor de forma segura
    ~CVector();

    void insert(T &elem);
    void resize();
    T& operator[](size_t index);
    size_t size() const { return m_count; }

    friend std::ostream& operator<<(std::ostream& os, const CVector& vec) {
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
};

template <typename T>
CVector<T>::CVector(size_t n) : m_max(n) {
    if (n > 0) {
        m_pVect = new T[n];
    }
}

template <typename T>
CVector<T>::CVector(CVector &v) : m_max(v.m_max), m_count(v.m_count) {
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
CVector<T>::CVector(CVector &&v) noexcept
: m_pVect(v.m_pVect), m_max(v.m_max), m_count(v.m_count) {
    v.m_pVect = nullptr;
    v.m_max = 0;
    v.m_count = 0;
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
    for(auto i=0; i < m_max ; ++i)
        pTmp[i] = m_pVect[i];
    delete [] m_pVect;
    m_max = new_capacity;
    m_pVect = pTmp;
}

template <typename T>
void CVector<T>::insert(T &elem){
    if(m_count == m_max)
        resize();
    m_pVect[m_count++] = elem;
}

template <typename T>
T& CVector<T>::operator[](size_t index) {
    if (index >= m_count) {
        throw std::out_of_range("Index out of range");
    }
    return m_pVect[index];
}

#endif // __VECTOR_H__