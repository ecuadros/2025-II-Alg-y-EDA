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
    CVector(CVector &v);
    CVector(size_t n);
    // COMPLETADO (Nivel 2): Move constructor implementado
    CVector(CVector &&v) noexcept;

    // COMPLETADO (Nivel 1): Destructor implementado de forma segura
    virtual ~CVector();
    void insert(T &elem);
    T&   operator[](size_t index);
    size_t size() const { return m_count; }
private:
    void resize();
    void Init(size_t n);
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

// COMPLETADO (Nivel 2): Move constructor para optimizar transferencias
template <typename T>
CVector<T>::CVector(CVector &&v) noexcept 
          : m_pVect(v.m_pVect), 
            m_count(v.m_count),
            m_max(v.m_max) {
    // Resetear el objeto fuente para dejarlo en estado válido
    v.m_pVect = nullptr;
    v.m_max = 0;
    v.m_count = 0;
}

// COMPLETADO (Nivel 1): Crecimiento dinámico duplicando la capacidad
template <typename T>
void CVector<T>::resize(){
    size_t new_max = (m_max == 0) ? 1 : m_max * 2;
    T *pTmp = new T[new_max];
    for(size_t i=0; i < m_max ; ++i)
        pTmp[i] = m_pVect[i];
    delete [] m_pVect;
    m_max = new_max;
    m_pVect = pTmp;
}

template <typename T>
void CVector<T>::Init(size_t n){
    Destroy();
    if (n > 0) {
        m_max = n;           // Usar el parámetro n
        m_pVect = new T[m_max]; // Allocar la memoria
        m_count = 0;         // Empezar con 0 elementos
    }
}

template <typename T>
    CVector<T>::~CVector(){
    Destroy();
}

template <typename T>
void CVector<T>::Destroy(){
    if (m_pVect != nullptr) {
        delete [] m_pVect;
        m_pVect = nullptr;
    }
    m_count = 0; 
    m_max   = 0;
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