#include <iostream>
#include <cstring>
#include "btree.h"

using DemoTrait = BTreeTrait<char, long>;
using DemoTree  = BTree<DemoTrait>;


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

int main()
{
    DemoTree bt(3);
    DemoOperations(bt);
    return 0;
}