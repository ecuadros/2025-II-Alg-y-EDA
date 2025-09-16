#ifndef __VECTOR_H__
#define __VECTOR_H__

// PC1: deben hacer:
//      2 problemas de nivel 2
//      3 problemas de nivel 1
// Cada solucion enviarla como un Pull request

// TODO (Nivel 2): Agregar Traits

// TODO (Nivel 2): Agregar Iterators (forward, backward)

// TODO  (Nivel 2): Agregar control de concurrencia en todo el vector
template <typename T>
class CVector{
	T      *m_pVect = nullptr;
	size_t  m_max   = 0; // Max capacity
	size_t m_currentDelta = 0;
public:
	
	/// @brief Default constructor that initialize the vector with a size of 5.
	CVector();

	/// @brief Constructor that initialize the vector witha size of n.
	/// @param n The size to initialize the vector.
	CVector(CVector &v);

	/// @brief Constructor that copy the value of another vector.
	/// @param v The vector to copy.
	CVector(size_t n);

	/// @brief 
	/// Constructor that moves a vector by reference to another vector.
	/// @param v The vector to move.
	CVector(CVector &&v);

	// TODO: (Nivel 1) implementar el destructor de forma segura

	virtual ~CVector();
	
	/// @brief Inserts the given element into the given position.
	/// @param elem The element to insert.
	/// @param position The position where to insert the element.
	/// @note You can also use the [] operator.
	void Insert(const T &elem, const size_t position);
	
	/// @brief Operator that returns the reference of the element of position n.
	/// @param index The position to return the reference.
	/// @return The reference of the element of position n.
	T& operator[](size_t index);

	/// @brief Returns the current max size of the vector.
	/// @return The max size of the vector.
	size_t GetMaxSize() const { return m_max; }

private:

	void Resize(size_t delta = 0);
	void Destroy();
};

template <typename T>
CVector<T>::CVector() : CVector(5) {};

template <typename T>
CVector<T>::CVector(size_t n){
	m_pVect = new T[n];
	m_max = n;
	m_currentDelta = n;
}

template <typename T>
CVector<T>::CVector(CVector &v) 
	: m_pVect(v.m_pVect), m_max(v.m_max) {}

template <typename T>
CVector<T>::CVector(CVector &&v) : CVector(v) {}

template <typename T>
void CVector<T>::Resize(size_t delta) {
	size_t auxDelta = m_currentDelta;

	if(delta != 0){
		if(delta < m_currentDelta) auxDelta = delta;
		else {
			m_currentDelta = delta;
			auxDelta = m_currentDelta;
		}
	}

	T *pTmp = new T[m_max + auxDelta];

	for(size_t i = 0; i < m_max; ++i)
		pTmp[i] = m_pVect[i];

	delete [] m_pVect;
	m_max += auxDelta;

	m_pVect = pTmp;
};

template <typename T>
	CVector<T>::~CVector(){
	Destroy();
}

template <typename T>
void CVector<T>::Destroy(){
	m_max   = 0;
	delete [] m_pVect;
	m_pVect = nullptr;
}

// TODO (ya está hecha): la funcion insert debe permitir que el vector crezca si ha desbordado
template <typename T>
void CVector<T>::Insert(const T &elem, const size_t position){
	this->operator[](position) = elem;
}

template <typename T>
T& CVector<T>::operator[](size_t index) {
	if(index > m_max)
		resize(index - m_max + 1);
	return m_pVect[index];
}

template <typename T>
std::ostream& operator<<(std::ostream& os, CVector<T>& vec) {
	os << "[";
		for (size_t i = 0; i < vector.m_max; i++) {
			os << vector[i];
			os << ", ";
		}
	os << "]";
	return os;
}

#endif // __VECTOR_H__