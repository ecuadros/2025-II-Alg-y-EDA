#include <iostream>
#include "rtree.h"

using namespace std;

typedef RTreeTrait<int, long> IntRTreeTrait;

int main() {
    cout << "Creating R-Tree...\n";
    CRTree<IntRTreeTrait> rtree(4);
    cout << "R-Tree created successfully!\n";

    cout << "Inserting rectangles...\n";
    rtree.Insert(Rectangle<int>(0, 0, 10, 10), 1);
    cout << "Insert 1 OK\n";

    rtree.Insert(Rectangle<int>(5, 5, 15, 15), 2);
    cout << "Insert 2 OK\n";

    rtree.Insert(Rectangle<int>(20, 20, 30, 30), 3);
    cout << "Insert 3 OK\n";

    rtree.Insert(Rectangle<int>(25, 25, 35, 35), 4);
    cout << "Insert 4 OK\n";

    rtree.Insert(Rectangle<int>(40, 0, 50, 10), 5);
    cout << "Insert 5 OK\n";

    cout << "\nHeight: " << rtree.GetHeight() << "\n";
    cout << "Size: " << rtree.GetSize() << "\n";

    cout << "\nTesting Range Query...\n";
    vector<long> results;
    rtree.RangeQuery(Rectangle<int>(0, 0, 20, 20), results);
    cout << "Found " << results.size() << " results\n";

    cout << "\nTesting Write to Disk...\n";
    if (rtree.WriteToFile("test.bin")) {
        cout << "Write OK\n";
    }

    cout << "\nTesting Read from Disk...\n";
    CRTree<IntRTreeTrait> rtree2(4);
    if (rtree2.ReadFromFile("test.bin")) {
        cout << "Read OK\n";
        cout << "Height: " << rtree2.GetHeight() << "\n";
        cout << "Size: " << rtree2.GetSize() << "\n";
    }

    cout << "\nTesting Remove...\n";
    if (rtree.Remove(Rectangle<int>(5, 5, 15, 15), 2)) {
        cout << "Remove OK\n";
        cout << "New size: " << rtree.GetSize() << "\n";
    }

    return 0;
}
