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
    // TODO  (Nivel 1) Agregar un constructor por copia
    CVector(const CVector &v); // El constructor por copia no debe modificar al objeto original

    CVector(size_t n);
    // TODO  (Nivel 2): Agregar un move constructor
    CVector(CVector &&v);

    // TODO: (Nivel 1) implementar el destructor de forma segura
    virtual ~CVector();
    void insert(T &elem);
    void resize();

    // TODO:  (Nivel 1) habilitar el uso de []
    T& operator[](size_t index){ // Permite modificar el valor (vector[i] = nuevo_valor)
        return m_pVect[index];
    }

    const T& operator[](size_t index) const { // version solo lectura
        return m_pVect[index];
    }
    // TODO  (Nivel 2) habilitar que el vector pueda ser escrito con cout <<
    template <typename U>
    friend std::ostream& operator<<(std::ostream& os, const CVector<U>& vector); // Una función friend (no es miembro de la clase pero puede acceder a sus miembros privados)

};  

template <typename T>
CVector<T>::CVector(size_t n){

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

// TODO (ya está hecha): la funcion insert debe permitir que el vector crezca si ha desbordado
template <typename T>
void CVector<T>::insert(T &elem){
    if(m_count == m_max)
        resize();
    m_pVect[m_count++] = elem;
}

// Implementación del Destructor
template <typename T>
CVector<T>::~CVector(){
    if(m_pVect != nullptr){ // Evita borrar un puntero nulo
        delete [] m_pVect;
        m_pVect = nullptr; // Evita dangling pointer
        m_count = 0;
        m_max   = 0;
    }
}

// Implementación del operador << como función amiga
template<typename T>
std::ostream& operator<<(std::ostream& os, const CVector<T>& vector){
    os << "[";
    for(size_t i = 0; i < vector.m_count; ++i){
        os << vector.m_pVect[i];
        if(i < vector.m_count - 1)
            os << ", ";
    }
    os << "]";
    return os;  //Retorna el stream para permitir encadenamiento 
}

// Implementación del constructor por copia
template <typename T>
CVector<T>::CVector(const CVector<T>& v)
    : m_count(v.m_pVect != nullptr ? v.m_count : 0), // Inicializa m_count
      m_max(v.m_pVect != nullptr ? v.m_max : 0),      // Inicializa m_max
      m_pVect(nullptr)                                 // Inicializa m_pVect
{
    if (v.m_pVect != nullptr){
        // Reserva memoria
        m_pVect = new T[m_max];
        // Copiar elementos
        for(size_t i = 0; i < m_count; ++i){
            m_pVect[i] = v.m_pVect[i];
        }
    }
}


#endif // __VECTOR_H__