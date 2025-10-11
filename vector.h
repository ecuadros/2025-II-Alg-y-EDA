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
    // ======== TODO  (Nivel 1) Agregar un constructor por copia ========
    CVector(const CVector &v) {
        m_count = v.m_count;
        m_max   = v.m_max;
        m_pVect = new T[m_max];
        for (size_t i = 0; i < m_count; ++i)
            m_pVect[i] = v.m_pVect[i];
    }

    // Constructor normal con tamaño inicial
    CVector(size_t n = 0) {
        m_count = n;
        m_max   = n > 0 ? n : 10;
        m_pVect = new T[m_max];
        // inicializar
        for (size_t i = 0; i < m_count; ++i)
            m_pVect[i] = T{};
    }

    // ======== TODO  (Nivel 2): Agregar un move constructor ========
    CVector(CVector &&v) noexcept {
        m_pVect = v.m_pVect;
        m_count = v.m_count;
        m_max   = v.m_max;
        // dejar v en estado seguro
        v.m_pVect = nullptr;
        v.m_count = 0;
        v.m_max   = 0;
    }

    // ======== TODO: (Nivel 1) implementar el destructor de forma segura ========
    virtual ~CVector() {
        delete[] m_pVect;
        m_pVect = nullptr;
        m_count = 0;
        m_max   = 0;
    }

    void insert(const T &elem);
    void resize();

    // ======== Nivel 1: operador [] ========
    T& operator[](std::size_t index) {
        return m_pVect[index];
    }
    const T& operator[](std::size_t index) const {
        return m_pVect[index];
    }

    // ======== Nivel 2: operador << ========
    template <typename U>
    friend std::ostream& operator<<(std::ostream& os, const CVector<U>& v);
};

// Implementación resize
// TODO (Nivel 1): hacer dinámico el delta de crecimiento
template <typename T>
void CVector<T>::resize(){
    size_t delta = 10; // podrías parametrizarlo si quieres
    T *pTmp = new T[m_max + delta];
    for (size_t i = 0; i < m_max ; ++i)
        pTmp[i] = m_pVect[i];
    delete [] m_pVect;
    m_max += delta;
    m_pVect = pTmp;
}

// TODO (ya está hecha): la funcion insert debe permitir que el vector crezca si ha desbordado
template <typename T>
void CVector<T>::insert(const T &elem){
    if (m_count == m_max)
        resize();
    m_pVect[m_count++] = elem;
}

// Operador << para imprimir el vector
template <typename T>
std::ostream& operator<<(std::ostream& os, const CVector<T>& v) {
    os << "[ ";
    for (size_t i = 0; i < v.m_count; ++i)
        os << v.m_pVect[i] << " ";
    os << "]";
    return os;
}

#endif // __VECTOR_H__
