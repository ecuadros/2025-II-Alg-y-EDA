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
    // std::cout << "Delete(10), Delete(70)\n";
        std::cout << "Delete(10))\n";
    t.Remove(10, 0);
    // t.Remove(70, 0);
    std::cout << "size=" << t.size() << ", height=" << t.height() << "\n";
    t.Print(std::cout);

    return 0;

}
