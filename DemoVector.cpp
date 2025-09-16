#include <iostream>
#include "DemoVector.h"
#include "vector.h"
using namespace std;

void DemoVector(){
    CVector<int> vector(10);
    vector.insert(15, 0);
    vector[3] = 8;
    vector[12] = 10;
    cout<< vector[12] <<endl;
    // TODO  (Nivel 2) habilitar que el vector pueda ser escrito con cout <<
    cout << vector << endl;
}