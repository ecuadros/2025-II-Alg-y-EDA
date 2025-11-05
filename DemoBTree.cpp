#include <iostream>
#include <functional>
#include "btree.h"
#include "types.h"

void DemoBTree(){
    std::cout << "Demo B tree (ASC): " << std::endl;
    vector<pair<T1, Ref>> v1 = {
        {45, 200}, {15, 201}, {75, 202}, {5, 203}, 
        {25, 204}, {55, 205}, {85, 206}, {35, 207}, 
        {65, 208}, {95, 209}, {40, 210}
    };


    BTree<BTreeAscTrait<T1, Ref>> tree1(3, true);

    for (auto &par : v1)
        tree1.Insert(par.first, par.second);

    tree1.Print(cout);

    std::cout << "Demo B tree (DESC): " << std::endl;
    // vector<pair<T1, Ref>> v1 = {
    //     {45, 200}, {15, 201}, {75, 202}, {5, 203}, 
    //     {25, 204}, {55, 205}, {85, 206}, {35, 207}, 
    //     {65, 208}, {95, 209}, {40, 210}
    // };


    BTree<BTreeDescTrait<T1, Ref>> tree2(3, true);

    for (auto &par : v1)
        tree2.Insert(par.first, par.second);

    tree2.Print(std::cout);


    std::cout << "Altura: " << tree2.height() << ", Claves: " << tree2.size() << std::endl;


    std::cout << "Search" << std::endl;
    int buscar[] = {45, 65, 90};
    for (int k : buscar) {
        Ref id = tree1.Search(k);
        if (id != -1)
            cout << "Encontrado " << k << ": ObjID=" << id << endl;
        else
            cout << "Clave " << k << " no encontrada" << endl;
    }

    std::cout << "Eliminacion" << std::endl;
    vector<pair<T1, Ref>> v2 = {
        {75, 202}, {35, 207}
    };

    for (auto &par : v2)
        tree1.Remove(par.first, par.second);

    tree1.Print(std::cout);

    std::cout << tree1 << std::endl;
}

// int main() {
//     DemoBTree();
//     return 0;
// }