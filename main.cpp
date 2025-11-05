#include <iostream>
#include "vector.h"
#include "DemoVector.h"
#include "hilos.h"
#include "linkedlist.h"
#include "doublelinkedlist.h"
#include "btree.h"  // ← SOLO AGREGAR ESTA LÍNEA

using namespace std;

/* Revisado por:
   1. Ernesto Cuadros-Vargas
   4. Héctor Bobbio Hermoza 
   2. Jharvy Jonas Cadillo Tarazona
   20. Ortiz Lozano Eric Hernan
   22. Chandler Steven Perez Cueva
*/

// Forma 1 de Compilar: 
// g++ -std=c++17 -Wall -g -pthread -o main main.cpp
// Forma #2 de Compilar (requiere el archivo Makefile)
// make

// ========== AGREGAR ESTA FUNCIÓN NUEVA ==========
void DemoBTree(){
    cout << "\n=== DEMO BTREE ===" << endl;
    
    // Definir el trait para char como clave y long como ObjID
    using CharTrait = BTreeTrait<char, long>;
    
    // Crear un BTree de orden 3
    const int BTreeSize = 3;
    BTree<CharTrait> bt(BTreeSize);
    
    // Cadenas de prueba
    const char * keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
    
    // Inserción
    cout << "\n--- Insertando elementos ---" << endl;
    for (int i = 0; keys1[i] && i < 20; i++) {
        bt.Insert(keys1[i], i * i);
        cout << "Insertado: " << keys1[i] << endl;
    }
    
    cout << "\n--- Estado del árbol ---" << endl;
    bt.Print(cout);
    cout << "Tamaño: " << bt.size() << endl;
    cout << "Altura: " << bt.height() << endl;
    
    // Búsqueda
    cout << "\n--- Buscando 'D' ---" << endl;
    long result = bt.Search('D');
    if (result != -1) {
        cout << "Encontrado! ID = " << result << endl;
    }
    
    cout << "\n=== FIN DEMO BTREE ===" << endl;
}
// ========== FIN DE LA FUNCIÓN NUEVA ==========

int main(){
    cout << "Hello Alg y EDA-UNI" << endl;
    
    // TUS DEMOS ORIGINALES (no los borres)
    // DemoThreads();
    [[maybe_unused]] int x = 5;
    // DemoVector();
    // DemoLinkedList();
    // DemoDoubleLinkedList();
    
    // ← SOLO AGREGAR ESTA LLAMADA
    DemoBTree();
    
    return 0;
}

// int main(int nArgs, char *pArgs[]){
//     cout << "Hello Alg y EDA-UNI (forma #2)" << endl;
//     int i;
//     for(i = 0 ; i < nArgs ; ++i){
//         cout << pArgs[i] << endl;
//     }
// }