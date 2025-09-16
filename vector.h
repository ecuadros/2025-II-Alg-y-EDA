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
    size_t  m_delta = 10; // Growth factor (dynamic)
public:
    // TODO  (Nivel 1) Agregar un constructor por copia
    CVector(const CVector &v);

    CVector(size_t n);
    // TODO  (Nivel 2): Agregar un move constructor
    CVector(CVector &&v);

    // TODO: (Nivel 1) implementar el destructor de forma segura
    // virtual CVector();
    virtual ~CVector();
    void insert(T &elem);
    void resize();
    void set_delta(size_t d){ m_delta = d; }
};

// destructor seguro
template <typename T>
CVector<T>::~CVector() {
    delete [] m_pVect;   // libera el buffer dinámico
    m_pVect = nullptr;   // deja el puntero en estado nulo
    m_count = 0;         
    m_max   = 0;         
}

template <typename T>
CVector<T>::CVector(size_t n)
: m_pVect(nullptr), m_count(0), m_max(n) {
    if (m_max > 0)
        m_pVect = new T[m_max];
}

// TODO (Nivel 1): hacer el constructor por copia
template <typename T>
CVector<T>::CVector(const CVector<T> &v)
: m_pVect(nullptr), m_count(v.m_count), m_max(v.m_max) {
    if (m_max > 0) {
        m_pVect = new T[m_max];
        for (size_t i = 0; i < m_count; ++i)
            m_pVect[i] = v.m_pVect[i];
    }
}

// TODO (Nivel 1): hacer dinamico el delta de crecimiento
template <typename T>
void CVector<T>::resize(){
    size_t new_cap;

    if (m_max == 0) {
        new_cap = (m_delta > 0) ? m_delta : 1;
    } else if (m_delta > 0) {
        new_cap = m_max + m_delta;   // crecimiento lineal (delta)
    } else {
        new_cap = m_max * 2;         // crecimiento multiplicativo
    }

    T *pTmp = new T[new_cap];

    // Copiar solo los elementos usados
    for (size_t i = 0; i < m_count; ++i)
        pTmp[i] = m_pVect[i];

    delete [] m_pVect;
    m_pVect = pTmp;
    m_max   = new_cap;
}

// TODO (ya está hecha): la funcion insert debe permitir que el vector crezca si ha desbordado
template <typename T>
void CVector<T>::insert(T &elem){
    if(m_count == m_max)
        resize();
    m_pVect[m_count++] = elem;
}

#endif // __VECTOR_H__