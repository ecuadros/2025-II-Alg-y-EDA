#include <iostream>
#include "btree.h"

using namespace std;
// Forma 1 de Compilar: 
// g++ -std=c++17 -Wall -g -pthread -o main main.cpp
// Forma #2 de Compilar (requiere el archivo Makefile)
// make
using Trait = BTreeTrait<int, long>;
static void Line() { std::cout << "--------------------------------------\n"; }


int main() {
   std::cout << "Demo B-tree\n";

    // Construcción del árbol
    BTree<Trait> t(/*order=*/3, /*unique=*/true);

    // Inserciones
    int keys[] = {40,70,10,20,30,50,60,80,90};
    long id = 1;
    for (int k : keys) t.Insert(k, id++);

    Line();
    std::cout << "size=" << t.size() << ", height=" << t.height() << "\n";
    t.Print(std::cout);

    Line();
    std::cout << "Search(60) -> " << t.Search(60) << "\n";
    std::cout << "Search(25) -> " << t.Search(25) << "\n";

    Line();
    std::cout << "Delete(10)\n";
    t.Remove(10, 0);
    std::cout << "size=" << t.size() << ", height=" << t.height() << "\n";
    t.Print(std::cout);

    // --- Ejemplo de ForEachT (opcional) ---
    // Recorre e imprime (key->ObjID) con indentación por nivel.
    Line();
    std::cout << "[ForEachT demo]\n";
    t.ForEachT([](BTree<Trait>::ObjectInfo& info, size_t lvl, std::ostream* pos){
        auto& os = *pos;
        for (size_t i = 0; i < lvl; ++i) os << '\t';
        os << info.key << "->" << info.ObjID << "\n";
    }, &std::cout);

    // --- Guardar a archivo ---
    Line();
    std::cout << "Saving to btree.txt\n";
    if (!t.Save("btree.txt")) {
        std::cerr << "Error: no se pudo guardar btree.txt\n";
        return 1;
    }

    // --- Cargar en otro árbol ---
    Line();
    std::cout << "Loading from btree.txt\n";
    BTree<Trait> u(/*order=*/3, /*unique=*/true);
    if (!u.Load("btree.txt")) {
        std::cerr << "Error: no se pudo cargar btree.txt\n";
        return 1;
    }

    std::cout << "size=" << u.size() << ", height=" << u.height() << "\n";
    u.Print(std::cout);

    // Fin
    Line();
    std::cout << "Done.\n";
    return 0;

}
