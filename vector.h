#ifndef __VECTOR_H__
#define __VECTOR_H__
#include <ostream>
#include <utility> // std::move

/**
 * @file
 * @brief Definición de la clase plantilla CVector (vector dinámico minimal).
 *
 * Crece con delta lineal configurable o en modo multiplicativo (delta=0).
 * Incluye constructor por copia (deep copy), move constructor, y operator<<.
 */

/**
 * @brief Vector dinámico inspirado en el enfoque del curso/libro.
 *
 * @tparam T Tipo de dato almacenado.
 *
 * Características:
 * - Inserción amortizada con @c resize() cuando hay desbordamiento.
 * - Crecimiento lineal (delta>0) o multiplicativo (delta==0).
 * - Gestión segura de memoria (destructor) y semántica de copia/movimiento.
 */
template <typename T>
class CVector{
    /** @brief Puntero al buffer dinámico. */
    T      *m_pVect = nullptr;
    /** @brief Número de elementos actualmente ocupados. */
    size_t  m_count = 0;
    /** @brief Capacidad reservada del buffer. */
    size_t  m_max   = 0;
    /** @brief Delta de crecimiento (0 => modo multiplicativo). */
    size_t  m_delta = 10;

public:
    /**
     * @brief Constructor por copia (deep copy).
     * @param v Otro vector a copiar.
     */
    CVector(const CVector &v);

    /**
     * @brief Constructor con capacidad inicial.
     * @param n Capacidad inicial (si n>0 se reserva memoria).
     */
    CVector(size_t n);

    /**
     * @brief Move constructor.
     * @param v Vector de origen cuyos recursos serán transferidos.
     */
    CVector(CVector &&v) noexcept;

    /**
     * @brief Destructor seguro (libera el buffer y deja estado válido).
     */
    virtual ~CVector();

    /**
     * @brief Inserta un elemento por copia.
     * @param elem Elemento a insertar.
     *
     * Si @c size()==capacity() se invoca @c resize().
     */
    void insert(const T &elem);

    /**
     * @brief Inserta un elemento por movimiento.
     * @param elem Elemento a insertar (rvalue).
     *
     * Si @c size()==capacity() se invoca @c resize().
     */
    void insert(T &&elem);

    /**
     * @brief Aumenta la capacidad del vector.
     *
     * - @c m_delta > 0  ⇒ crecimiento lineal (@c m_max += m_delta).
     * - @c m_delta == 0 ⇒ crecimiento multiplicativo (~x2).
     */
    void resize();

    /**
     * @brief Configura el delta de crecimiento.
     * @param d Nuevo delta (0 activa modo multiplicativo).
     */
    void set_delta(size_t d){ m_delta = d; }

    /// @name Accesores
    ///@{
    /** @brief Número de elementos. */ 
    size_t size() const noexcept { return m_count; }
    /** @brief Capacidad actual. */
    size_t capacity() const noexcept { return m_max; }
    ///@}

    /**
     * @brief Operador de salida para imprimir el vector.
     * @tparam U Tipo almacenado
     * @param os Stream de salida
     * @param v  Vector a imprimir
     * @return @c os para encadenar
     *
     * Imprime con el formato: @code [e0, e1, e2, ...] @endcode
     */
    template <typename U>
    friend std::ostream& operator<<(std::ostream& os, const CVector<U>& v);
};

// destructor seguro
template <typename T>
CVector<T>::~CVector() {
    delete [] m_pVect;   // libera el buffer dinámico
    m_pVect = nullptr;   // deja el puntero en estado nulo
    m_count = 0;         
    m_max   = 0;         
}

// TODO (Nivel 1): hacer el constructor por tamaño
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

/// Move constructor (Nivel 2)
template <typename T>
CVector<T>::CVector(CVector<T> &&v) noexcept
: m_pVect(v.m_pVect), m_count(v.m_count), m_max(v.m_max), m_delta(v.m_delta) {
    v.m_pVect = nullptr;
    v.m_count = 0;
    v.m_max   = 0;
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
void CVector<T>::insert(const T &elem){
    if (m_count == m_max) resize();
    m_pVect[m_count++] = elem;
}

// insert por movimiento
template <typename T>
void CVector<T>::insert(T &&elem){
    if (m_count == m_max) resize();
    m_pVect[m_count++] = std::move(elem);
}

/// operator<< (Nivel 2)
template <typename U>
std::ostream& operator<<(std::ostream& os, const CVector<U>& v){
    os << "[";
    for (size_t i = 0; i < v.m_count; ++i) {
        if (i) os << ", ";
        os << v.m_pVect[i];
    }
    return os << "]";
}


#endif // __VECTOR_H__