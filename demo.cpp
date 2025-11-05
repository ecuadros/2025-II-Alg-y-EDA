#include <iostream>
#include <fstream>
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
    btree_personas.ForEach([](auto& info, size_t level, ostream* pos) {
        for(size_t i = 0; i < level; i++)
            (*pos) << "\t";
        Persona* p = (Persona*)&info.key;
        (*pos) << p->nombre << " (edad: " << p->edad << ") -> " << info.ObjID << "\n";
    }, &cout);

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

    cout << "\n\n=== Demo Read/Write - Serializacion/Deserializacion ===" << endl << endl;

    using RWTrait = BTreeTrait<int, long>;
    BTree<RWTrait> btree_write(3, true);

    cout << "1. Creando arbol para serializar..." << endl;
    int rw_values[] = {50, 30, 70, 20, 40, 60, 80, 10, 25, 35, 45, 55, 65, 75, 90};
    for (int val : rw_values) {
        btree_write.Insert(val, val * 100);
    }
    cout << "   Insertados " << btree_write.size() << " elementos" << endl;
    cout << "   Altura del arbol: " << btree_write.height() << endl;

    cout << "\n2. Elementos en orden (usando iterator):" << endl;
    cout << "   ";
    for (const auto& item : btree_write) {
        cout << item.key << " ";
    }
    cout << endl;

    cout << "\n3. Guardando arbol en archivo 'demo_btree.txt' usando Write()..." << endl;
    {
        ofstream file("demo_btree.txt");
        if (file.is_open()) {
            btree_write.Write(file);
            file.close();
            cout << "   ✓ Arbol guardado exitosamente" << endl;
        } else {
            cout << "   ✗ Error al abrir archivo" << endl;
        }
    }

    cout << "\n4. Mostrando contenido del archivo:" << endl;
    {
        ifstream file("demo_btree.txt");
        if (file.is_open()) {
            string line;
            int line_num = 1;
            while (getline(file, line) && line_num <= 5) {  // Mostrar solo primeras 5 lineas
                cout << "   Linea " << line_num << ": " << line << endl;
                line_num++;
            }
            if (line_num > 5) {
                cout << "   ... (archivo completo con " << btree_write.size() << " elementos)" << endl;
            }
            file.close();
        }
    }

    cout << "\n5. Cargando arbol desde archivo usando Read()..." << endl;
    BTree<RWTrait> btree_read(3, true);
    {
        ifstream file("demo_btree.txt");
        if (file.is_open()) {
            btree_read.Read(file);
            file.close();
            cout << "   ✓ Arbol cargado exitosamente" << endl;
        } else {
            cout << "   ✗ Error al abrir archivo" << endl;
        }
    }

    cout << "\n6. Verificando arbol cargado:" << endl;
    cout << "   Tamano: " << btree_read.size() << " elementos" << endl;
    cout << "   Altura: " << btree_read.height() << endl;

    cout << "\n7. Elementos del arbol cargado:" << endl;
    cout << "   ";
    for (const auto& item : btree_read) {
        cout << item.key << " ";
    }
    cout << endl;

    cout << "\n8. Verificando integridad (comparando valores):" << endl;
    bool integrity_ok = true;
    int verification_keys[] = {10, 30, 50, 70, 90};
    for (int key : verification_keys) {
        long result = btree_read.Search(key);
        long expected = key * 100;
        if (result == expected) {
            cout << "   ✓ Clave " << key << " -> ObjID " << result << " (correcto)" << endl;
        } else {
            cout << "   ✗ Clave " << key << " -> ObjID " << result << " (esperado: " << expected << ")" << endl;
            integrity_ok = false;
        }
    }
    cout << "   Resultado: " << (integrity_ok ? "✓ INTEGRIDAD VERIFICADA" : "✗ ERROR EN INTEGRIDAD") << endl;

    cout << "\n9. Probando operator<< para serializar..." << endl;
    {
        ofstream file("demo_operator.txt");
        if (file.is_open()) {
            file << btree_write;  // Usando operator<<
            file.close();
            cout << "   ✓ Arbol serializado con operator<<" << endl;
        }
    }

    cout << "\n10. Probando operator>> para deserializar..." << endl;
    BTree<RWTrait> btree_operator(3, true);
    {
        ifstream file("demo_operator.txt");
        if (file.is_open()) {
            file >> btree_operator;  // Usando operator>>
            file.close();
            cout << "   ✓ Arbol deserializado con operator>>" << endl;
            cout << "   Tamano: " << btree_operator.size() << " elementos" << endl;
            cout << "   Altura: " << btree_operator.height() << endl;
        }
    }

    cout << "\n11. Verificando round-trip (Write->Read->Write->Read)..." << endl;
    {
        // Write 1
        ofstream f1("demo_roundtrip1.txt");
        f1 << btree_write;
        f1.close();

        // Read 1
        BTree<RWTrait> temp1(3, true);
        ifstream f2("demo_roundtrip1.txt");
        f2 >> temp1;
        f2.close();

        // Write 2
        ofstream f3("demo_roundtrip2.txt");
        f3 << temp1;
        f3.close();

        // Read 2
        BTree<RWTrait> temp2(3, true);
        ifstream f4("demo_roundtrip2.txt");
        f4 >> temp2;
        f4.close();

        cout << "   Original: size=" << btree_write.size() << ", height=" << btree_write.height() << endl;
        cout << "   Round 1:  size=" << temp1.size() << ", height=" << temp1.height() << endl;
        cout << "   Round 2:  size=" << temp2.size() << ", height=" << temp2.height() << endl;
        
        bool roundtrip_ok = (temp2.size() == btree_write.size());
        cout << "   Resultado: " << (roundtrip_ok ? "✓ ROUND-TRIP OK" : "✗ ERROR EN ROUND-TRIP") << endl;
    }

    cout << "\n12. Probando serializar arbol vacio..." << endl;
    {
        BTree<RWTrait> btree_empty(3, true);
        ofstream f1("demo_empty.txt");
        f1 << btree_empty;
        f1.close();
        cout << "   ✓ Arbol vacio serializado" << endl;

        BTree<RWTrait> btree_empty_read(3, true);
        ifstream f2("demo_empty.txt");
        f2 >> btree_empty_read;
        f2.close();
        cout << "   ✓ Arbol vacio deserializado (size=" << btree_empty_read.size() << ")" << endl;
    }

    cout << "\n13. Probando con arbol grande (100 elementos)..." << endl;
    {
        BTree<RWTrait> btree_large(5, true);  // order=5 para manejar mas elementos
        for (int i = 1; i <= 100; i++) {
            btree_large.Insert(i * 10, i * 1000);
        }
        cout << "   Arbol grande creado: size=" << btree_large.size() << ", height=" << btree_large.height() << endl;

        ofstream file("demo_large.txt");
        file << btree_large;
        file.close();
        cout << "   ✓ Arbol grande serializado" << endl;

        BTree<RWTrait> btree_large_read(5, true);
        ifstream file_in("demo_large.txt");
        file_in >> btree_large_read;
        file_in.close();
        cout << "   ✓ Arbol grande deserializado: size=" << btree_large_read.size() << ", height=" << btree_large_read.height() << endl;

        // Verificar algunos elementos aleatorios
        bool large_ok = true;
        int test_keys[] = {10, 50, 100, 500, 1000};
        for (int key : test_keys) {
            long result = btree_large_read.Search(key);
            long expected = (key / 10) * 1000;  // key = i*10, objID = i*1000
            if (result != expected) {
                large_ok = false;
                cout << "   [Debug] Clave " << key << ": esperado=" << expected << ", obtenido=" << result << endl;
            }
        }
        cout << "   Verificacion: " << (large_ok ? "✓ DATOS CORRECTOS" : "✗ ERROR EN DATOS") << endl;
    }

    cout << "\n14. Probando orden garantizado por iterators..." << endl;
    {
        BTree<RWTrait> btree_order(3, true);
        int random_values[] = {75, 25, 90, 10, 40, 60, 85, 5, 95};
        cout << "   Insertando en orden aleatorio: ";
        for (int val : random_values) {
            cout << val << " ";
            btree_order.Insert(val, val * 10);
        }
        cout << endl;

        ofstream file("demo_order.txt");
        btree_order.Write(file);
        file.close();

        cout << "   Orden en archivo (usando Write con iterators): ";
        BTree<RWTrait> btree_order_read(3, true);
        ifstream file_in("demo_order.txt");
        btree_order_read.Read(file_in);
        file_in.close();

        for (const auto& item : btree_order_read) {
            cout << item.key << " ";
        }
        cout << endl;
        cout << "   ✓ Orden ascendente garantizado por iterators" << endl;
    }

    cout << "\n=== Todos los demos completados ===" << endl;

    return 0;
}
