
#include <iostream>
#include <vector>
#include <utility>
#include "btree.h"
using namespace std;

using DemoTraits = BTreeTrait<int, long, std::less<int>>;

static void printSep(const char* t) {
	cout << "\n======= " << t << " =======\n";
}

void DemoBTree() {
	// Create a BTree and insert some elements
	BTree<DemoTraits> tree(3);
	cout << "Inserting values: 10, 20, 5, 15, 25\n";
	tree.Insert(10, 100);
	tree.Insert(20, 200);
	tree.Insert(5, 50);
	tree.Insert(15, 150);
	tree.Insert(25, 250);

	printSep("Print original tree");
	tree.Print(cout);

    printSep("Tree (operator<<)");
    std::cout << tree << "\n";

    // Iterate with forward iterator (pre-increment)
    printSep("Iterate with ++it");
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        std::cout << "key=" << it->key << " obj=" << it->ObjID << "\n";
    }

    // Test post-increment semantics
    printSep("Test post-increment it++");
    auto it = tree.begin();
    if (it != tree.end()) {
        auto old = it++; // old points to first, it advanced to second
        std::cout << "old.key=" << old->key << " now.key=" << it->key << "\n";
    }

	// Test write to disk
	const string fname = "btree_test.bin";
	printSep("Write to file");
	if (tree.Write(fname))
		cout << "Wrote tree to '" << fname << "'\n";
	else
		cout << "Failed to write tree to '" << fname << "'\n";

	// Test read into another tree
	printSep("Read into new tree");
	BTree<DemoTraits> tree2(3);
	if (tree2.Read(fname)) {
		cout << "Read succeeded. Contents of tree2:" << endl;
		tree2.Print(cout);
	} else {
		cout << "Read failed." << endl;
	}

	// Test move constructor
	printSep("Move constructor");
	BTree<DemoTraits> movedTree(std::move(tree));
	cout << "Contents of movedTree (after move):" << endl;
	movedTree.Print(cout);


	printSep("Done :D");
}