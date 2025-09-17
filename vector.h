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

/**
    *   @file vector.h
    *   @brief A simple implementation of a dynamic array (vector) in C++
    *   @date 2025
*/
template <typename T>
class CVector{
    /**
    *   @brief Pointer to the dynamic array
    */
    T      *m_pVect = nullptr;

    /**
    *   @brief Current number of elements in the vector
    */
    size_t  m_count = 0; // How many elements we have now?

    /**
    *   @brief Max number of elements the vector can hold
    */
    size_t  m_max   = 0; // Max capacity
public:
    // TODO  (Nivel 1) Agregar un constructor por copia
    /**
    *   @brief Copy constructor
    */
    CVector(CVector &v);

    /**
    *   @brief Constructor, initializes vector with given size
    *   @param n initial size of the vector, n > 0
    */
    CVector(size_t n);
    // TODO  (Nivel 2): Agregar un move constructor

    /**
    *   @brief Move constructor
    */
    CVector(CVector &&v);

    // TODO: (Nivel 1) implementar el destructor de forma segura

    /**
    *   @brief Destructor, frees allocated memory
    */
    virtual ~CVector();

    /**
    *   @brief Insert a new element at the end of the vector
    *   @param elem element to insert
    */
    void insert(T &elem);

    /**
    *   @brief Overload operator [] to access elements in the vector
    *   @param index position of the element to access
    *   \return reference to the element at the given index
    */
    T&   operator[](size_t index);

    /**
    *   @brief Get the current size of the vector
    *   \return number of elements
    */
    size_t size() const { return m_count; }
private:
    /**
    *   @brief Resize the internal array to have more capacity
    */
    void resize();

    /**
    *   @brief Initialize the CVector object, used in constructor and assignment
    *   @param n initial size of the vector
    */
    void Init(size_t n);

    /**
    *   @brief Destroy the CVector object, used in destructor and Init
    */
    void Destroy();
};

template <typename T>
CVector<T>::CVector(size_t n){
    Init(n);
}

template <typename T>
CVector<T>::CVector(CVector &v) 
          : m_max(v.m_max), 
            m_count(v.m_count) {
    if (m_max > 0)
        m_pVect = new T[m_max];
    for (size_t i = 0; i < m_count; ++i)
        m_pVect[i] = v[i];       
}

// TODO (Nivel 1): hacer dinamico el delta de crecimiento
template <typename T>
void CVector<T>::resize(){
    T *pTmp = new T[m_max+10];
    for(auto i=0; i < m_max ; ++i)
        pTmp[i] = m_pVect[i];
    delete [] m_pVect;
    m_max += 10;
    m_pVect = pTmp;
}

template <typename T>
void CVector<T>::Init(size_t n){
    Destroy();
    resize();
}

template <typename T>
    CVector<T>::~CVector(){
    Destroy();
}

template <typename T>
void CVector<T>::Destroy(){
    m_count = 0; 
    m_max   = 0;
    delete [] m_pVect;
    m_pVect = nullptr;
}

// TODO (ya está hecha): la funcion insert debe permitir que el vector crezca si ha desbordado
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

template <typename T>
std::ostream& operator<<(std::ostream& os, CVector<T>& vec) {
    // os << "[";
    for (size_t i = 0; i < vec.size(); ++i)
        os << vec[i] << " ";
    // os << "]";
    return os;
}

#endif // __VECTOR_H__


/**
 * @mainpage CVector Implementation
 *
 * @section intro_sec Introduction
 * Implementation of a simple dynamic array (vector) in C++ for the course Advanced Algorithms and Data Structures 2025-II.
 * Check the defined methods in the [CVector header file](classCVector.html).
 *
 * @section install_sec Run
 * To run the code, compile using GNU Make with the command `make all`.
*/
