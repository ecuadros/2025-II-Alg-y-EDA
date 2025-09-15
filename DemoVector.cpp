#include <iostream>
#include "DemoVector.h"
#include "vector.h"
using namespace std;

void DemoVector(){
    CVector<int> vector(10);
    int val = 5;
    vector.insert(val);
    int val2 = 10;
    vector.insert(val2);
    cout << "\nVector original: " << vector << endl;
    cout << "vector[0] original: " << vector[0] << endl;
    cout << "vector[1] original: " << vector[1] << endl;
    // TODO  (Nivel 1) Agregar un constructor por copy
    CVector<int> copy = vector; // Usando el constructor por copy
    cout << "\nCopia: " << copy << endl;
    cout << "Original después de copy: " << vector << endl; // El original no debe cambiar
    cout << "\nModificando la copia:" << endl;
    copy[0] = 20; // Modificando la copia
    cout << "Copia modificada: " << copy << endl;
    cout << "Original después de modificar la copia: " << vector << endl; // Tampoco cambia
    // TODO  (Nivel 2): Agregar un move constructor
    cout << "\nUsando move constructor:" << endl;
    CVector<int> mov = std::move(vector); // Usando el move constructor
    cout << "Movida: " << mov << endl;
    cout << "Original después de move: " << vector << endl;

    // TODO  (Nivel 1) habilitar el uso de []
    // vector[3] = 8;
    // TODO  (Nivel 2) habilitar que el vector pueda ser escrito con cout <<
    // cout << vector << endl;
}