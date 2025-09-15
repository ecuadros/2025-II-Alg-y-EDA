#include <iostream>
#include "DemoVector.h"
#include "vector.h"

void DemoVector(){
    CVector<int> vector(6);
    vector.insert(5);
    // TODO  (Nivel 1) habilitar el uso de []
    vector[3] = 8;
    std::cout << vector[0] << std::endl;
    std::cout << vector[3] << std::endl;

    
    // return;
    // TODO  (Nivel 2) habilitar que el vector pueda ser escrito con cout <<
    std::cout << vector << std::endl;
}