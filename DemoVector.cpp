#include <iostream>
#include "DemoVector.h"
#include "vector.h"
using namespace std;

void DemoVector(){
    CVector<int> vector(10);
    vector.Insert(5, 0);
    vector[3] = 8;
    cout << vector << endl;
}