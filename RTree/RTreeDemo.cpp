#include "RTreeDemo.h"

#include "types.h"
#include "RTree.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

#define TEST_CASE(name) std::cout << "[TEST] " << name << std::endl;

#define TEST_OK() std::cout << "  -> OK\n";


void TestInsertAndSplit() {
    TEST_CASE("Insert & Split");

    CRTree<RTreeTraits> tree;

    for (int i = 0; i < 20; ++i) {
        Rect r{(float)i, (float)i, (float)i + 1, (float)i + 1};
        tree.Insert(r, i);
    }

    std::vector<Ref> res;
    tree.RangeQuery({-1, -1, 100, 100}, res);

    assert(res.size() == 20);
    TEST_OK();
}

void TestRangeQuery() {
    TEST_CASE("RangeQuery");

    CRTree<RTreeTraits> tree;

    tree.Insert({0, 0, 2, 2}, 1);
    tree.Insert({3, 3, 5, 5}, 2);
    tree.Insert({6, 6, 8, 8}, 3);

    std::vector<Ref> res;
    tree.RangeQuery({1, 1, 4, 4}, res);

    assert(res.size() == 2);
    TEST_OK();
}

void TestDeleteSimple() {
    TEST_CASE("Delete Simple");

    CRTree<RTreeTraits> tree;

    Rect r{0, 0, 1, 1};
    tree.Insert(r, 42);

    std::vector<Ref> res;
    tree.RangeQuery({0, 0, 2, 2}, res);
    assert(res.size() == 1);

    tree.Delete(r, 42);
    res.clear();
    tree.RangeQuery({0, 0, 2, 2}, res);

    assert(res.empty());
    TEST_OK();
}

void TestDeleteUnderflow() {
    TEST_CASE("Delete Underflow + Reinsertion");

    CRTree<RTreeTraits> tree;

    for (int i = 0; i < 12; ++i) {
        Rect r{(float)i, 0, (float)i + 1, 1};
        tree.Insert(r, i);
    }

    // Provocar underflow
    for (int i = 0; i < 8; ++i) {
        Rect r{(float)i, 0, (float)i + 1, 1};
        tree.Delete(r, i);
    }

    std::vector<Ref> res;
    tree.RangeQuery({0, 0, 20, 20}, res);

	std::cout << res.size() << std::endl;
    assert(res.size() == 4);
    TEST_OK();
}

void TestWriteReadStream() {
    TEST_CASE("Write / Read Stream");

    CRTree<RTreeTraits> tree;

    for (int i = 0; i < 10; ++i)
        tree.Insert({(float)i, (float)i, (float)i + 1, (float)i + 1}, i);

    std::stringstream ss;
    tree.Write(ss);

    CRTree<RTreeTraits> loaded;
    loaded.Read(ss);

    std::vector<Ref> res;
    loaded.RangeQuery({0, 0, 100, 100}, res);

    assert(res.size() == 10);
    TEST_OK();
}

void TestWriteReadFile() {
    TEST_CASE("WriteToFile / ReadFromFile");

    CRTree<RTreeTraits> tree;
    tree.Insert({1, 1, 2, 2}, 99);

    tree.WriteToFile("rtree_test.dat");

    CRTree<RTreeTraits> loaded;
    loaded.ReadFromFile("rtree_test.dat");

    std::vector<Ref> res;
    loaded.RangeQuery({0, 0, 10, 10}, res);

    assert(res.size() == 1 && res[0] == 99);
    TEST_OK();
}

void RTreeDemo() {
    TestInsertAndSplit();
    TestRangeQuery();
    TestDeleteSimple();
    TestDeleteUnderflow();
    TestWriteReadStream();
    TestWriteReadFile();
    std::cout << "\n=== ALL RTREE TESTS PASSED ===\n";
}
