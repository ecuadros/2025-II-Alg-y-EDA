#include <iostream>
#include <functional>
#include <cmath>
#include <cstring>
#include "btree.h"

using namespace std;

struct CompareAbsolute {
    bool operator()(int a, int b) const {
        return abs(a) < abs(b);
    }
};

struct CaseInsensitive {
    bool operator()(const string& a, const string& b) const {
        return strcasecmp(a.c_str(), b.c_str()) < 0;
    }
};

struct Persona {
    string nombre;
    int edad;

    Persona() : nombre(""), edad(0) {}
    Persona(string n, int e) : nombre(n), edad(e) {}

    operator string() const { return nombre; }
};

struct CompararPorEdad {
    bool operator()(const Persona& a, const Persona& b) const {
        return a.edad < b.edad;
    }
};

template <typename keyType, typename ObjIDType>
void PrintPersona(tagObjectInfo<keyType, ObjIDType> &info, size_t level, void *pExtra)
{
    ostream &os = *(ostream *)pExtra;
    for(size_t i = 0; i < level ; i++)
        os << "\t";
    Persona* p = (Persona*)&info.key;
    os << p->nombre << " (edad: " << p->edad << ") -> " << info.ObjID << "\n";
}

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

    cout << "\n\n=== Demo BTree con Comparador Descendente ===" << endl << endl;

    using DescTrait = BTreeTrait<int, long, greater<int>>;
    BTree<DescTrait> btree_desc(3, true);

    cout << "1. Insertando elementos (orden descendente)..." << endl;
    int desc_values[] = {50, 20, 70, 10, 30, 60, 80, 5, 15};
    for (int val : desc_values) {
        cout << "   Insertando: " << val << endl;
        btree_desc.Insert(val, val * 10);
    }

    cout << "\n2. Arbol resultante (orden descendente):" << endl;
    btree_desc.Print(cout);

    cout << "\n3. Informacion del arbol:" << endl;
    cout << "   Tamano: " << btree_desc.size() << endl;
    cout << "   Altura: " << btree_desc.height() << endl;

    cout << "\n4. Buscando elementos:" << endl;
    int search_desc[] = {70, 15, 100};
    for (int key : search_desc) {
        long result = btree_desc.Search(key);
        if (result != -1) {
            cout << "   Clave " << key << " encontrada con ObjID: " << result << endl;
        } else {
            cout << "   Clave " << key << " NO encontrada" << endl;
        }
    }

    cout << "\n\n=== Demo BTree con Comparador por Valor Absoluto ===" << endl << endl;

    using AbsTrait = BTreeTrait<int, long, CompareAbsolute>;
    BTree<AbsTrait> btree_abs(3, true);

    cout << "1. Insertando numeros positivos y negativos..." << endl;
    int abs_values[] = {-15, 5, -25, 10, -8, 30, 3, -20, 12};
    for (int val : abs_values) {
        cout << "   Insertando: " << val << endl;
        btree_abs.Insert(val, abs(val) * 100);
    }

    cout << "\n2. Arbol resultante (ordenado por valor absoluto):" << endl;
    btree_abs.Print(cout);

    cout << "\n3. Informacion del arbol:" << endl;
    cout << "   Tamano: " << btree_abs.size() << endl;
    cout << "   Altura: " << btree_abs.height() << endl;
    cout << "   Nota: Los numeros estan ordenados por |valor|" << endl;

    cout << "\n\n=== Demo BTree con Strings Case-Insensitive ===" << endl << endl;

    using StrTrait = BTreeTrait<string, long, CaseInsensitive>;
    BTree<StrTrait> btree_str(3, true);

    cout << "1. Insertando strings con diferentes casos..." << endl;
    string str_values[] = {"manzana", "BANANA", "Cereza", "durazno", "FRESA", "Guayaba", "kiwi"};
    for (int i = 0; i < 7; i++) {
        cout << "   Insertando: " << str_values[i] << endl;
        btree_str.Insert(str_values[i], i * 10);
    }

    cout << "\n2. Arbol resultante (case-insensitive):" << endl;
    btree_str.Print(cout);

    cout << "\n3. Informacion del arbol:" << endl;
    cout << "   Tamano: " << btree_str.size() << endl;
    cout << "   Altura: " << btree_str.height() << endl;

    cout << "\n4. Buscando (case-insensitive):" << endl;
    string search_str[] = {"MANZANA", "banana", "Kiwi", "naranja"};
    for (const string& key : search_str) {
        long result = btree_str.Search(key);
        if (result != -1) {
            cout << "   \"" << key << "\" encontrada con ObjID: " << result << endl;
        } else {
            cout << "   \"" << key << "\" NO encontrada" << endl;
        }
    }

    cout << "\n\n=== Demo BTree con Objetos Personalizados (Persona) ===" << endl << endl;

    using PersonaTrait = BTreeTrait<Persona, long, CompararPorEdad>;
    BTree<PersonaTrait> btree_personas(3, true);

    cout << "1. Insertando personas (ordenadas por edad)..." << endl;
    Persona personas[] = {
        Persona("Ana", 25),
        Persona("Carlos", 18),
        Persona("Elena", 35),
        Persona("David", 22),
        Persona("Beatriz", 30),
        Persona("Francisco", 28),
        Persona("Gloria", 20)
    };

    for (int i = 0; i < 7; i++) {
        cout << "   Insertando: " << personas[i].nombre << " (edad: " << personas[i].edad << ")" << endl;
        btree_personas.Insert(personas[i], i * 100);
    }

    cout << "\n2. Arbol resultante (ordenado por edad):" << endl;
    btree_personas.ForEach(&PrintPersona<Persona, long>, &cout);

    cout << "\n3. Informacion del arbol:" << endl;
    cout << "   Tamano: " << btree_personas.size() << endl;
    cout << "   Altura: " << btree_personas.height() << endl;

    cout << "\n4. Buscando personas:" << endl;
    Persona buscar[] = {Persona("Test", 22), Persona("Test", 30), Persona("Test", 100)};
    for (const Persona& p : buscar) {
        long result = btree_personas.Search(p);
        if (result != -1) {
            cout << "   Persona con edad " << p.edad << " encontrada con ObjID: " << result << endl;
        } else {
            cout << "   Persona con edad " << p.edad << " NO encontrada" << endl;
        }
    }

    cout << "\n\n=== Demo de Iterators ===" << endl << endl;

    using ItTrait = BTreeTrait<int, long>;
    BTree<ItTrait> btree_it(3, true);

    cout << "1. Insertando elementos para demostrar iterators..." << endl;
    int it_values[] = {15, 10, 20, 5, 12, 18, 25, 3, 7, 14, 22, 30};
    for (int val : it_values) {
        btree_it.Insert(val, val * 10);
    }

    cout << "\n2. Iteracion forward con iterator:" << endl;
    cout << "   ";
    for (auto it = btree_it.begin(); it != btree_it.end(); ++it) {
        cout << it->key << " ";
    }
    cout << endl;

    cout << "\n3. Range-based for loop:" << endl;
    cout << "   ";
    for (const auto& item : btree_it) {
        cout << item.key << " ";
    }
    cout << endl;

    cout << "\n4. Buscando elemento con clave 18 (manual):" << endl;
    bool found = false;
    int found_objid = -1;
    for (auto it = btree_it.begin(); it != btree_it.end(); ++it) {
        if (it->key == 18) {
            found = true;
            found_objid = it->ObjID;
            break;
        }
    }
    if (found) {
        cout << "   Encontrado: 18 -> " << found_objid << endl;
    }

    cout << "\n5. Contando elementos mayores a 15:" << endl;
    int count = 0;
    for (auto it = btree_it.begin(); it != btree_it.end(); ++it) {
        if (it->key > 15) count++;
    }
    cout << "   Elementos mayores a 15: " << count << endl;

    cout << "\n6. Suma de todas las claves:" << endl;
    int sum = 0;
    for (auto it = btree_it.begin(); it != btree_it.end(); ++it) {
        sum += it->key;
    }
    cout << "   Suma de todas las claves: " << sum << endl;

    cout << "\n7. Verificando si todos son positivos:" << endl;
    bool all_positive = true;
    for (auto it = btree_it.begin(); it != btree_it.end(); ++it) {
        if (it->key <= 0) {
            all_positive = false;
            break;
        }
    }
    cout << "   Todos los elementos son positivos: " << (all_positive ? "Si" : "No") << endl;

    cout << "\n8. Mostrando claves x2:" << endl;
    cout << "   Claves x2: ";
    for (auto it = btree_it.begin(); it != btree_it.end(); ++it) {
        cout << (it->key * 2) << " ";
    }
    cout << endl;

    cout << "\n9. Iteracion BACKWARD con reverse_iterator:" << endl;
    cout << "   ";
    for (auto it = btree_it.rbegin(); it != btree_it.rend(); ++it) {
        cout << it->key << " ";
    }
    cout << endl;

    cout << "\n=== Todos los demos completados ===" << endl;

    return 0;
}
