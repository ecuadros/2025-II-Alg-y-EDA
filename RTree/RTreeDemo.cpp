#include "RTreeDemo.h"

#include "types.h"
#include "RTree.h"
#include <cassert>
#include <sstream>
#include <iostream>

void RTreeDemo() {
    using Traits = RTreeTraits;
    CRTree<Traits> tree;

    // ---------------------------------------
    // 1. Inserción básica
    // ---------------------------------------
    Rect r1{0, 0, 1, 1};
    Rect r2{2, 2, 3, 3};
    Rect r3{4, 4, 5, 5};
    Rect r4{6, 6, 7, 7};
    Rect r5{8, 8, 9, 9};
    Rect r6{10, 10, 11, 11};
    Rect r7{12, 12, 13, 13};
    Rect r8{14, 14, 15, 15};
    Rect r9{16, 16, 17, 17}; // fuerza split

    tree.Insert(r1, 1);
    tree.Insert(r2, 2);
    tree.Insert(r3, 3);
    tree.Insert(r4, 4);
    tree.Insert(r5, 5);
    tree.Insert(r6, 6);
    tree.Insert(r7, 7);
    tree.Insert(r8, 8);
    tree.Insert(r9, 9);

    std::cout << "[OK] Inserciones realizadas\n";

    // ---------------------------------------
    // 2. Range Query simple
    // ---------------------------------------
    Rect query{3, 3, 10, 10};
    std::vector<Ref> result;
    tree.RangeQuery(query, result);

    std::cout << "RangeQuery result: ";
    for (auto id : result)
        std::cout << id << " ";
    std::cout << "\n";

    assert(!result.empty());
    std::cout << "[OK] RangeQuery básico\n";

    // ---------------------------------------
    // 3. Range Query vacío
    // ---------------------------------------
    Rect emptyQuery{100, 100, 200, 200};
    result.clear();
    tree.RangeQuery(emptyQuery, result);

    assert(result.empty());
    std::cout << "[OK] RangeQuery vacío\n";

    // ---------------------------------------
    // 4. Persistencia (Write)
    // ---------------------------------------
    std::stringstream ss;
    tree.Write(ss);

    assert(!ss.str().empty());
    std::cout << "[OK] Escritura en stream\n";

    // ---------------------------------------
    // 5. Inserción incremental + consultas
    // ---------------------------------------
    for (int i = 20; i < 40; ++i) {
        Rect r{(float)i, (float)i, (float)i + 0.5f, (float)i + 0.5f};
        tree.Insert(r, i);
    }

    Rect largeQuery{0, 0, 50, 50};
    result.clear();
    tree.RangeQuery(largeQuery, result);

    assert(result.size() >= 9);
    std::cout << "[OK] Inserciones incrementales\n";
}
