#ifndef __DEMOS_H__
#define __DEMOS_H__

#include <iostream>
#include <fstream>      
#include <functional>   
#include <utility>      
#include "btree.h"

void BTreeTest()
{
    // --- INSERCION ---
    std::cout << "Test #1: Inserción" << std::endl;

    using MyTestTrait = BTreeTrait<int, long, std::less<int>>;
    BTree<MyTestTrait> myTree;

    int keysToInsert[] = {
        10, 20, 5, 15, 25, 30, 1, 8, 12, 18, 
        22, 28, 3, 7, 11, 14, 17, 19, 21, 23
    };
    for (int key : keysToInsert)
    {
        myTree.Insert(key, key * 100);
    }
    std::cout << "Se insertaron " << myTree.size() << " elementos." << std::endl;
    std::cout << "Esperado: 20" << std::endl;

    // --- PRINT ---
    std::cout << "\nTest #2: Print()" << std::endl;
    myTree.Print(std::cout);

    // --- OPERADOR << ---
    std::cout << "\nTest #3: Operador <<" << std::endl;
    std::cout << "Salida de 'cout << myTree': " ;
    std::cout << myTree << std::endl;
    std::cout << "(Esperado: [1, 3, 5, 7, 8, 10, 11, 12, 14, 15, 17, 18, 19, 20, 21, 22, 23, 25, 28, 30])" << std::endl;

    // --- FIRST THAT ---
    std::cout << "\nTest #4: FirstThat()" << std::endl;
    auto* result = 
        myTree.FirstThat([](auto& info) {
            return info.key > 20; // Encontrar la primera clave > 20
        });
    if (result) {
        std::cout << "Primera clave encontrada que sea mayor que 20: " << result->key 
                  << " (Esperado: 21)" << std::endl;
    }

    // --- SEARCH ---
    std::cout << "\nTest #5: Search()" << std::endl;
    long valueFound = myTree.Search(17);
    std::cout << "Buscando clave 17... Resultado: " << valueFound 
              << " (Esperado: 1700)" << std::endl;
    valueFound = myTree.Search(99);
    std::cout << "Buscando clave 99... Resultado: " << valueFound 
              << " (Esperado: -1)" << std::endl;

    // --- REMOVE ---
    std::cout << "\nTest #6: Remove()" << std::endl;
    myTree.Remove(8, 800);
    std::cout << "Borrando clave 8... (size: " << myTree.size() << ")" << std::endl;
    myTree.Remove(20, 2000);
    std::cout << "Borrando clave 20... (size: " << myTree.size() << ")" << std::endl;
    std::cout << "Arbol post-borrado: " << myTree << std::endl;

    // --- MOVE CONSTRUCTOR ---
    std::cout << "\nTest #7: Move Constructor" << std::endl;
    std::cout << "Moviendo 'myTree' a 'myMovedTree'..." << std::endl;
    BTree<MyTestTrait> myMovedTree = std::move(myTree);

    std::cout << "Tamano de 'myTree': " << myTree.size() << " (Esperado: 0)" << std::endl;
    std::cout << "Tamano de 'myMovedTree' (con datos): " << myMovedTree.size() << " (Esperado: 18)" << std::endl;
    std::cout << "Contenido de 'myMovedTree': " << myMovedTree << std::endl;

    // --- READ & WRITE---
    std::cout << "\nTest #8: Read() y Write()" << std::endl;
    std::string filename = "arbol_demo.txt";
    myMovedTree.Write(filename);
    std::cout << "Arbol movido guardado en " << filename << std::endl;
    
    BTree<MyTestTrait> loadedTree;
    loadedTree.Read(filename);
    std::cout << "Arbol cargado desde " << filename << std::endl;
    
    std::cout << "Contenido del arbol cargado: " << loadedTree << std::endl;

    // --- FUNCION COMPARE ---
    std::cout << "\nTest #9: Función Compare (Orden Inverso)" << std::endl;
    using ReverseTrait = BTreeTrait<int, long, std::greater<int>>;
    BTree<ReverseTrait> reverseTree;
    for (int key : keysToInsert)
    {
        reverseTree.Insert(key, key * 100);
    }
    std::cout << "Contenido de 'reverseTree': " << reverseTree << std::endl;
    std::cout << "(Esperado: [30, 28, 25, 23, 22, 21, 20, 19, 18, 17, 15, 14, 12, 11, 10, 8, 7, 5, 3, 1])" << std::endl;

    // --- ITERADORES ---
    std::cout << "\nTest #10: Iteradores" << std::endl;
    std::cout << "Iterando desde begin() hasta end(): ";
    for (auto& item : reverseTree) {
        std::cout << item.key << " ";
    }
}

#endif // __DEMOS_H__