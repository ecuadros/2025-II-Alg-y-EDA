#include <iostream>
#include <cstring>
#include <fstream>
#include <algorithm>
#include <thread>
#include <vector>
#include "btree.h"

using DemoTrait = BTreeTrait<char, long>;
using DemoTree  = BTree<DemoTrait>;

using ConcTrait = BTreeTrait<int, int>;
using ConcTree  = BTree<ConcTrait>;

void ThreadWorker(ConcTree &bt, int start_key, int count, int thread_id) {
    // 1. Inserciones (operaciones de Escritura)
    for (int i = 0; i < count; ++i) {
        int key = start_key + i;
        if (!bt.Insert(key, thread_id)) {
            // Esto debería ocurrir si m_Unique fuera true y la clave ya existiera.
        }
    }
    
    // 2. Búsquedas (operaciones de Lectura)
    int found_count = 0;
    for (int i = 0; i < count; ++i) {
        int key = start_key + i;
        if (bt.Search(key) != -1) {
            found_count++;
        }
    }
    std::cout << "  [Thread " << thread_id << "] Insertó y buscó " << count 
              << " claves. Encontradas: " << found_count << '\n';
}

void TestConcurrency() {
    std::cout << "\n--- 🧪 Test de Concurrencia (Seguridad de Hilos) ---\n";
    
    // Parámetros de la prueba
    const size_t NumThreads = 4;
    const int KeysPerThread = 50;
    const int TotalKeys = NumThreads * KeysPerThread;
    
    // Creamos el árbol con un orden bajo (3) y unicidad
    ConcTree concurrent_bt(3, true); 
    
    std::vector<std::thread> threads;
    std::cout << "Lanzando " << NumThreads << " hilos para insertar " << TotalKeys << " claves...\n";

    // Creamos y lanzamos los hilos
    for (size_t i = 0; i < NumThreads; ++i) {
        int start_key = i * KeysPerThread; // 0, 50, 100, 150...
        threads.emplace_back(ThreadWorker, std::ref(concurrent_bt), start_key, KeysPerThread, (int)i + 1);
    }
    
    // Esperamos a que todos los hilos terminen
    for (auto &t : threads) {
        t.join();
    }
    
    std::cout << "\nTodos los hilos terminaron.\n";
    std::cout << "Resultado final:\n";
    std::cout << "  Claves insertadas (esperado): " << TotalKeys << '\n';
    std::cout << "  Tamaño reportado por el árbol: " << concurrent_bt.size() << '\n';
    std::cout << "  Altura del árbol: " << concurrent_bt.height() << '\n';

    // Verificación final del árbol
    bool integrity_ok = true;
    for (int key = 0; key < TotalKeys; ++key) {
        if (concurrent_bt.Search(key) == -1) {
            std::cerr << "Fallo de integridad! Clave " << key << " no encontrada.\n";
            integrity_ok = false;
            break;
        }
    }

    if (integrity_ok && concurrent_bt.size() == TotalKeys) {
        std::cout << "El árbol pasó la prueba de integridad concurrente.\n";
    } else {
        std::cout << "El árbol falló la prueba de integridad o el conteo de claves.\n";
    }

    std::cout << "---------------------------------------------------------\n";
}




void DemoOperations(DemoTree &bt)
{
    const char *keys = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
    size_t n = std::strlen(keys);

    std::cout << "Insert (Demo: insertando " << n << " claves...)\n";
    for(size_t i = 0; i < n; ++i) {
        bt.Insert(keys[i], static_cast<long>(i * i));
    }

    std::cout << "\nArbol actual (Print):\n";
    bt.Print(std::cout);
    std::cout << "\n";

    const char probes[] = {'D','z','0','Q','!','A', keys[n/2], '\0'};
    std::cout << "(Searching) Busquedas de prueba:\n";
    for(size_t i = 0; probes[i]; ++i) {
        long id = bt.Search(probes[i]);
        if (id != -1)
            std::cout << "  Encontrado '" << probes[i] << "' -> ObjID = " << id << '\n';
        else
            std::cout << "  No encontrado '" << probes[i] << "\n";
    }
    std::cout << '\n';

    std::cout << "Delete (Demo: Eliminando claves en posiciones pares (ejemplo)...)\n";
    for(size_t i = 0; i < n; i += 2) {
        bt.Remove(keys[i], -1);
    }

    std::cout << "\nArbol después de eliminaciones:\n";
    bt.Print(std::cout);
    std::cout << "\nTamaño reportado: " << bt.size() << "  Altura reportada: " << bt.height() << "\n";
}


// void TestMove() {
//     std::cout << "\n--- Test para el move constructor ---\n";

//     // Creamos una página A (nodo de árbol B)
//     DemoTree::BTNode pageA(3, true); // (order, unique)
//     pageA.Insert('K', 10);
//     pageA.Insert('M', 20);
//     std::cout << "Estado Inicial (A): Claves=" << pageA.GetKeyCount() << "\n";

//     // Mover A a B (deberia de llamar al move constructor del btreepage)
//     std::cout << "Accion: pageB = std::move(pageA); \n";
//     DemoTree::BTNode pageB = std::move(pageA); // <-- deberia imprimir lo del move constructor

//     std::cout << "Estado Final (B): Claves=" << pageB.GetKeyCount() << "\n";
//     // pageA debería estar en estado 'movido' (válido pero sin recursos)
//     std::cout << "Estado Final (A): Claves=" << pageA.GetKeyCount() << "\n";
    
//     // Mover C a B (Llamará al OPERADOR DE ASIGNACIÓN DE MOVIMIENTO)
//     DemoTree::BTNode pageC(3, true);
//     pageC.Insert('Z', 30);
//     pageC.Insert('A', 40);
//     std::cout << "Estado Inicial (C): Claves=" << pageC.GetKeyCount() << "\n";

//     std::cout << "Accion: pageB = std::move(pageC); \n";
//     pageB = std::move(pageC); // <-- Debería imprimir OPERADOR DE ASIGNACIÓN DE MOVIMIENTO

//     std::cout << "Estado Final (B): Claves=" << pageB.GetKeyCount() << "\n";
//     std::cout << "Estado Final (C): Claves=" << pageC.GetKeyCount() << "\n";

// }

// Función para probar FirstThat
void TestFirstThat(DemoTree &bt)
{
    std::cout << "\n--- Test para FirstThat ---\n";
    std::cout << "Buscando la PRIMERA clave que sea una letra minúscula...\n";

    auto esMinuscula = [](DemoTree::ObjectInfo &info, size_t level) -> bool {
        return info.key >= 'a' && info.key <= 'z';
    };

    // Llamamos a FirstThat con nuestra lambda
    DemoTree::ObjectInfo* pEncontrado = bt.FirstThat(esMinuscula);

    if (pEncontrado) {
        std::cout << "  Encontrado: Clave='" << pEncontrado->key 
                  << "', ObjID=" << pEncontrado->ObjID << "\n";
    } else {
        std::cout << "  No se encontró ninguna clave minúscula.\n";
    }

    std::cout << "Buscando la PRIMERA clave que sea un '9'...\n";
    DemoTree::ObjectInfo* pNueve = bt.FirstThat([](auto& info, auto level){
        return info.key == '9';
    });

    if (pNueve) {
        std::cout << "  Encontrado: Clave='" << pNueve->key 
                  << "', ObjID=" << pNueve->ObjID << "\n";
    } else {
        std::cout << "  No se encontró la clave '9'.\n";
    }
}

void TestWriteRead(){
    BTree<BTreeTrait<int, long>> tree1(3, true);
    tree1.Insert(10, 100);
    tree1.Insert(20, 200);
    tree1.Insert(5, 50);
    
    std::cout << "Árbol original:\n";
    tree1.Print(std::cout);
    
    // Guardar
    std::ofstream ofs("test_tree.txt");
    tree1.Write(ofs);
    ofs.close();
    
    // Cargar
    BTree<BTreeTrait<int, long>> tree2(3, true);
    std::ifstream ifs("test_tree.txt");
    tree2.Read(ifs);
    ifs.close();
    
    std::cout << "\nÁrbol cargado:\n";
    tree2.Print(std::cout);


    BTree<BTreeTrait<int, long>> tree3(3, true);
    std::ifstream ifs("BT.txt");
    tree3.ReadBinaryTreeFormat(ifs);  
    ifs.close();

    std::cout << "\nArbol  cargado (formato del archivo BT.txt):\n";
    tree3.Print(std::cout);

    //probemos si el operador << funciona
    std::cout << "\nUsando operador << sobrecargado:\n";
    std::cout << tree3;
}

int main()
{
    DemoTree bt(3);
    DemoOperations(bt);
    // TestMove();
    TestFirstThat(bt);
    TestWriteRead();

    TestConcurrency();
    return 0;
}