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

    friend std::ostream& operator<<(std::ostream& os, CVector& v);

    T& operator[](size_t index);
};

template <typename T>
CVector<T>::CVector(size_t n){

}

// TODO (Nivel 1): hacer dinamico el delta de crecimiento
template <typename T>
void CVector<T>::resize(){
    size_t delta = m_max * 0.5;
    T *pTmp = new T[m_max + delta];
    for(auto i=0; i < m_max ; ++i)
        pTmp[i] = m_pVect[i];
    delete [] m_pVect;
    m_max += delta;
    m_pVect = pTmp;
}

// TODO (ya está hecha): la funcion insert debe permitir que el vector crezca si ha desbordado
template <typename T>
void CVector<T>::insert(T &elem){
    if(m_count == m_max)
        resize();
    m_pVect[m_count++] = elem;
}

// TODO  (Nivel 2) habilitar que el vector pueda ser escrito con cout <<
// cout << vector << endl;
template <typename T>
std::ostream& operator<<(std::ostream& os, CVector<T>& v){
    os << "CVector: [ ";

    for(size_t i = 0; i<v.m_max; i++){
        os << v.m_pVect[i]<< " ";
    }
    os << "]"<< endl;

    return os;
}

// TODO  (Nivel 1) habilitar el uso de []
// vector[3] = 8;
template <typename T>
T& CVector<T>::operator[](size_t index){
    if(index >= m_max){
        std::cerr<<"Error de ìndice";
        exit(0);
    }
    return m_pVect[index];
}


#endif // __VECTOR_H__