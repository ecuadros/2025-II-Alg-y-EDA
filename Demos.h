#ifndef __DEMOS_H__
#define __DEMOS_H__

#include <iostream>
#include <functional>
#include "BTree.h"


inline void TestFeatures()
{
    std::cout << "--------------------------------------" << std::endl;
    std::cout << "     INICIANDO PRUEBAS DEL B-TREE     " << std::endl;
    std::cout << "--------------------------------------" << std::endl;

    // Usar Trait comun
    using TestTrait = BTreeTrait<int, long>;
    BTree<TestTrait> testTree;

    // Inserción
    // Prueba Insert() (que incluye insert_at()) 
    int keysToInsert[] = {
        10, 20, 5, 15, 25, 30, 1, 8, 12, 18, 
        22, 28, 3, 7, 11, 14, 17, 19, 21, 23
    };
    for (int key : keysToInsert)
    {
        testTree.Insert(key, key * 100);
    }

    // Print
    // Prueba Print() (que incluye ForEach())
    std::cout << "\n--- PRINT ---" << std::endl;
     testTree.Print(std::cout);
    std::cout << "------------------" << std::endl;

    // Iterador
    // Prueba iterador con for (auto& item : tree)
    std::cout << "\n--- ITERADOR ---" << std::endl;
    std::cout << "  [ ";
    for (auto& item :  testTree) // Esto usa begin(), end(), ++, y *
    {
        std::cout << item.key << " ";
    }
    std::cout << "]\n" << std::endl;
    std::cout << "(Esperado: 1 3 5 7 8 10 11 12 14 15 17 18 19 20 21 22 23 25 28 30 )" << std::endl;
    std::cout << "------------------" << std::endl;

    // Search
    // Prueba Search()
    std::cout << "\n--- SEARCH ---" << std::endl;
    long valueFound =  testTree.Search(17);
    std::cout << "Buscando clave 17. Resultado: " << valueFound 
              << " (Esperado: 1700)" << std::endl;
              
    valueFound =  testTree.Search(99);
    std::cout << "Buscando clave 99. Resultado: " << valueFound 
              << " (Esperado: -1)" << std::endl;
    std::cout << "------------------" << std::endl;

    // Comparador
    // Prueba un comparador predefinido

    using ReverseTrait = BTreeTrait<int, long, std::greater<int>>;
    BTree<ReverseTrait> reverseTestTree;
    
    for (int key : keysToInsert) {
        reverseTestTree.Insert(key, key * 100);
    }

    std::cout << "\n--- COMPARADOR (Inverso) ---" << std::endl;
    std::cout << "Estructura del árbol (Inverso):" << std::endl;
    reverseTestTree.Print(std::cout);
    std::cout << "Esperado: 30, 28, 25, 23, 22, 21, 20, 19, 18, 17, 15, 14, 12, 11, 10, 8, 7, 5, 3, 1" << std::endl;
    std::cout << "------------------" << std::endl;
}   


#endif // __DEMOS_H__