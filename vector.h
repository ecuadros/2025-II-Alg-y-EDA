#ifndef __VECTOR_H__
#define __VECTOR_H__
#include <cstring>
#include <iostream>
#include <cmath>
// PC1: deben hacer:
//      2 problemas de nivel 2
//      3 problemas de nivel 1
// Cada solucion enviarla como un Pull request

// TODO (Nivel 2): Agregar Traits

// TODO (Nivel 2): Agregar Iterators (forward, backward)

// TODO (Nivel 1): Agregar Documentacion para generar con doxygen

// TODO  (Nivel 2): Agregar control de concurrencia en todo el vector

/**
 * @class CVector
 * @brief Esta es la implementación de un vector con template
 */
template <typename T>
class CVector{

    T      *m_pVect = nullptr;
    size_t  m_count = 0; // How many elements we have now?
    size_t  m_max   = 0; // Max capacity
    
    // Delta
    size_t  delta = 0;
    float   k = 0.1;
    size_t  min_delta = 32;
public:
    // TODO  (Nivel 1) Agregar un constructor por copia
    /**
     * @brief Copy constructor: A type of constructor that creates an objecto using another object of the same class
     * @param v A reference to an object of the same class as an argument
     */
    CVector(CVector &v);

    /**
     * @brief Constructor asign the m_max
     * @param n the size of the CVector
     */
    CVector(size_t n);
    // TODO  (Nivel 2): Agregar un move constructor
    /**
     * @brief Move Constructor: A special constructor to transfer the contents of one objecto to another without copying the data
     * @param v An rvalue reference to another object of the same class
     */
    CVector(CVector &&v);

    // TODO: (Nivel 1) implementar el destructor de forma segura
    /**
     * @brief Destructor virtual
     */
    virtual ~CVector();
    /**
     * @brief Insert a new element to vector
     * @param elem The element to insert to the vector
     */
    void insert(T const &elem);
    /**
     * @brief Resize the max using a dynamic delta
     */
    void resize();
    /**
     * @brief Overloading the array index operator or subscript
     */
    T& operator[](size_t);

    /**
     * @brief Overloading the << operator
     * @param os the ostream variable to return it
     * @param v the CVector 
     */
    friend std::ostream& operator<<(std::ostream &os, const CVector& v){
        os << "[";
        for (size_t i = 0; i < v.m_max; ++i){
            os << v.m_pVect[i];
            if (i < v.m_max - 1)
                os << ",";
        }
        os << "]";
        return os;
    }
};


template <typename T>
CVector<T>::CVector(CVector &v)
    :m_count(v.m_count), m_max(v.m_max)
{
    m_pVect = new T[m_max];
    for (size_t i = 0; i < m_count; i++)
        m_pVect[i] = v.m_pVect[i];
    
    // std::cout << "Iniciando constructor por copia" << std::endl;
}


template <typename T>
CVector<T>::CVector(size_t n)
    :m_pVect(new T[n]), m_count(0), m_max(n)
{
    // std::cout << "Iniciando con size_t" << std::endl;
}


// template <typename T>
// CVector<T>::CVector(size_t n){
//     m_pVect = new T[n];
//     m_max = n;
// }


template <typename T>
CVector<T>::CVector(CVector &&v)
    :CVector(v)
{
    // std::cout << "Move Constructor: " << &v << std::endl;
    v.m_pVect = nullptr;
    v.m_count = 0;
    v.m_max = 0;

}


template <typename T>
CVector<T>::~CVector(){
    // std::cout << "Destructor: " << m_pVect << std::endl;
    delete[] m_pVect;
}


// TODO (Nivel 1): hacer dinamico el delta de crecimiento
template <typename T>
void CVector<T>::resize(){
    delta = std::max((size_t) std::ceil((float)m_max * k), min_delta);
    T *pTmp = new T[m_max+delta];
    for(size_t i=0; i < m_max ; ++i)
        pTmp[i] = m_pVect[i];
    delete [] m_pVect;
    m_max += delta;
    m_pVect = pTmp;
}

// TODO (ya está hecha): la funcion insert debe permitir que el vector crezca si ha desbordado
template <typename T>
void CVector<T>::insert(T const &elem){
    if(m_count == m_max)
        resize();
    m_pVect[m_count++] = elem;
}

template <typename T>
T& CVector<T>::operator[](size_t index){
    if(index >= m_max){
        // std::cout << index << std::endl;
        // std::cout << m_count << std::endl;
        throw std::out_of_range("Out of range");
        
    }
    return m_pVect[index];
}

// template <typename T>
// std::ostream& CVector<T>::operator<<(std::ostream &os, const CVector &v) {
    
// };

#endif // __VECTOR_H__