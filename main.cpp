#include <iostream>
#include "vector.h"
#include "DemoVector.h"
#include "hilos.h"
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

int main(){
    cout << "Hello Alg y EDA-UNI" << endl;
    DemoVector();
    return 0;
}

// int main(int nArgs, char *pArgs[]){
//     cout << "Hello Alg y EDA-UNI (forma #2)" << endl;
//     int i;
//     for(i = 0 ; i < nArgs ; ++i){
//         cout << pArgs[i] << endl;
//     }
// }



