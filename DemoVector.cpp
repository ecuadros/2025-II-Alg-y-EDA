#include <iostream>
#include "DemoVector.h"
#include "vector.h"
using namespace std;

void DemoVector(){
    CVector vector(10);
    vector.insert(5);
    // TODO habilitar el uso de []
    vector[3] = 8;
    // TODO habilitar que el vector pueda ser escrito con cout <<
    cout << vector << endl;
}