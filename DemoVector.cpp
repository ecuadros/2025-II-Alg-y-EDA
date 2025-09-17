#include <iostream>
#include "DemoVector.h"
#include "vector.h"
using namespace std;

void DemoVector(){
    CVector<int> vector(10);
    // vector.insert(5);
    // TODO  (Nivel 1) habilitar el uso de []
    vector[3] = 8;
    // TODO  (Nivel 2) habilitar que el vector pueda ser escrito con cout <<
    cout << vector << endl;
}