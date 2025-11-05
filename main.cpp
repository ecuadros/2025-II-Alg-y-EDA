#include <iostream>
#include <functional>
#include "types.h"
#include "btree.h"

int main()
{

    std::cout << "B Tree Asc " << std::endl;
    vector<pair<T1, Ref>> v1 = {
        {10, 300}, {20, 301}, {30, 302}, {40, 303}, {50, 304}, {60, 305}, {70, 306}, {80, 307}, {90, 308}, {100, 309}, {110, 310}};

    BTree<BTreeAscTrait<T1, Ref>> tree1(3, true);

    for (auto &par : v1)
    {
        tree1.Insert(par.first, par.second);
    }

    tree1.Print(cout);

    std::cout << "B Tree Desc " << std::endl;
    vector<pair<T1, Ref>> v2 = {
        {100, 400}, {90, 401}, {80, 402}, {70, 403}, {60, 404}, {50, 405}, {40, 406}, {30, 407}, {20, 408}, {10, 409}};

    BTree<BTreeDescTrait<T1, Ref>> tree2(3, true);

    for (auto &par : v2)
    {
        tree2.Insert(par.first, par.second);
    }

    tree2.Print(cout);

    tree1.Write("btree_data.txt");

    BTree<BTreeDescTrait<T1, Ref>> tree3(3, true);
    tree3.Read("btree_data.txt");

    tree3.Print(cout);

    tree1.ForEach(
        [](auto &info, size_t level, std::ostream *out)
        {
            *out << std::string(level, '\t') << info.key << '\n';
        },
        &std::cout);
    return 0;
}