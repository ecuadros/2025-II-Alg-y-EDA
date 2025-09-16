#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <iostream>
#include <stdexcept>

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
    // TODO  (Nivel 1) Agregar un constructor por copia
    CVector(CVector &v);

    CVector(size_t n);
    // TODO  (Nivel 2): Agregar un move constructor
    CVector(CVector &&v);

    // TODO: (Nivel 1) implementar el destructor de forma segura
    vrtual CVector();
    void insert(T &elem);
    void resize();

    T& operator[](size_t index);
    
    size_t size();
};

template <typename T>
CVector<T>::CVector(size_t n){
    m_max = n;
    m_pVect = (n > 0) ? new T[n] : nullptr;
    m_count = 0;
}

//destructor
template <typename T>
CVector<T>::~CVector(){
    delete [] m_pVect;
}

// constructor por copia
template <typename T>
CVector<T>::CVector(CVector<T> &v) {
    m_count = v.m_count;
    m_max = v.m_max;
    m_pVect = new T[m_max];
    for (size_t i = 0; i < m_count; ++i) {
        m_pVect[i] = v.m_pVect[i];  
    }
}

//definimos el operador []
template <typename T>
T& CVector<T>::operator[](size_t index){
    if(index >= m_count)
        throw std::out_of_range("Index out of range");
    return m_pVect[index];
}

//funcion para determinar el tamaño del vector
template <typename T>
size_t CVector<T>::size() {
    return m_count;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, CVector<T>& v) {
    size_t n = v.size();
    os << "[";
    for (size_t i = 0; i < n; ++i) {
        os << v[i];
        if (i + 1 < n) 
            os << ", ";
    }
    os << "]";
    return os;
}


// TODO (Nivel 1): hacer dinamico el delta de crecimiento
template <typename T>
void CVector<T>::resize(){
    T *pTmp = new T[m_max+10];
    for(size_t i=0; i < m_max ; ++i)
        pTmp[i] = m_pVect[i];
    delete [] m_pVect;
    m_max += 10;
    m_pVect = pTmp;
}

// TODO (ya está hecha): la funcion insert debe permitir que el vector crezca si ha desbordado
template <typename T>
void CVector<T>::insert(T &elem){   //para que acepte lvalues y rvalues le tengo que agregar const a T& elem
    if(m_count == m_max)
        resize();
    m_pVect[m_count++] = elem;
}

#endif // __VECTOR_H__