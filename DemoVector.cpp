#include <iostream>
#include "DemoVector.h"
#include "vector.h"

using namespace std;

void DemoVector(){
    int valueToInsert = 5;

    CVector<int> vector(1);

    vector.insert(valueToInsert);

    // TODO  (Nivel 1) habilitar el uso de []

    vector[0] = 7;
    vector.insert(valueToInsert);

    // TODO  (Nivel 2) habilitar que el vector pueda ser escrito con cout <<
    cout << "Vector inicial: " << vector << endl;

    const CVector<int> vector2 = vector;

    cout <<"Vector 2 copia del 1: " << vector2 << endl;

    const CVector<int> vector3 = std::move(vector);

    cout << "Vector 3 movido del 1:  " <<  vector3 << endl;
    cout << "Vector 1 muerto: " << vector << endl;

    // Crear un vector con más elementos para probar iteradores
    CVector<int> iteratorDemo(5);
    for (int i = 1; i <= 5; ++i) {
        iteratorDemo.insert(i * 10);
    }

    cout << "\n=== Demostracion de Iteradores ===" << endl;
    cout << "Vector completo: " << iteratorDemo << endl;

    // 1. Iterator (forward)
    cout << "\n1. Forward iterator (begin/end):" << endl;
    cout << "   ";
    for (auto it = iteratorDemo.begin(); it != iteratorDemo.end(); ++it) {
        cout << *it << " ";
    }
    cout << endl;

    // 2. Const iterator (forward)
    cout << "\n2. Const iterator (begin/end const):" << endl;
    cout << "   ";
    const CVector<int>& constRef = iteratorDemo;
    for (auto it = constRef.begin(); it != constRef.end(); ++it) {
        cout << *it << " ";
    }
    cout << endl;

    // 3. Const iterator explicit (cbegin/cend)
    cout << "\n3. Const iterator explicit (cbegin/cend):" << endl;
    cout << "   ";
    for (auto it = iteratorDemo.cbegin(); it != iteratorDemo.cend(); ++it) {
        cout << *it << " ";
    }
    cout << endl;

    // 4. Range-based for loop (uses begin/end)
    cout << "\n4. Range-based for loop:" << endl;
    cout << "   ";
    for (const auto& value : iteratorDemo) {
        cout << value << " ";
    }
    cout << endl;

    // 5. Reverse iterator (backward)
    cout << "\n5. Reverse iterator (rbegin/rend):" << endl;
    cout << "   ";
    for (auto it = iteratorDemo.rbegin(); it != iteratorDemo.rend(); ++it) {
        cout << *it << " ";
    }
    cout << endl;

    // 6. Const reverse iterator (backward)
    cout << "\n6. Const reverse iterator (rbegin/rend const):" << endl;
    cout << "   ";
    for (auto it = constRef.rbegin(); it != constRef.rend(); ++it) {
        cout << *it << " ";
    }
    cout << endl;

    // 7. Const reverse iterator explicit (crbegin/crend)
    cout << "\n7. Const reverse iterator explicit (crbegin/crend):" << endl;
    cout << "   ";
    for (auto it = iteratorDemo.crbegin(); it != iteratorDemo.crend(); ++it) {
        cout << *it << " ";
    }
    cout << endl;

    // 8. Manual reverse iteration using reverse iterator
    cout << "\n8. Manual reverse iteration:" << endl;
    cout << "   ";
    auto rit = iteratorDemo.rbegin();
    while (rit != iteratorDemo.rend()) {
        cout << *rit << " ";
        ++rit;
    }
    cout << endl;

    cout << "\n=== Fin de demostracion ===" << endl;
}