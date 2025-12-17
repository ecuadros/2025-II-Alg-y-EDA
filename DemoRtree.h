#ifndef __DEMO_RTREE_H__
#define __DEMO_RTREE_H__

#include <iostream>
#include <fstream>
#include "Rtree.h"

using namespace std;

void DemoRTree() {
    cout << "R-Tree Demo" << endl << endl;

    // Probar R-Tree 2D
    using Trait2D = RTreeTrait<int, string, 2>;
    RTree<Trait2D> tree(4, 2);

    // Insert
    cout << "Insert" << endl;
    tree.Insert({0, 0}, {10, 10}, "R1");
    cout << "R1 insertado" << endl;
    tree.Insert({20, 20}, {30, 30}, "R2");
    cout << "R2 insertado" << endl;
    tree.Insert({5, 5}, {15, 15}, "R3");
    cout << "R3 insertado" << endl;
    tree.Insert({100, 100}, {110, 110}, "R4");
    cout << "R4 insertado" << endl;
    tree.Insert({50, 50}, {60, 60}, "R5");
    cout << "R5 insertado" << endl;

    tree.Print();

    // Range Query
    cout << "RangeQuery [(0,0)-(12,12)]" << endl;
    auto res = tree.RangeQuery({0, 0}, {12, 12});
    cout << "Encontrados: ";
    for (const auto& r : res) cout << r.second << " ";
    cout << endl << endl;

    // Write to Disk
    cout << "Guardando..." << endl;
    tree.WriteToDisk("rtree_data.txt");

    // Read from Disk
    cout << "Cargando..." << endl;
    RTree<Trait2D> tree2(4, 2);
    tree2.ReadFromDisk("rtree_data.txt");

    // Delete
    cout << "Eliminando R1..." << endl;
    bool removed = tree.Remove({0, 0}, {10, 10}, "R1");
    cout << "Status: " << (removed ? "Ok" : "Fallo") << endl;

    cout << "Query despues de eliminar: ";
    res = tree.RangeQuery({0, 0}, {12, 12});
    for (const auto& r : res) cout << r.second << " ";
    cout << endl;

    tree.Print();

    /*
    // Demo 3D 
    using Trait3D = RTreeTrait<int, string, 3>;
    RTree<Trait3D> tree3d(4, 2);
    tree3d.Insert({0, 0, 0}, {10, 10, 10}, "Cubo1");
    tree3d.Insert({20, 20, 20}, {30, 30, 30}, "Cubo2");
    tree3d.Print();
    */
}

#endif