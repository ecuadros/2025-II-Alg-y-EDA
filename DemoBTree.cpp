#include <iostream>
#include <functional>
#include <fstream>
#include "btree.h"
#include "types.h"

void testForEach(BTree<BTreeAscTrait<T1, Ref>>::ObjectInfo& info, size_t level, T2 value){
    info.ObjID += value;
}

bool testFirstThat(BTree<BTreeAscTrait<T1, Ref>>::ObjectInfo& info, size_t level, T2 limInf){
    if(info.ObjID > limInf) return true;
    return false;
}

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

    std::cout << "TREE 2 WRITE" << std::endl;
    std::cout << tree2 << std::endl;

    std::ofstream of("btree.txt");
    tree2.Write(of);
    of.close();

    BTree<BTreeDescTrait<int, long>> tree3;
    std::ifstream ifs("btree.txt");
    tree2.Read(ifs);
    ifs.close();
    std::cout << "TREE 2 READ" << std::endl;
    std::cout << tree2 << std::endl;

    T1 value = 100;
    tree2.ForEach_variadic(testForEach, value);
    std::cout << "TREE 2 AFTER FOREACH" << std::endl;
    std::cout << tree2 << std::endl;

    T1 limInf = 309;
    auto obj = tree2.FirstThat_variadic(testFirstThat, limInf);
    std::cout << "TREE 2 FIRST THAT" << std::endl;
    std::cout << obj->key << std::endl;
    std::cout << obj->ObjID << std::endl;

}

// int main() {
//     DemoBTree();
//     return 0;
// }
