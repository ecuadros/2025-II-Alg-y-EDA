#include <iostream>
#include "DemoVector.h"
#include "vector.h"
using namespace std;

void DemoVector(){
    // CVector vector(10);
    // vector.insert(5);
    // TODO  (Nivel 1) habilitar el uso de []
    // vector[3] = 8;
    // TODO  (Nivel 2) habilitar que el vector pueda ser escrito con cout <<
    // cout << vector << endl;

    CVector<int> vector(10);
    int val1 = 5, val2 = 8, val3 = 12;
    vector.insert(val1);
    vector.insert(val2);
    vector.insert(val3);
    vector[0] = 100;
    cout << vector << endl;
}