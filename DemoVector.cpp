#include <iostream>
#include "DemoVector.h"
#include "vector.h"
using namespace std;

void DemoVector(){
    // Creamos un vector con 10 elementos
    CVector<int> vector(10);

    // Insertamos algo
    vector.insert(5);

    // ===== Nivel 1: uso de [] =====
    vector[3] = 8;

    // ===== Nivel 2: imprimir con cout << =====
    cout << "Contenido del vector: " << vector << endl;
}
