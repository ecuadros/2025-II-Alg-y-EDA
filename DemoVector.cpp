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

    //ejemplo donde se usa el operador <<
    CVector<int> vector(10);
    for (int i = 0; i < 10; ++i){
        vector.insert(i);
    }
    cout << vector << endl;

    vector[3] = 8;  //probamos que el operador[] ya funciona
    cout << vector << endl;
}