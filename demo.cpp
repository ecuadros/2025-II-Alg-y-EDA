#include <iostream>
#include "btree.h"

using namespace std;

int main() {
    cout << "=== Demo BTree ===" << endl << endl;

    // Crear un BTree con orden 3
    using Trait = BTreeTrait<int, long>;
    BTree<Trait> btree(3, true);

    cout << "1. Insertando elementos..." << endl;
    int values[] = {10, 20, 5, 6, 12, 30, 7, 17, 3, 15, 25, 35, 40};
    for (int val : values) {
        cout << "   Insertando: " << val << endl;
        btree.Insert(val, val * 100);
    }

    cout << "\n2. Arbol resultante:" << endl;
    btree.Print(cout);

    cout << "\n3. Informacion del arbol:" << endl;
    cout << "   Tamano (numero de claves): " << btree.size() << endl;
    cout << "   Altura: " << btree.height() << endl;
    cout << "   Orden: " << btree.GetOrder() << endl;

    cout << "\n4. Buscando elementos:" << endl;
    int search_keys[] = {10, 15, 100, 7};
    for (int key : search_keys) {
        long result = btree.Search(key);
        if (result != -1) {
            cout << "   Clave " << key << " encontrada con ObjID: " << result << endl;
        } else {
            cout << "   Clave " << key << " NO encontrada" << endl;
        }
    }

    cout << "\n5. Eliminando elementos (3, 10, 25)..." << endl;
    int remove_keys[] = {3, 10, 25};
    for (int key : remove_keys) {
        bool removed = btree.Remove(key, key * 100);
        if (removed) {
            cout << "   Clave " << key << " eliminada exitosamente" << endl;
        } else {
            cout << "   No se pudo eliminar clave " << key << endl;
        }
    }

    cout << "\n6. Arbol despues de eliminaciones:" << endl;
    btree.Print(cout);

    cout << "\n7. Tamano final: " << btree.size() << endl;

    cout << "\n=== Demo completado ===" << endl;

    return 0;
}
