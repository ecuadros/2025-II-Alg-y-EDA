/**
 * @file
 * @brief Demostración de CVector: inserción, delta dinámico y operator<<.
 *
 * Valida:
 * - Inserciones con redimensionamiento.
 * - Impresión con operator<<.
 * - Accesores size() y capacity().
 */

#include <iostream>
#include "DemoVector.h"
#include "vector.h"
using namespace std;

/**
 * @brief Ejecuta la demostración de CVector.
 */
void DemoVector(){
    cout << "== Demo CVector ==" << '\n';

    // Crear un vector con capacidad inicial 5
    CVector<int> v(5);
    for (int i = 1; i <= 5; ++i) v.insert(i * 10);

    // Validación del operator<< (Nivel 2)
    cout << "Contenido del vector: " << v << '\n';

    // Validación de accesores
    cout << "Tamaño actual: " << v.size()
         << " | Capacidad: " << v.capacity() << '\n';
}
