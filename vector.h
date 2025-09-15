#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <iostream>

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
    double  m_delta = 2.0;

public:
    // TODO  (Nivel 1) (listo) Agregar un constructor por copia
    CVector(CVector &v);

    CVector(size_t n);
    // TODO  (Nivel 2) (listo): Agregar un move constructor
    CVector(CVector &&v);

    // TODO: (Nivel 1) (listo) implementar el destructor de forma segura
    virtual ~CVector();
    void insert(T &elem);
    void resize();
    void resize(double delta);

    // Operador [] para acceso por índice
    T& operator[](size_t index);
    const T& operator[](size_t index) const;

    friend std::ostream& operator<<(std::ostream &os, const CVector<T> &v){
        os << "[";
        for(size_t i=0;i<v.m_count;i++){
            os<< v.m_pVect[i];
            if(i+1 <v.m_count ) os << ", ";
        }
        os << "]";
        return os;
    }
};

template <typename T>
CVector<T>::CVector(size_t n){
    m_pVect = new T[n];
    m_max   = n;
}

template <typename T>
CVector<T>::CVector(CVector &v)
    :m_pVect(v.m_max ? new T[v.m_max] : nullptr),m_count(v.m_count),m_max(v.m_max),m_delta(v.m_delta)
{
    for (size_t i = 0; i < m_count; ++i) {
        m_pVect[i] = v.m_pVect[i];
    }
}

template <typename T>
CVector<T>::CVector(CVector &&v)
    :m_pVect(v.m_pVect),m_count(v.m_count),m_max(v.m_max),m_delta(v.m_delta)
    {
        v.m_pVect =nullptr;
        v.m_count =0;
        v.m_max   =0;
    }

template <typename T>
CVector<T>::~CVector(){
    delete [] m_pVect;
}

template <typename T>
T& CVector<T>::operator[](size_t index){
    return m_pVect[index];
}

template <typename T>
const T& CVector<T>::operator[](size_t index) const {
    return m_pVect[index];
}

// TODO (Nivel 1) (listo): hacer dinamico el delta de crecimiento
template <typename T>
void CVector<T>::resize(double delta){
    size_t new_cap = (m_max == 0) ? 1 : static_cast<size_t>(m_max * delta);
    if (new_cap <= m_max) new_cap = m_max + 1; 

    T *pTmp = new T[new_cap];
    for (size_t i = 0; i < m_count; ++i)
        pTmp[i] = m_pVect[i];

    delete [] m_pVect;
    m_pVect = pTmp;
    m_max   = new_cap;
}

template <typename T>
void CVector<T>::resize(){
    resize(m_delta);
}

// TODO (ya está hecha): la funcion insert debe permitir que el vector crezca si ha desbordado
template <typename T>
void CVector<T>::insert(T &elem){
    if(m_count == m_max)
        resize();
    m_pVect[m_count++] = elem;
}

#endif // __VECTOR_H__