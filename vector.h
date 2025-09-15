#ifndef __VECTOR_H__
#define __VECTOR_H__
#include <iostream>
using namespace std;

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
    size_t  m_delta = 10;
public:
    // TODO  (Nivel 1) Agregar un constructor por copia - done
    CVector(CVector &v);

    CVector(size_t n);
    // TODO  (Nivel 2): Agregar un move constructor - done
    CVector(CVector &&v);

    // TODO: (Nivel 1) implementar el destructor de forma segura - done
    ~CVector();
    void insert(const T &elem);
    void resize();
    void resize(size_t new_size);

    T& operator[](size_t index){
        if(index >= m_max)
            throw out_of_range("Index out of range");
        return m_pVect[index];
    }

    friend ostream& operator<<(ostream &os, const CVector<T> &vec) {
        for(size_t i=0; i < vec.m_count ; ++i)
            os << vec.m_pVect[i] << " ";
        return os;
    }
};

template <typename T>
CVector<T>::CVector(CVector &v)
{
    m_pVect = new T[v.m_max]();
    m_count = v.m_count;
    m_max = v.m_max;
    m_delta = v.m_delta;
    for(size_t i = 0; i < m_count; i++){
        m_pVect[i] = v[i];
    }
}

template <typename T>
CVector<T>::CVector(size_t n)
{
    m_pVect = new T[n]();
    m_max = n;
}

template <typename T>
CVector<T>::CVector(CVector &&v)
    : m_pVect(v.m_pVect), m_count(v.m_count), m_max(v.m_max), m_delta(v.m_delta)
{
    v.m_pVect = nullptr;
    v.m_count = 0;
    v.m_max = 0;
    v.m_delta = 10;
}

// TODO (Nivel 1): hacer dinamico el delta de crecimiento - done
template <typename T>
void CVector<T>::resize(){
    while(m_count >= 10 * m_delta){
        m_delta *= 10;
    }
    T *pTmp = new T[m_max + m_delta]();
    for(size_t i=0; i < m_max ; ++i)
        pTmp[i] = m_pVect[i];
    delete [] m_pVect;
    m_max += m_delta;
    //cout << "m_max : "<< m_max << " m_delta : " << m_delta << endl; 
    m_pVect = pTmp;
}

template <typename T>
void CVector<T>::resize(size_t new_size)
{
    if(new_size < m_max){
        return;
    }
    while(m_count >= 10 * m_delta){
        m_delta *= 10;
    }
    T *pTmp = new T[new_size]();
    for(size_t i=0; i < m_max ; ++i)
        pTmp[i] = m_pVect[i];
    delete [] m_pVect;
    m_max = new_size;
    m_pVect = pTmp;
}

template <typename T>
CVector<T>::~CVector()
{
    delete [] m_pVect;
}

// TODO (ya está hecha): la funcion insert debe permitir que el vector crezca si ha desbordado
template <typename T>
void CVector<T>::insert(const T &elem) {
    if(m_count == m_max)
        resize();
    m_pVect[m_count++] = elem;
}

#endif // __VECTOR_H__