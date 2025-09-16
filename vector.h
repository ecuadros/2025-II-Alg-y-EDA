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
	size_t m_currentDelta = 0;
public:
	
// TODO  (Nivel 1) Agregar un constructor por copia
	CVector(CVector &v);

	CVector(size_t n);

	// TODO  (Nivel 2): Agregar un move constructor
	CVector(CVector &&v);

	// TODO: (Nivel 1) implementar el destructor de forma segura
	virtual ~CVector();
	
	void insert(const T &elem);
	
	T& operator[](size_t index);
	
	size_t size() const { return m_count; }

private:

	void resize(size_t delta = 0);
	void Destroy();
};

template <typename T>
CVector<T>::CVector(size_t n){
	m_pVect = new T[n];
	m_max = n;
	m_currentDelta = n;
}

template <typename T>
CVector<T>::CVector(CVector &v) 
	: m_max(v.m_max), m_count(v.m_count) {

	if (m_max > 0)
		m_pVect = new T[m_max];
	for (size_t i = 0; i < m_count; ++i)
		m_pVect[i] = v[i];       
}

template <typename T>
void CVector<T>::resize(size_t delta) {
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
	m_count = 0; 
	m_max   = 0;
	delete [] m_pVect;
	m_pVect = nullptr;
}

// TODO (ya está hecha): la funcion insert debe permitir que el vector crezca si ha desbordado
template <typename T>
void CVector<T>::insert(const T &elem){
	if(m_count == m_max)
		resize();
	m_pVect[m_count++] = elem;
}

template <typename T>
T& CVector<T>::operator[](size_t index) {
	if(index > m_max)
		resize(index - m_max + 1);
	return m_pVect[n];
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