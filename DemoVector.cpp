#include <iostream>
#include "DemoVector.h"
#include "vector.h"
using namespace std;

void DemoVector(){
    CVector<int> vector(10);

    vector.insert(5);
    vector.insert(10);
    vector.insert(20);
    vector.insert(40);
    cout << vector << endl;

    // TODO  (Nivel 1) habilitar el uso de [] - done
    vector[3] = 8;
    // TODO  (Nivel 2) habilitar que el vector pueda ser escrito con cout << - done
    cout << vector << endl;

    CVector<int> vector2(10);
    for(auto i = 0; i < 1001; i++){
        vector2.insert(i);
    }
    //cout << vector2 << endl;

    CVector<int> vector3(vector);
    vector3.insert(1000);
    cout << vector3 << endl;
    CVector<int> vector4(move(vector));
    vector4.insert(2000);
    cout << vector4 << endl;
}