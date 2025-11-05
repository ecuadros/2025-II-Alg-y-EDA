#include <time.h>
#include <stdlib.h>
#include "btree.h"
#include <string>

const char * keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
const char * keys2 = "]*[¨{3456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz*";
const char * keys3 = "DYZakHIUwxVJ203ejOP9Qc8AdtuEop1XvTRghSNbW567BfiCqrs4FGMyzKLlmn";

const int BTreeSize = 3;
int main (int argc, char ** argv){
    int result, i;
    cout << "--- Probando BTree con orden ascendente por defecto ---" << endl;
    // BTree con orden ascendente (std::less por defecto)
    BTree<BTreeTrait<char, int>> bt(BTreeSize);
    cout << bt;
    for (i = 0; keys1[i]; ++i) {
        result = bt.Insert(keys1[i], i+1);
    }
    cout << bt;
       
    cout << "------------------------------------------\n" << endl;
    cout << "\n--- Probando la busqueda ---" << endl;
    for (i = 0; keys2[i]; ++i) {
        cout << "Buscando " << keys2[i] << ": ";
        int ObjID = bt.Search(keys2[i]);
        if (ObjID != -1)
            cout << "Encontrado " << keys2[i] << ", ID = " << ObjID << endl;
        else
            cout <<"No encontrado " << keys2[i] << endl;
    }
    cout << "------------------------------------------\n" << endl;
    cout << "\n--- Probando BTree con orden descendente antes de ser movido ---" << endl;
    // BTree con orden descendente (std::greater)
    BTree<BTreeTrait<char, int, std::greater<char>>> bt_desc(BTreeSize);
    for (i = 0; keys1[i]; i++) {
        result = bt_desc.Insert(keys1[i], i+1); 
    }
    cout << bt_desc;

    cout << "\n--- Moviendo el arbol bt_desc a bt_moved ---" << endl;
    BTree<BTreeTrait<char, int, std::greater<char>>> bt_moved(std::move(bt_desc));

    cout << "Arbol movido (bt_moved):" << endl;
    cout << bt_moved;

    cout << "Arbol original (bt_desc) despues de mover:" << endl;
    if (bt_desc.size() == 0) {
        cout << "(El arbol esta vacio)" << endl;
    } else {
        cout << bt_desc; // No deberia llegar aqui
    }

    cout << "------------------------------------------\n" << endl;
    cout << "\n--- Probando ForEach con funcion lambda ---" << endl;
    bt.ForEach(0, [](const auto& info, size_t level) {
        if (info.key > 'f') {
                std::cout << "Clave: " << info.key << " en nivel " << level << std::endl;
        }
    });
    cout << "\n--- Probando Write y Read ---" << endl;
    // Write
    cout << "Guardando el arbol 'bt' en 'btree.dat'..." << endl;
    ofstream outFile("btree.txt");
    if (outFile) {
        bt.Write(outFile);
        outFile.close();
        cout << "Guardado con exito" << endl;
    }

    // Read
    BTree<BTreeTrait<char, int>> bt_loaded(BTreeSize);
    cout << "Cargando desde 'btree.txt' a 'bt_loaded' " << endl;
    ifstream inFile("btree.txt");
    if (inFile) {
        bt_loaded.Read(inFile);
        inFile.close();
        cout << "Cargado con exito:" << endl;
        cout << bt_loaded;
        
    }
    cout << "------------------------------------------\n" << endl;
    cout << "\n--- Probando iteradores ---" << endl;
    
    string iterated_keys;
    cout << "Recorriendo el arbol 'bt' con iteradores..." << endl;
    for (auto it = bt.begin(); it != bt.end(); ++it) {
        cout << (*it).key;
    }
    cout << endl;
    cout << "------------------------------------------\n" << endl;

       return 1;
}