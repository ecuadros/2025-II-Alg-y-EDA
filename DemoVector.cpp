#include <iostream>
#include "DemoVector.h"
#include "vector.h"
using namespace std;

void DemoVector(){
    // CVector vector(10);
    // vector.insert(5);
    // TODO  (Nivel 1) (listo) habilitar el uso de []
    // vector[3] = 8;
    // TODO  (Nivel 2) (listo) habilitar que el vector pueda ser escrito con cout <<
    // cout << vector << endl;
    cout << "=== Probando constructores de CVector ===" << endl;

    cout << "1. Constructor con tamaño:" << endl;
    CVector<int> v1(5);
    cout << "   Vector v1 creado con tamaño 5" << endl;

    cout << "2. Constructor por copia:" << endl;
    CVector<int> v2(v1);
    cout << "   Vector v2 creado como copia de v1" << endl;

    cout << "3. Move constructor:" << endl;
    CVector<int> v3(std::move(v2));
    cout << "   Vector v3 creado con move de v2" << endl;

    cout << "4. Probando resize:" << endl;
    CVector<int> v4(2);
    cout << "   Vector v4 creado con tamaño 2" << endl;
    v4.resize();
    cout << "   v4.resize() - redimensionado con delta por defecto" << endl;
    v4.resize(3.0);
    cout << "   v4.resize(3.0) - redimensionado con delta personalizado" << endl;

    cout << "5. Probando operator<< e insert:" << endl;
    CVector<int> v5(2);
    cout << "   Vector vacío: " << v5 << endl;

    int val1 = 10, val2 = 20, val3 = 30;
    v5.insert(val1);
    cout << "   Después de insert(10): " << v5 << endl;
    v5.insert(val2);
    cout << "   Después de insert(20): " << v5 << endl;
    v5.insert(val3);
    cout << "   Después de insert(30): " << v5 << endl;

    cout << "6. Probando operator[]:" << endl;
    cout << "   v5[0] = " << v5[0] << endl;
    cout << "   v5[1] = " << v5[1] << endl;
    cout << "   v5[2] = " << v5[2] << endl;

    v5[1] = 999;
    cout << "   Después de v5[1] = 999: " << v5 << endl;

    cout << "=== Fin de pruebas ===" << endl;
}