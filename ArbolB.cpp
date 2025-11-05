// ArbolB.cpp
// Programa independiente para probar el BTree
// Compilar: g++ -std=c++17 -Wall -g -pthread -o arbolb ArbolB.cpp

#include <iostream>
#include <time.h>
#include <stdlib.h>
#include "btree.h"

using namespace std;

// Cadenas de prueba del archivo antiguo tstbtree.cc
const char * keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
const char * keys2 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const char * keys3 = "DYZakHIUwxVJ203ejOP9Qc8AdtuEop1XvTRghSNbW567BfiCqrs4FGMyz";

const int BTreeSize = 3;

int main(int argc, char * argv[]){
    cout << "=== DEMO BTREE - MIGRADO DE tstbtree.cc ===" << endl;
    
    // Definir el trait
    using CharTrait = BTreeTrait<char, long>;
    
    // Crear el BTree
    BTree<CharTrait> bt(BTreeSize);
    
    // FASE 1: INSERCIÓN (igual que en tstbtree.cc)
    cout << "\n--- INSERTANDO ELEMENTOS ---" << endl;
    for (int i = 0; keys1[i]; i++) {
        cout << "Insertando " << keys1[i] << endl;
        bool result = bt.Insert(keys1[i], i*i);
        if (!result) {
            cout << "Error: clave duplicada o fallo en inserción" << endl;
        }
    }
    
    cout << "\n--- ARBOL COMPLETO ---" << endl;
    bt.Print(cout);
    cout << "\nEstadísticas:" << endl;
    cout << "  Tamaño: " << bt.size() << endl;
    cout << "  Altura: " << bt.height() << endl;
    cout << "  Orden: " << bt.GetOrder() << endl;
    
    // FASE 2: BÚSQUEDA (descomentar para probar)
    cout << "\n--- BUSCANDO ELEMENTOS ---" << endl;
    for (int i = 0; keys2[i]; i++) {
        cout << "Buscando " << keys2[i] << " ";
        long ObjID = bt.Search(keys2[i]);
        if (ObjID != -1) {
            cout << "Encontrado! ID = " << ObjID << endl;
        } else {
            cout << "No encontrado!" << endl;
        }
    }
    cout.flush();
    
    // FASE 3: ELIMINACIÓN (descomentar para probar)
    cout << "\n--- ELIMINANDO ELEMENTOS ---" << endl;
    for (int i = 0; keys3[i]; i++) {
        cout << "Eliminando " << keys3[i] << " ";
        if (bt.Remove(keys3[i], -1)) {
            cout << keys3[i] << " eliminado!" << endl;
        } else {
            cout << "No encontrado!" << endl;
        }
        bt.Print(cout);
    }
    bt.Print(cout);
    cout.flush();
    
    return 0;
}

/* CÓDIGO ANTIGUO COMENTADO PARA REFERENCIA
 * Puedes descomentar y adaptar según necesites
 
// Ejemplo con enteros grandes
const int BTreeSize = 3;
int main_enteros(int argc, char * argv) {
    using IntTrait = BTreeTrait<long long, long>;
    BTree<IntTrait> bt(BTreeSize);
    
    for (int i = 0; i < 1000000; i++) {
        bt.Insert(i, i-1);
    }

    for (int i = 0; i < 1000; i++) {
        long long key = 975000 + (::rand() % 50000);
        long ObjID = bt.Search(key);
        if (ObjID != -1) {
            cout << "Encontrado " << key << " ID = " << ObjID << endl;
        } else {
            cout << "No encontrado " << key << endl;
        }
    }
    cout.flush();
    return 1;
}
*/