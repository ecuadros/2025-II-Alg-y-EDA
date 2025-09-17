# 2025-II-Alg-y-EDA

## PC1

> [!TIP]
> Recomendaciones para manejar flujo de revisión:
> 
> - Manejar todo el código del docente en una rama principal (usar carpetas en vez de ramas)
> - Cada alumno tiene su propia rama en donde haria sus PRs a la rama main (rama del profesor)
> - Implementar un formateador de código para evitar confusion en la lectura de cambios del PR
>   
> ✅ Ventajas:
> - Cada estudiante trabaja en su espacio sin interferir con otros (no habría conflictos)
> - El profesor puede rastrear el progreso individual de cada alumno
> - Los PRs muestran solo los cambios específicos del estudiante
> - Cuando el profesor haga un cambio en main, los alumnos solo harian un merge a su rama

## Nivel 1

### a) Habilitar el uso de []

Primero declaramos el modificador del operador []. Este debe recibir un size_t y retornar una referencia a T.

```cpp
T& operator[](size_t index);
```

Luego implementamos la funcionalidad

```cpp
template <typename T>
T& CVector<T>::operator[](size_t index) {
    if (index >= m_count) {
        throw std::out_of_range("Index out of range");
    }
    return m_pVect[index];
}
```

### b) Agregar un constructor por copia

Declaramos la funcion que va a recibir como parametro una referencia a otro CVector

```cpp
CVector(CVector &v);
```

Implementamos la funcionalidad, donde copiamos los atributos de v para crear una copia nueva.

```cpp
template <typename T>
CVector<T>::CVector(CVector &v) : m_max(v.m_max), m_count(v.m_count) {
    if (m_max > 0) {
        m_pVect = new T[m_max];
        for (size_t i = 0; i < m_count; ++i) {
            m_pVect[i] = v.m_pVect[i];
        }
    } else {
        m_pVect = nullptr;
    }
}
```

### c) Implementar el destructor de forma segura

Declaramos el destructor

```cpp
~CVector();
```

Implementamos la funcionalidad, donde liberamos la memoria dinámica y ponemos los atributos a 0 / nulo.

```cpp
template <typename T>
CVector<T>::~CVector(){
    delete[] m_pVect;
    m_pVect = nullptr;
    m_count = 0;
    m_max = 0;
}
```

## Nivel 2

### a) Habilitar que el vector pueda ser escrito con cout <<

Implementamos la funcion amiga inline, que recibe como parametro un std::ostream y una constante referencia a CVector.

```cpp
friend std::ostream& operator<<(std::ostream& os, const CVector& vec) {
        os << "[";
        for (size_t i = 0; i < vec.m_count; ++i) {
            os << vec.m_pVect[i];
            if (i < vec.m_count - 1) {
                os << ", ";
            }
        }
        os << "]";
        return os;
    }
```

### b) Agregar un move constructor

Declaramos la funcion que va a recibir como parametro un CVector por r-value reference.

```cpp
CVector(CVector &&v) noexcept;
```

Implementamos el move constructor, donde movemos los atributos de v a nuestro nuevo CVector, y ponemos los atributos de v a 0 / nulo.

```cpp
template <typename T>
CVector<T>::CVector(CVector &&v) noexcept
: m_pVect(v.m_pVect), m_max(v.m_max), m_count(v.m_count) {
    v.m_pVect = nullptr; 
    v.m_max = 0;
    v.m_count = 0;
}
```

## Ejemplo de uso

```cpp
#include <iostream>
#include "DemoVector.h"
#include "vector.h"

using namespace std;

void DemoVector(){
    int valueToInsert = 5;

    CVector<int> vector(1);

    vector.insert(valueToInsert);

    // TODO  (Nivel 1) habilitar el uso de []

    vector[0] = 7;
    vector.insert(valueToInsert);

    // TODO  (Nivel 2) habilitar que el vector pueda ser escrito con cout <<
    cout << "Vector inicial: " << vector << endl;

    const CVector<int> vector2 = vector;

    cout <<"Vector 2 copia del 1: " << vector2 << endl;

    const CVector<int> vector3 = std::move(vector);

    cout << "Vector 3 movido del 1:  " <<  vector3 << endl;
    cout << "Vector 1 muerto: " << vector << endl;
}
```

Esto nos da como output

```
Hello Alg y EDA-UNI
Vector inicial: [7, 5]
Vector 2 copia del 1: [7, 5]
Vector 3 movido del 1:  [7, 5]
Vector 1 muerto: []
```