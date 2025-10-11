#include <iostream>
#include "vector.h"
#include "DemoVector.h"
#include "hilos.h"

using namespace std;
void DemoLinkedList();
void DemoDoubleLinkedList();
void DemoBinaryTree();
/* Revisado por:
   1. Ernesto Cuadros-Vargas
   4. Héctor Bobbio Hermoza 
   2. Jharvy Jonas Cadillo Tarazona
   20. Ortiz Lozano Eric Hernan
   22. Chandler Steven Perez Cueva
*/

int main(){
    cout << "Hello Alg y EDA-UNI" << endl;
    cout.flush();

    // Prueba DemoVector
    //DemoVector();

    // Prueba threads
    //DemoThreads();

    // Pausa para ver la salida en Windows
    //cout << "Presiona Enter para salir..." << endl;
    //cin.get();


    cout << "===== DEMO LINKED LIST =====" << endl;
    DemoLinkedList();

    cout << "\n===== DEMO DOUBLE LINKED LIST =====" << endl;
    DemoDoubleLinkedList();

    cout << "\n===== DEMO BINARY TREE =====" << endl;
    DemoBinaryTree();
    return 0;
}
