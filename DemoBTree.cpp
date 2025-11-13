#include <iostream>
#include <functional>
#include "btree.h"

//---------------------------------------------------------
// Trait personalizado para BTree<int,long>
struct IntLongTrait {
    using keyType = int;
    using ObjIDType = long;
    using CompareFn = std::greater<keyType>;
};
//---------------------------------------------------------

// Función de impresión jerárquica para cada nodo
void PrintInfo(tagObjectInfo<int, long> &info, size_t level, void *pExtra) {
    std::ostream &os = *(std::ostream *)pExtra;
    for (size_t i = 0; i < level; i++)
        os << "   ";
    os << "- (" << info.key << ", " << info.ObjID << ")\n";
}

int DemoBTree() {
    using namespace std;

    // Crear BTree con orden 3 (máx 3 claves por nodo)
    BTree<IntLongTrait> tree(3, true);

    cout << "Insersion de elementos" << endl;
    int claves[] = {50, 20, 70, 10, 30, 60, 80, 25, 35, 55, 65};
    for (int i = 0; i < 11; i++) {
        bool ok = tree.Insert(claves[i], i + 100);
        cout << "Insert(" << claves[i] << "): " << (ok ? "OK" : "Duplicado") << endl;
    }

    cout << "\nImpresion del arbol" << endl;
    tree.ForEach(PrintInfo, &cout);

    cout << "\nAltura del árbol: " << tree.height() << endl;
    cout << "Número de claves: " << tree.size() << endl;

    cout << "\nBusqueda" << endl;
    int buscar[] = {25, 70, 90};
    for (int k : buscar) {
        long id = tree.Search(k);
        if (id != -1)
            cout << "Encontrado " << k << " -> ObjID=" << id << endl;
        else
            cout << "Clave " << k << " no encontrada" << endl;
    }

    cout << "\nEliminacion de Claves" << endl;
    int borrar[] = {10, 20, 25, 50};
    for (int k : borrar) {
        bool ok = tree.Remove(k, 0);
        cout << "Remove(" << k << "): " << (ok ? "OK" : "No encontrado") << endl;
    }

    cout << "\nArbol luego de eliminaciones" << endl;
    tree.ForEach(PrintInfo, &cout);

    cout << "\nAltura actual: " << tree.height() << endl;
    cout << "Número de claves actual: " << tree.size() << endl;

    
    return 0;
}