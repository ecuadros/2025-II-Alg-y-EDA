#include <iostream>
#include "DemoVector.h"
#include "vector.h"
using namespace std;

template <typename _T>
struct GeneralTraits{
    using value_type = _T;
    
};

void DemoVector(){
    CVector< GeneralTraits<int> > vector(10);
    // vector.insert(5);
    // TODO  (Nivel 1) habilitar el uso de []
    vector[3] = 8;
    // TODO  (Nivel 2) habilitar que el vector pueda ser escrito con cout <<
    cout << vector << endl;
}