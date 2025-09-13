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