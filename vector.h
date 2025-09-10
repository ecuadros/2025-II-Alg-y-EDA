#ifndef __VECTOR_H__
#define __VECTOR_H__

// TODO: Agregar Traits

// TODO: Agregar Iterators (forward, backward)

// TODO: Agregar Documentacion para gebnerar con doxygen

template <typename T>
class CVector{
   
    T *m_pVect = nullptr;
public:
    CVector(size_t n);
    void insert(T &elem);

};

#endif // __VECTOR_H__