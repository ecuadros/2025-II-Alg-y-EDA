#ifndef __DEMO_RTREE_H__
#define __DEMO_RTREE_H__

#include <iostream>
#include <fstream>
#include "Rtree.h"

using namespace std;

void DemoRTree() {
    cout << "R-Tree Demo" << endl << endl;

    using MyTrait = RTreeTrait<int, string>;
    RTree<MyTrait> tree(4, 2);

    // Insert
    cout << "Insert" << endl;
    tree.Insert(0, 0, 10, 10, "R1");
    cout << "R1 insertado" << endl;
    tree.Insert(20, 20, 30, 30, "R2");
    cout << "R2 insertado" << endl;
    tree.Insert(5, 5, 15, 15, "R3");
    cout << "R3 insertado" << endl;
    tree.Insert(100, 100, 110, 110, "R4");
    cout << "R4 insertado" << endl;
    tree.Insert(2, 2, 3, 3, "R5");
    cout << "R5 insertado" << endl;

    cout << endl << "Insertando R6 (provoca split)" << endl;
    tree.Insert(50, 50, 60, 60, "R6");
    cout << "R6 insertado" << endl;

    tree.Insert(0, 50, 10, 60, "R7");
    cout << "R7 insertado" << endl << endl;

    tree.Print();

    // Range Query
    cout << endl << "Range Query" << endl;
    cout << "Query: [(0,0)-(12,12)]" << endl;
    auto results = tree.RangeQuery(0, 0, 12, 12);
    cout << "Encontrados " << results.size() << " elementos: ";
    for (const auto& r : results)
        cout << r.second << " ";
    cout << endl << endl;

    // Write to disk
    cout << "Guardando en archivo..." << endl;
    tree.WriteToDisk("rtree_save.txt");
    cout << endl;

    // Read from disk
    cout << "Cargando desde archivo..." << endl;
    RTree<MyTrait> tree2(4, 2);
    tree2.ReadFromDisk("rtree_save.txt");

    cout << "Query en arbol cargado:" << endl;
    results = tree2.RangeQuery(0, 0, 12, 12);
    cout << "Encontrados: ";
    for (const auto& r : results)
        cout << r.second << " ";
    cout << endl << endl;

    // Otra query
    cout << "Query2: [(90,90)-(120,120)]" << endl;
    results = tree2.RangeQuery(90, 90, 120, 120);
    cout << "Encontrados: ";
    for (const auto& r : results)
        cout << r.second << " ";
    cout << endl << endl;

    // Delete
    cout << "Eliminando R1..." << endl;
    bool removed = tree2.Remove(0, 0, 10, 10, "R1");
    cout << "Status: " << (removed ? "OK" : "Fallo") << endl;

    cout << "Query despues de eliminar:" << endl;
    results = tree2.RangeQuery(0, 0, 12, 12);
    cout << "Encontrados: ";
    for (const auto& r : results)
        cout << r.second << " ";
    cout << endl << endl;

    tree2.Print();
}

#endif