//tstbtree.cc  
//Author: Chicana D�az, Johan Pier
//Bachelor in Engineering of System
//#include <iostream.h>
#include <time.h>
#include <stdlib.h>

#include "btree.h"
#include <string>
#include <functional>
#include <iostream>
#include <fstream>

using namespace std;

//const char * keys="CDAMPIWNBKEHOLJYQZFXVRTSGU";
const char * keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
const char * keys2 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const char * keys3 = "DYZakHIUwxVJ203ejOP9Qc8AdtuEop1XvTRghSNbW567BfiCqrs4FGMyz";

const int BTreeSize = 3;

//test compare
void test_default_comparison() {
       using CharTrait = BTreeTrait<char, long>;
       BTree<CharTrait> bt(BTreeSize);

       for (int i = 0; keys1[i]; i++) {
               bt.Insert(keys1[i], i*i);
       }
       bt.Print(cout);
}

void test_descending_comparison() {
       using CharTraitDesc = BTreeTrait<char, long, std::greater<char>>;
       BTree<CharTraitDesc> bt(BTreeSize);

       const char* keys = "ABCDEFGHIJ";
       for (int i = 0; keys[i]; i++) {
               bt.Insert(keys[i], i);
       }
       bt.Print(cout);
}

void test_integer_comparison() {
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(BTreeSize);

       int numbers[] = {50, 30, 70, 20, 40, 60, 80, 10, 25, 35, 45};
       for (int i = 0; i < 11; i++) {
               bt.Insert(numbers[i], i);
       }
       bt.Print(cout);
}

void test_search() {
       using CharTrait = BTreeTrait<char, long>;
       BTree<CharTrait> bt(BTreeSize);

       for (int i = 0; keys2[i]; i++) {
               bt.Insert(keys2[i], i);
       }

       cout << "Searching for '5', 'A', 'z':\n";
       long objID = bt.Search('5');
       if (objID != -1)
               cout << "Found '5' with ObjID = " << objID << endl;
       else
               cout << "Not found '5'\n";

       objID = bt.Search('A');
       if (objID != -1)
               cout << "Found 'A' with ObjID = " << objID << endl;
       else
               cout << "Not found 'A'\n";

       objID = bt.Search('z');
       if (objID != -1)
               cout << "Found 'z' with ObjID = " << objID << endl;
       else
               cout << "Not found 'z'\n";
}

void test_remove() {
       using CharTrait = BTreeTrait<char, long>;
       BTree<CharTrait> bt(BTreeSize);

       const char* keys = "ABCDEFGHIJ";
       for (int i = 0; keys[i]; i++) {
               bt.Insert(keys[i], i);
       }

       cout << "Before removal:\n";
       bt.Print(cout);

       cout << "\nRemoving 'E':\n";
       if (bt.Remove('E', -1))
               cout << "Successfully removed 'E'\n";
       else
               cout << "Failed to remove 'E'\n";

       bt.Print(cout);
}
// fin test compare

// test move constructor
void test_move_constructor() {
       using CharTrait = BTreeTrait<char, long>;
       BTree<CharTrait> bt1(BTreeSize);
       const char* keys = "ABCDEFGHIJ";
       for (int i = 0; keys[i]; i++) {
               bt1.Insert(keys[i], i);
       }

       cout << "Original tree (bt1) before move:\n";
       cout << "  Size: " << bt1.size() << endl;
       cout << "  Height: " << bt1.height() << endl;

       // Move constructor
       BTree<CharTrait> bt2(std::move(bt1));
       cout << "\nAfter move construction:\n";
       cout << "  bt2 size: " << bt2.size() << endl;
       cout << "  bt2 height: " << bt2.height() << endl;
       cout << "  bt1 size (should be 0): " << bt1.size() << endl;

       cout << "\nbt2 tree content:\n";
       bt2.Print(cout);
}

void test_move_assignment() {
       using IntTrait = BTreeTrait<int, long>;

       // Create and populate first tree
       BTree<IntTrait> bt1(BTreeSize);
       int numbers1[] = {10, 20, 30, 40, 50};
       for (int i = 0; i < 5; i++) {
               bt1.Insert(numbers1[i], i);
       }

       // Create and populate second tree
       BTree<IntTrait> bt2(BTreeSize);
       int numbers2[] = {100, 200, 300};
       for (int i = 0; i < 3; i++) {
               bt2.Insert(numbers2[i], i);
       }

       cout << "Before move assignment:\n";
       cout << "  bt1 size: " << bt1.size() << endl;
       cout << "  bt2 size: " << bt2.size() << endl;

       bt2 = std::move(bt1);

       cout << "\nAfter move assignment:\n";
       cout << "  bt2 size: " << bt2.size() << " (should be 5)" << endl;
       cout << "  bt1 size: " << bt1.size() << " (should be 0)" << endl;

       cout << "\nbt2 tree content:\n";
       bt2.Print(cout);
}

BTree<BTreeTrait<int, long>> createTree() {
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(BTreeSize);

       for (int i = 1; i <= 10; i++) {
               bt.Insert(i * 10, i);
       }
       return bt;  
}

void test_return_value_optimization() {
       auto bt = createTree();

       cout << "Returned tree:\n";
       cout << "  Size: " << bt.size() << endl;
       cout << "  Height: " << bt.height() << endl;
       bt.Print(cout);
}
//fin move

// foreach
void test_generic_foreach() {
       using CharTrait = BTreeTrait<char, long>;
       BTree<CharTrait> bt(BTreeSize);

       const char* keys = "ABCDEFGHIJ";
       for (int i = 0; keys[i]; i++) {
               bt.Insert(keys[i], i);
       }
       bt.ForEach([](auto& info) {
               cout << info.key << " ";
       });
       cout << "\n";
       int count = 0;
       bt.ForEach([](auto& info, int& counter) {
               cout << info.key << "(" << counter++ << ") ";
       }, count);
       cout << "\nTotal elements: " << count << "\n";
}

void test_generic_foreach_functor() {
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(BTreeSize);

       int numbers[] = {5, 3, 7, 1, 9, 4, 6, 8, 2};
       for (int i = 0; i < 9; i++) {
               bt.Insert(numbers[i], i);
       }

       struct SumAccumulator {
               int sum = 0;
               void operator()(const auto& info) {
                       sum += info.key;
               }
       };

       SumAccumulator acc;
       bt.ForEach(std::ref(acc));
       cout << "Sum of all keys: " << acc.sum << "\n";
}
void test_generic_firstthat() {

       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(BTreeSize);

       int numbers[] = {10, 20, 30, 40, 50, 60, 70};
       for (int i = 0; i < 7; i++) {
               bt.Insert(numbers[i], i);
       }

       auto* result = bt.FirstThat([](const auto& info) {
               return info.key > 35;
       });

       if (result) {
               cout << "First element > 35: " << result->key << "\n";
       } else {
               cout << "No element found\n";
       }

       int threshold = 55;
       result = bt.FirstThat([](const auto& info, int thresh) {
               return info.key > thresh;
       }, threshold);

       if (result) {
               cout << "First element > " << threshold << ": " << result->key << "\n";
       }
}

void test_generic_foreach_multiparams() {
       using CharTrait = BTreeTrait<char, long>;
       BTree<CharTrait> bt(BTreeSize);

       const char* keys = "HELLO";
       for (int i = 0; keys[i]; i++) {
               bt.Insert(keys[i], i);
       }

       cout << "Custom formatting with prefix and suffix:\n";
       string prefix = "[";
       string suffix = "]";
       bt.ForEach([](const auto& info, const string& pre, const string& suf) {
               cout << pre << info.key << suf << " ";
       }, prefix, suffix);
       cout << "\n";
}

void test_oldstyle_vs_generic() {
       using CharTrait = BTreeTrait<char, long>;
       BTree<CharTrait> bt(BTreeSize);

       const char* keys = "ABC";
       for (int i = 0; keys[i]; i++) {
               bt.Insert(keys[i], i);
       }
       struct PrintHelper {
               static void print(tagObjectInfo<char, long>& info, void* extra) {
                       ostream& os = *(ostream*)extra;
                       os << info.key << " ";
               }
       };
       bt.ForEach(&PrintHelper::print, &cout);
       cout << "\n";

       bt.ForEach([](const auto& info) {
               cout << info.key << " ";
       });
       cout << "\n";
}
//fin for each y first

// test iteradores
void test_iterator_basic() {
       cout << "\n=== Test Iterator Basic ===\n";
       using CharTrait = BTreeTrait<char, long>;
       BTree<CharTrait> bt(BTreeSize);

       const char* keys = "ABCDEFGHIJ";
       for (int i = 0; keys[i]; i++) {
               bt.Insert(keys[i], i);
       }

       cout << "Iterating with range-based for loop:\n";
       for (auto& info : bt) {
               cout << info.key << " ";
       }
       cout << "\n";

       cout << "\nIterating with explicit iterator:\n";
       for (auto it = bt.begin(); it != bt.end(); ++it) {
               cout << it->key << "(" << it->ObjID << ") ";
       }
       cout << "\n";
}

void test_iterator_empty_tree() {
       cout << "\n=== Test Iterator Empty Tree ===\n";
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(BTreeSize);

       cout << "Iterating over empty tree (should print nothing):\n";
       int count = 0;
       for (auto& info : bt) {
               cout << info.key << " ";
               count++;
       }
       cout << "Iterated " << count << " elements\n";
}

void test_iterator_single_element() {
       cout << "\n=== Test Iterator Single Element ===\n";
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(BTreeSize);

       bt.Insert(42, 100);

       cout << "Single element tree:\n";
       for (auto& info : bt) {
               cout << "Key: " << info.key << ", ObjID: " << info.ObjID << "\n";
       }
}

void test_iterator_with_stl_algorithms() {
       cout << "\n=== Test Iterator with STL Algorithms ===\n";
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(BTreeSize);

       int numbers[] = {5, 3, 7, 1, 9, 4, 6, 8, 2};
       for (int i = 0; i < 9; i++) {
               bt.Insert(numbers[i], i);
       }

       cout << "Elements in order:\n";
       for (auto& info : bt) {
               cout << info.key << " ";
       }
       cout << "\n";

       // Count elements
       auto count = std::distance(bt.begin(), bt.end());
       cout << "Total elements (using std::distance): " << count << "\n";

       // Find element
       auto it = std::find_if(bt.begin(), bt.end(),
               [](const auto& info) { return info.key == 5; });

       if (it != bt.end()) {
               cout << "Found element with key 5, ObjID: " << it->ObjID << "\n";
       }

       // Count elements greater than 5
       auto count_gt5 = std::count_if(bt.begin(), bt.end(),
               [](const auto& info) { return info.key > 5; });
       cout << "Elements > 5: " << count_gt5 << "\n";
}

void test_iterator_large_tree() {
       cout << "\n=== Test Iterator Large Tree ===\n";
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(BTreeSize);

       // Insert 20 elements
       for (int i = 1; i <= 20; i++) {
               bt.Insert(i * 5, i);
       }

       cout << "First 10 elements:\n";
       int count = 0;
       for (auto& info : bt) {
               cout << info.key << " ";
               if (++count >= 10) break;
       }
       cout << "\n";

       cout << "All elements:\n";
       for (auto& info : bt) {
               cout << info.key << " ";
       }
       cout << "\n";
       cout << "Total elements: " << bt.size() << "\n";
}

void test_iterator_modification() {
       cout << "\n=== Test Iterator Modification ===\n";
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(BTreeSize);

       int numbers[] = {10, 20, 30, 40, 50};
       for (int i = 0; i < 5; i++) {
               bt.Insert(numbers[i], i);
       }

       cout << "Original ObjIDs:\n";
       for (auto& info : bt) {
               cout << "Key: " << info.key << ", ObjID: " << info.ObjID << "\n";
       }

       // Modify ObjID through iterator
       cout << "\nModifying ObjIDs to 999:\n";
       for (auto& info : bt) {
               info.ObjID = 999;
       }

       cout << "After modification:\n";
       for (auto& info : bt) {
               cout << "Key: " << info.key << ", ObjID: " << info.ObjID << "\n";
       }
}

void test_iterator_comparison() {
       cout << "\n=== Test Iterator Comparison ===\n";
       using CharTrait = BTreeTrait<char, long>;
       BTree<CharTrait> bt(BTreeSize);

       const char* keys = "ABCDE";
       for (int i = 0; keys[i]; i++) {
               bt.Insert(keys[i], i);
       }

       auto it1 = bt.begin();
       auto it2 = bt.begin();
       auto it_end = bt.end();

       cout << "it1 == it2: " << (it1 == it2 ? "true" : "false") << "\n";
       cout << "it1 != it_end: " << (it1 != it_end ? "true" : "false") << "\n";

       ++it2;
       cout << "After ++it2:\n";
       cout << "it1 == it2: " << (it1 == it2 ? "true" : "false") << "\n";
       cout << "it1 != it2: " << (it1 != it2 ? "true" : "false") << "\n";
}
//fin iteradores

// test reverse iterator
void test_reverse_iterator_basic() {
       cout << "\n=== Test Reverse Iterator Basic ===\n";
       using CharTrait = BTreeTrait<char, long>;
       BTree<CharTrait> bt(BTreeSize);

       const char* keys = "ABCDEFGHIJ";
       for (int i = 0; keys[i]; i++) {
               bt.Insert(keys[i], i);
       }

       cout << "Forward iteration:\n";
       for (auto& info : bt) {
               cout << info.key << " ";
       }
       cout << "\n";

       cout << "Reverse iteration with rbegin()/rend():\n";
       for (auto it = bt.rbegin(); it != bt.rend(); ++it) {
               cout << it->key << " ";
       }
       cout << "\n";
}

void test_reverse_iterator_decrement() {
       cout << "\n=== Test Reverse Iterator with Decrement ===\n";
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(BTreeSize);

       int numbers[] = {5, 3, 7, 1, 9, 4, 6, 8, 2};
       for (int i = 0; i < 9; i++) {
               bt.Insert(numbers[i], i);
       }

       cout << "Forward with operator++:\n";
       for (auto it = bt.begin(); it != bt.end(); ++it) {
               cout << it->key << " ";
       }
       cout << "\n";

       cout << "Backward with operator--:\n";
       auto it = bt.end();
       --it; // Move to last element
       while (true) {
               cout << it->key << " ";
               if (it == bt.begin()) break;
               --it;
       }
       cout << "\n";
}

void test_reverse_iterator_empty_tree() {
       cout << "\n=== Test Reverse Iterator Empty Tree ===\n";
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(BTreeSize);

       cout << "Reverse iterating over empty tree (should print nothing):\n";
       int count = 0;
       for (auto it = bt.rbegin(); it != bt.rend(); ++it) {
               cout << it->key << " ";
               count++;
       }
       cout << "Iterated " << count << " elements\n";
}

void test_reverse_iterator_single_element() {
       cout << "\n=== Test Reverse Iterator Single Element ===\n";
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(BTreeSize);

       bt.Insert(42, 100);

       cout << "Forward:\n";
       for (auto& info : bt) {
               cout << info.key << " ";
       }
       cout << "\n";

       cout << "Reverse:\n";
       for (auto it = bt.rbegin(); it != bt.rend(); ++it) {
               cout << it->key << " ";
       }
       cout << "\n";
}

void test_reverse_iterator_with_stl_algorithms() {
       cout << "\n=== Test Reverse Iterator with STL Algorithms ===\n";
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(BTreeSize);

       int numbers[] = {5, 3, 7, 1, 9, 4, 6, 8, 2};
       for (int i = 0; i < 9; i++) {
               bt.Insert(numbers[i], i);
       }

       cout << "Elements in reverse order:\n";
       for (auto it = bt.rbegin(); it != bt.rend(); ++it) {
               cout << it->key << " ";
       }
       cout << "\n";

       // Find element in reverse
       auto rit = std::find_if(bt.rbegin(), bt.rend(),
               [](const auto& info) { return info.key < 5; });

       if (rit != bt.rend()) {
               cout << "First element < 5 from end: " << rit->key << "\n";
       }

       // Count elements less than 5 (searching from end)
       auto count_lt5 = std::count_if(bt.rbegin(), bt.rend(),
               [](const auto& info) { return info.key < 5; });
       cout << "Elements < 5: " << count_lt5 << "\n";
}

void test_bidirectional_navigation() {
       cout << "\n=== Test Bidirectional Navigation ===\n";
       using CharTrait = BTreeTrait<char, long>;
       BTree<CharTrait> bt(BTreeSize);

       const char* keys = "ABCDE";
       for (int i = 0; keys[i]; i++) {
               bt.Insert(keys[i], i);
       }

       auto it = bt.begin();
       cout << "Start at begin: " << it->key << "\n";

       ++it;
       cout << "After ++: " << it->key << "\n";

       ++it;
       cout << "After ++: " << it->key << "\n";

       --it;
       cout << "After --: " << it->key << "\n";

       --it;
       cout << "After --: " << it->key << "\n";

       cout << "Back to begin: " << (it == bt.begin() ? "true" : "false") << "\n";
}

void test_reverse_iterator_modification() {
       cout << "\n=== Test Reverse Iterator Modification ===\n";
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(BTreeSize);

       int numbers[] = {10, 20, 30, 40, 50};
       for (int i = 0; i < 5; i++) {
               bt.Insert(numbers[i], i);
       }

       cout << "Original (forward):\n";
       for (auto& info : bt) {
               cout << "Key: " << info.key << ", ObjID: " << info.ObjID << "\n";
       }

       // Modify using reverse iterator
       cout << "\nModifying ObjIDs to 888 (reverse iteration):\n";
       for (auto it = bt.rbegin(); it != bt.rend(); ++it) {
               it->ObjID = 888;
       }

       cout << "After modification (forward):\n";
       for (auto& info : bt) {
               cout << "Key: " << info.key << ", ObjID: " << info.ObjID << "\n";
       }
}

void test_reverse_iterator_large_tree() {
       cout << "\n=== Test Reverse Iterator Large Tree ===\n";
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(BTreeSize);

       // Insert 15 elements
       for (int i = 1; i <= 15; i++) {
               bt.Insert(i * 10, i);
       }

       cout << "Forward (first 8):\n";
       int count = 0;
       for (auto& info : bt) {
               cout << info.key << " ";
               if (++count >= 8) break;
       }
       cout << "\n";

       cout << "Reverse (first 8):\n";
       count = 0;
       for (auto it = bt.rbegin(); it != bt.rend(); ++it) {
               cout << it->key << " ";
               if (++count >= 8) break;
       }
       cout << "\n";

       cout << "Full reverse:\n";
       for (auto it = bt.rbegin(); it != bt.rend(); ++it) {
               cout << it->key << " ";
       }
       cout << "\n";
}
//fin reverse iterator

// test operator<<
void test_operator_output_basic() {
       cout << "\n=== Test operator<< Basic ===\n";
       using CharTrait = BTreeTrait<char, long>;
       BTree<CharTrait> bt(BTreeSize);

       const char* keys = "ABCDE";
       for (int i = 0; keys[i]; i++) {
               bt.Insert(keys[i], i);
       }

       cout << "Using operator<<:\n";
       cout << bt;
}

void test_operator_output_empty_tree() {
       cout << "\n=== Test operator<< Empty Tree ===\n";
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(BTreeSize);

       cout << "Empty tree:\n";
       cout << bt;
}

void test_operator_output_large_tree() {
       cout << "\n=== Test operator<< Large Tree ===\n";
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(BTreeSize);

       for (int i = 1; i <= 15; i++) {
               bt.Insert(i * 10, i);
       }

       cout << "Large tree (15 elements):\n";
       cout << bt;
}

void test_operator_output_with_metadata() {
       cout << "\n=== Test operator<< Metadata Display ===\n";
       using CharTrait = BTreeTrait<char, long>;

       BTree<CharTrait> bt1(3);
       const char* keys1 = "ABCDEFGHIJ";
       for (int i = 0; keys1[i]; i++) {
               bt1.Insert(keys1[i], i);
       }

       cout << "Tree with order=3:\n";
       cout << bt1;

       BTree<CharTrait> bt2(5);
       const char* keys2 = "KLMNOPQRST";
       for (int i = 0; keys2[i]; i++) {
               bt2.Insert(keys2[i], i);
       }

       cout << "\nTree with order=5:\n";
       cout << bt2;
}

void test_operator_output_to_file() {
       cout << "\n=== Test operator<< to File ===\n";
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(BTreeSize);

       int numbers[] = {5, 3, 7, 1, 9, 4, 6, 8, 2};
       for (int i = 0; i < 9; i++) {
               bt.Insert(numbers[i], i);
       }

       // Output to file
       ofstream outFile("btree_output.txt");
       if (outFile.is_open()) {
               outFile << bt;
               outFile.close();
               cout << "Tree written to 'btree_output.txt'\n";
       }

       // Also output to console
       cout << "Tree content:\n";
       cout << bt;
}

void test_operator_output_chaining() {
       cout << "\n=== Test operator<< Chaining ===\n";
       using CharTrait = BTreeTrait<char, long>;
       BTree<CharTrait> bt(BTreeSize);

       const char* keys = "ABC";
       for (int i = 0; keys[i]; i++) {
               bt.Insert(keys[i], i);
       }

       cout << "Chained output: " << bt << "End of output\n";
}
//fin operator<<

//test write
void test_write_basic() {
       cout << "\n=== Test Write: Basic functionality ===" << endl;
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(3);

       // Insert some data
       bt.Insert(5, 100);
       bt.Insert(3, 200);
       bt.Insert(7, 300);
       bt.Insert(1, 400);
       bt.Insert(9, 500);

       // Write to file
       const char* filename = "btree_test_basic.dat";
       bool success = bt.Write(filename);

       cout << "Write operation: " << (success ? "SUCCESS" : "FAILED") << endl;
       cout << "File: " << filename << endl;
       cout << "Keys written: " << bt.size() << endl;
       cout << "Tree height: " << bt.height() << endl;
}

void test_write_empty_tree() {
       cout << "\n=== Test Write: Empty tree ===" << endl;
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(3);

       // Write empty tree
       const char* filename = "btree_test_empty.dat";
       bool success = bt.Write(filename);

       cout << "Write empty tree: " << (success ? "SUCCESS" : "FAILED") << endl;
       cout << "File: " << filename << endl;
       cout << "Keys written: " << bt.size() << endl;
}

void test_write_large_tree() {
       cout << "\n=== Test Write: Large tree ===" << endl;
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(5);

       // Insert many elements to create multi-level tree
       for (int i = 1; i <= 100; i++) {
              bt.Insert(i, i * 10);
       }

       const char* filename = "btree_test_large.dat";
       bool success = bt.Write(filename);

       cout << "Write large tree: " << (success ? "SUCCESS" : "FAILED") << endl;
       cout << "File: " << filename << endl;
       cout << "Keys written: " << bt.size() << endl;
       cout << "Tree height: " << bt.height() << endl;
}

void test_write_single_element() {
       cout << "\n=== Test Write: Single element ===" << endl;
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(3);

       bt.Insert(42, 999);

       const char* filename = "btree_test_single.dat";
       bool success = bt.Write(filename);

       cout << "Write single element: " << (success ? "SUCCESS" : "FAILED") << endl;
       cout << "File: " << filename << endl;
       cout << "Keys written: " << bt.size() << endl;
}

void test_write_non_unique() {
       cout << "\n=== Test Write: Non-unique BTree ===" << endl;
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(3, false);  // Non-unique

       // Insert duplicates
       bt.Insert(5, 100);
       bt.Insert(5, 200);
       bt.Insert(5, 300);
       bt.Insert(3, 400);
       bt.Insert(7, 500);

       const char* filename = "btree_test_nonunique.dat";
       bool success = bt.Write(filename);

       cout << "Write non-unique tree: " << (success ? "SUCCESS" : "FAILED") << endl;
       cout << "File: " << filename << endl;
       cout << "Keys written: " << bt.size() << endl;
}

void test_write_file_error() {
       cout << "\n=== Test Write: File error handling ===" << endl;
       using IntTrait = BTreeTrait<int, long>;
       BTree<IntTrait> bt(3);

       bt.Insert(1, 10);
       bt.Insert(2, 20);

       // Try to write to invalid path (should fail gracefully)
       const char* invalid_path = "C:\\invalid_path_that_does_not_exist\\btree.dat";
       bool success = bt.Write(invalid_path);

       cout << "Write to invalid path: " << (success ? "UNEXPECTED SUCCESS" : "FAILED (expected)") << endl;

       // Try to write to valid path
       const char* valid_path = "btree_test_valid.dat";
       success = bt.Write(valid_path);
       cout << "Write to valid path: " << (success ? "SUCCESS" : "FAILED") << endl;
}

// ============================================
// TESTS: Read (Carga) - Implementation #8
// ============================================

void test_read_basic() {
       cout << "\n=== Test Read: Basic functionality ===" << endl;
       using IntTrait = BTreeTrait<int, long>;

       // First, create and write a tree
       const char* filename = "btree_read_test_basic.dat";
       {
              BTree<IntTrait> btWrite(3);
              btWrite.Insert(5, 100);
              btWrite.Insert(3, 200);
              btWrite.Insert(7, 300);
              btWrite.Insert(1, 400);
              btWrite.Insert(9, 500);

              bool writeSuccess = btWrite.Write(filename);
              cout << "Write: " << (writeSuccess ? "SUCCESS" : "FAILED") << endl;
              cout << "Written keys: " << btWrite.size() << endl;
       }

       // Now read it back
       BTree<IntTrait> btRead(3);
       bool readSuccess = btRead.Read(filename);

       cout << "Read: " << (readSuccess ? "SUCCESS" : "FAILED") << endl;
       cout << "Read keys: " << btRead.size() << endl;
       cout << "Tree height: " << btRead.height() << endl;
}

void test_read_and_verify() {
       cout << "\n=== Test Read: Verify data integrity ===" << endl;
       using IntTrait = BTreeTrait<int, long>;

       const char* filename = "btree_read_verify.dat";
       int testData[] = {10, 20, 30, 40, 50, 60, 70, 80, 90};
       int dataSize = 9;

       // Write
       {
              BTree<IntTrait> btWrite(3);
              for (int i = 0; i < dataSize; i++) {
                     btWrite.Insert(testData[i], testData[i] * 10);
              }
              btWrite.Write(filename);
              cout << "Original tree size: " << btWrite.size() << endl;
       }

       // Read and verify
       BTree<IntTrait> btRead(3);
       btRead.Read(filename);

       cout << "Read tree size: " << btRead.size() << endl;
       cout << "Verifying data..." << endl;

       bool allCorrect = true;
       for (int i = 0; i < dataSize; i++) {
              long objID = btRead.Search(testData[i]);
              if (objID != testData[i] * 10) {
                     cout << "ERROR: Key " << testData[i] << " has wrong ObjID: "
                          << objID << " (expected " << (testData[i] * 10) << ")" << endl;
                     allCorrect = false;
              }
       }

       cout << "Data verification: " << (allCorrect ? "PASSED" : "FAILED") << endl;

       // Print the tree structure
       cout << "Tree structure:" << endl;
       btRead.Print(cout);
}

void test_read_large_tree() {
       cout << "\n=== Test Read: Large tree ===" << endl;
       using IntTrait = BTreeTrait<int, long>;

       const char* filename = "btree_read_large.dat";
       int numElements = 100;

       // Write large tree
       {
              BTree<IntTrait> btWrite(5);
              for (int i = 1; i <= numElements; i++) {
                     btWrite.Insert(i, i * 10);
              }
              btWrite.Write(filename);
              cout << "Written: " << btWrite.size() << " keys, height: "
                   << btWrite.height() << endl;
       }

       // Read large tree
       BTree<IntTrait> btRead(5);
       bool success = btRead.Read(filename);

       cout << "Read: " << (success ? "SUCCESS" : "FAILED") << endl;
       cout << "Read: " << btRead.size() << " keys, height: "
            << btRead.height() << endl;

       // Verify some random elements
       cout << "Verifying random elements..." << endl;
       bool verified = true;
       for (int key : {1, 25, 50, 75, 100}) {
              long objID = btRead.Search(key);
              if (objID != key * 10) {
                     cout << "ERROR: Key " << key << " verification failed" << endl;
                     verified = false;
              }
       }
       cout << "Verification: " << (verified ? "PASSED" : "FAILED") << endl;
}

void test_read_empty_tree() {
       cout << "\n=== Test Read: Empty tree ===" << endl;
       using IntTrait = BTreeTrait<int, long>;

       const char* filename = "btree_read_empty.dat";

       // Write empty tree
       {
              BTree<IntTrait> btWrite(3);
              btWrite.Write(filename);
              cout << "Written empty tree" << endl;
       }

       // Read empty tree
       BTree<IntTrait> btRead(3);
       bool success = btRead.Read(filename);

       cout << "Read: " << (success ? "SUCCESS" : "FAILED") << endl;
       cout << "Tree size: " << btRead.size() << endl;
       cout << "Tree height: " << btRead.height() << endl;
}

void test_read_invalid_file() {
       cout << "\n=== Test Read: Invalid file ===" << endl;
       using IntTrait = BTreeTrait<int, long>;

       BTree<IntTrait> bt(3);

       // Try to read non-existent file
       cout << "Reading non-existent file:" << endl;
       bool success1 = bt.Read("nonexistent_file_xyz.dat");
       cout << "Result: " << (success1 ? "UNEXPECTED SUCCESS" : "FAILED (expected)") << endl;

       // Create invalid file (wrong magic number)
       const char* invalidFile = "btree_invalid_magic.dat";
       {
              std::ofstream ofs(invalidFile, std::ios::binary);
              const char wrongMagic[6] = "WRONG";
              ofs.write(wrongMagic, 5);
              ofs.close();
       }

       cout << "Reading file with wrong magic number:" << endl;
       bool success2 = bt.Read(invalidFile);
       cout << "Result: " << (success2 ? "UNEXPECTED SUCCESS" : "FAILED (expected)") << endl;
}

void test_read_order_mismatch() {
       cout << "\n=== Test Read: Order mismatch ===" << endl;
       using IntTrait = BTreeTrait<int, long>;

       const char* filename = "btree_order_mismatch.dat";

       // Write with order=3
       {
              BTree<IntTrait> btWrite(3);
              btWrite.Insert(5, 100);
              btWrite.Insert(3, 200);
              btWrite.Insert(7, 300);
              btWrite.Write(filename);
              cout << "Written tree with order=3" << endl;
       }

       // Try to read with different order
       BTree<IntTrait> btRead(5);  // Different order!
       cout << "Attempting to read with order=5:" << endl;
       bool success = btRead.Read(filename);
       cout << "Result: " << (success ? "UNEXPECTED SUCCESS" : "FAILED (expected)") << endl;
}

void test_write_read_cycle() {
       cout << "\n=== Test Read: Write-Read cycle ===" << endl;
       using IntTrait = BTreeTrait<int, long>;

       const char* filename = "btree_cycle.dat";

       // Original tree
       BTree<IntTrait> bt1(3);
       bt1.Insert(15, 150);
       bt1.Insert(10, 100);
       bt1.Insert(20, 200);
       bt1.Insert(5, 50);
       bt1.Insert(12, 120);
       bt1.Insert(18, 180);
       bt1.Insert(25, 250);

       size_t originalSize = bt1.size();
       size_t originalHeight = bt1.height();

       cout << "Original tree - Size: " << originalSize
            << ", Height: " << originalHeight << endl;

       // Write
       bt1.Write(filename);

       // Read into new tree
       BTree<IntTrait> bt2(3);
       bt2.Read(filename);

       cout << "After read - Size: " << bt2.size()
            << ", Height: " << bt2.height() << endl;

       // Verify
       bool sizeMatch = (bt2.size() == originalSize);
       bool heightMatch = (bt2.height() == originalHeight);

       cout << "Size match: " << (sizeMatch ? "YES" : "NO") << endl;
       cout << "Height match: " << (heightMatch ? "YES" : "NO") << endl;

       // Verify all keys
       int keys[] = {15, 10, 20, 5, 12, 18, 25};
       bool allKeysFound = true;
       for (int key : keys) {
              long objID = bt2.Search(key);
              if (objID != key * 10) {
                     cout << "ERROR: Key " << key << " not found or incorrect" << endl;
                     allKeysFound = false;
              }
       }

       cout << "All keys found: " << (allKeysFound ? "YES" : "NO") << endl;
       cout << "Overall: " << ((sizeMatch && heightMatch && allKeysFound) ? "PASSED" : "FAILED") << endl;
}

int main (int argc, char * argv){
       test_default_comparison();
       test_descending_comparison();
       test_integer_comparison();
       test_search();
       test_remove();

       test_move_constructor();
       test_move_assignment();
       test_return_value_optimization();

       test_generic_foreach();
       test_generic_foreach_functor();
       test_generic_firstthat();
       test_generic_foreach_multiparams();
       test_oldstyle_vs_generic();

       test_iterator_basic();
       test_iterator_empty_tree();
       test_iterator_single_element();
       test_iterator_with_stl_algorithms();
       test_iterator_large_tree();
       test_iterator_modification();
       test_iterator_comparison();

       test_reverse_iterator_basic();
       test_reverse_iterator_decrement();
       test_reverse_iterator_empty_tree();
       test_reverse_iterator_single_element();
       test_reverse_iterator_with_stl_algorithms();
       test_bidirectional_navigation();
       test_reverse_iterator_modification();
       test_reverse_iterator_large_tree();

       test_operator_output_basic();
       test_operator_output_empty_tree();
       test_operator_output_large_tree();
       test_operator_output_with_metadata();
       test_operator_output_to_file();
       test_operator_output_chaining();

       test_write_basic();
       test_write_empty_tree();
       test_write_large_tree();
       test_write_single_element();
       test_write_non_unique();
       test_write_file_error();

       test_read_basic();
       test_read_and_verify();
       test_read_large_tree();
       test_read_empty_tree();
       test_read_invalid_file();
       test_read_order_mismatch();
       test_write_read_cycle();

       return 0;
}


// OLD CODE - kept for reference
/*int main_old (int argc, char * argv){
       int result, i;
       using CharTrait = BTreeTrait<char, long>;
       BTree<CharTrait> bt (BTreeSize);
       for (i = 0; keys1[i]; i++)
       {
               //cout<<"Inserting "<<keys1[i]<<endl;
               result = bt.Insert(keys1[i], i*i);
               //bt.Print(cout);
       }
       bt.Print(cout);
       /*for (i = 0; keys2[i]; i++)
       {
               cout << "Searching " << keys2[i] << " ";
               long ObjID = bt.Search(keys2[i]);
               if( ObjID != -1 )
                       cout << "Achei " << keys2[i] << " ID = " << ObjID << endl;
               else
                       cout <<"Nao achei!" << keys2[i] << endl;
       }*/
       /*cout.flush();

       for (i = 0; keys3[i]; i++)
       {
               cout << "Removing " << keys3[i] << " ";
               if( bt.Remove(keys3[i], -1) )
                       cout << keys3[i] << " removido !" << endl;
               else
                       cout <<"Nao achei!" << keys3[i] << endl;
               bt.Print(cout);
       }
       bt.Print(cout);
       cout.flush();*/
       return 1;
}









/*const char * keys="CDAMPIWNBKEHOLJYQZFXVRTSGU";
const char * keys2="CDAMPIWNBKEHOLJYQZFXVRTSGU";
const int BTreeSize = 3;
main (int argc, char * argv)
{
       //__int64 li;
       BTree <__int64> bt (BTreeSize);
       for (register int i = 0; i < 1000000; i++)
       {
               //cout<<"Inserting "<<keys[i]<<endl;
               bt.Insert(i, i-1);
               //bt.Print(cout);
       }

       for (i = 0; i < 1000; i++)
       {
               __int64 key = 975000+(::rand()%50000);
               //cout << "Searching " << (long)key << " ";
               long ObjID = bt.Search(key);
               if( ObjID != -1 )
                       cout << "Achei " << (long)key << " ID = " << ObjID << endl;
               else
                       cout <<"  Nao achei!" << (long)key << endl;
       }
       cout.flush();

       return 1;
}*/



/*const int BTreeSize = 3;
main (int argc, char * argv)
{
       int result, i;
       BTree <LONGLONG> bt(BTreeSize);
       result = bt.Create ("ernesto3-string-btree-start.dat",ios::in|ios::out);
       if (!result) { cout<<"Please delete testbt.dat"<<endl;return 0; }
       srand( (unsigned)time( NULL ) );
       LARGE_INTEGER key;
       for (i = 0; i < 1000000; i++)
       {
               //cout<<"Inserting "<<keys[i]<<endl;
               char strTmp[50];
               key.LowPart = rand();
               key.HighPart = rand();
               std::string str(strTmp);
               result = bt.Insert(key.QuadPart, i);
               //bt.Print(cout);
               if( i % 100000 == 0 )
               {       cout << i << endl; cout.flush();        }
       }
       //cout << "Searching D " << bt.Search();
       //bt.Search(1,1);
       cout.flush();
       return 1;
}*/
