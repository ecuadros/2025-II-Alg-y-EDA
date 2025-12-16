#include "rtree_trait.h"
#include "rtree_rectangle.h"
#include "rtreenode.h"
#include "rtree.h"

#include <iostream>
#include <fstream>
using namespace std;

int main()
{
    using MyTrait = RTreeTrait<int, 2, long>;
    using Tree    = RTree<MyTrait>;
    using Rect    = Rectangle<MyTrait>;

    Tree myTree;

    cout << "insert data" << endl;
    for(int i=0; i<20; ++i)
    {
        Rect r;
        int p[] = {i, i};
        r.Set(p, p);
        myTree.Insert(r, i);
    }
    cout << "tree height: " << myTree.m_Root->m_Level << endl;

    cout << endl;

    cout << "searching range [5,5] to [8,8]..." << endl;

    Rect searchBox;
    int minS[] = {5, 5};
    int maxS[] = {8, 8};
    searchBox.Set(minS, maxS);

    auto results = myTree.Search(searchBox);

    cout << "found: " << results.size() << " elements." << endl;
    cout << "IDs: ";
    bool pass = true;
    for(long id : results)
    {
        cout << id << "; ";
        if (id < 5 || id > 8) pass = false;
    }
    cout << endl;

    ofstream outFile("rtree_output.txt");
    if (outFile.is_open()) {
        outFile << myTree;
        outFile.close();
        cout << "written to rtree_output.txt" << endl;
    } else {
        cout << "err opening file" << endl;
    }

    cout << endl;
    cout << "reading tree from file..." << endl;
    Tree loadedTree;
    ifstream inFile("rtree_output.txt");
    if (inFile.is_open()) {
        inFile >> loadedTree;
        inFile.close();
        cout << "tree loaded from rtree_output.txt" << endl;
        cout << "loaded tree height: " << loadedTree.m_Root->m_Level << endl;

        cout << endl;
        cout << "searching loaded tree in range [5,5] to [8,8]..." << endl;
        auto loadedResults = loadedTree.Search(searchBox);
        cout << "found: " << loadedResults.size() << " elements." << endl;
        cout << "IDs: ";
        for(long id : loadedResults) {
            cout << id << " ";
        }
        cout << endl;
    } else {
        cout << "err opening file for reading" << endl;
    }

    cout << myTree;

    cout << endl;
    cout << "deleting IDs 5..8..." << endl;
    for (int id = 5; id <= 8; ++id)
    {
        Rect delRect;
        int p[] = {id, id};
        delRect.Set(p, p);
        bool ok = myTree.Remove(delRect, id);
        cout << "remove(" << id << ") -> " << (ok ? "true" : "false") << endl;
    }

    cout << endl;
    cout << "searching again after deletions..." << endl;
    auto resultsAfter = myTree.Search(searchBox);
    cout << "found: " << resultsAfter.size() << " elements." << endl;
    cout << "IDs: ";
    for(long id : resultsAfter)
    {
        cout << id << " ";
    }
    cout << endl;

    return 0;
}