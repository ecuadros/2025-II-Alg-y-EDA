#include <iostream>
#include "btree.h"
#include <functional>
#include <thread>
#include <vector>
#include <chrono>
using namespace std;

template <typename _keyType, typename _ObjIDType>
struct DemoTraits {
    using keyType = _keyType;
    using ObjIDType = _ObjIDType;
    
    static bool compare(const keyType& k1, const keyType& k2) {
        return k1 < k2;
    }
};

void DemoBTree() {
    // Create a BTree of integers with default order=3 (minimum degree)
    BTree<DemoTraits<int, long>> tree(3);
    
    // Insert some values
    cout << "Inserting values..." << endl;
    tree.Insert(10, 100);
    tree.Insert(20, 200);
    tree.Insert(5, 50);
    tree.Insert(15, 150);
    tree.Insert(25, 250);
    
    // Test Search functionality
    cout << "\nTesting Search..." << endl;
    long objId = tree.Search(15);
    cout << "Search for key 15 returned ObjID: " << objId << endl;
    
    // Test FirstThat with a lambda function that returns bool
    cout << "\nTesting FirstThat with lambda..." << endl;
    auto result = tree.FirstThat([](const auto& info, size_t level) -> bool {
        // Find the first element greater than 15
        return info.key > 15;
    });
    
    if (result) {
        cout << "Found first element greater than 15: key = " << result->key 
             << ", ObjID = " << result->ObjID << endl;
    } else {
        cout << "No element found greater than 15" << endl;
    }
    
    // Test file I/O operations
    cout << "\nTesting Write/Read to/from file..." << endl;
    
    // Write tree to file
    if (!tree.Write("test_btree.dat")) {
        cout << "Failed to write tree to file" << endl;
        return;
    }
    cout << "Tree written to file successfully" << endl;

    // Create a new tree and read from file
    BTree<DemoTraits<int, long>> readTree(3);
    if (!readTree.Read("test_btree.dat")) {
        cout << "Failed to read tree from file" << endl;
        return;
    }
    cout << "Tree read from file successfully" << endl;

    // Verify the read tree by searching for a known value
    objId = readTree.Search(15);
    cout << "Search in read tree for key 15 returned ObjID: " << objId << endl;

    // Test Move Constructor
    cout << "\nTesting Move Constructor..." << endl;
    BTree<DemoTraits<int, long>> movedTree(std::move(tree));
    
    // Verify the moved tree works correctly
    objId = movedTree.Search(20);
    cout << "Search in moved tree for key 20 returned ObjID: " << objId << endl;
    
    // Original tree should be empty or in valid state after move
    objId = tree.Search(20);
    if (objId == -1) {
        cout << "Original tree is empty after move (as expected)" << endl;
    }
    
    cout << "\nAll tests completed!" << endl;
    if (tree.Write("btree_test.dat")) {
        cout << "Successfully wrote tree to file" << endl;
    } else {
        cout << "Failed to write tree to file" << endl;
    }
    
    // Create a new tree and read from file
    BTree<DemoTraits<int, long>> tree2(3);
    if (tree2.Read("btree_test.dat")) {
        cout << "Successfully read tree from file" << endl;
        
        // Verify the read tree by searching for a known value
        long objId2 = tree2.Search(15);
        cout << "Search in read tree for key 15 returned ObjID: " << objId2 << endl;
    } else {
        cout << "Failed to read tree from file" << endl;
    }
    
    // Test Remove functionality
    cout << "\nTesting Remove..." << endl;
    if (tree.Remove(15, 150)) {
        cout << "Successfully removed key 15" << endl;
        
        // Verify removal
        long objId3 = tree.Search(15);
        cout << "Search for removed key 15 returned ObjID: " << objId3 << endl;
    } else {
        cout << "Failed to remove key 15" << endl;
    }
    
    // Print final tree state
    cout << "\nFinal tree state:" << endl;
    cout << "Tree size: " << tree.size() << endl;
    cout << "Tree height: " << tree.height() << endl;

    // Test concurrency
    cout << "\nTesting concurrency..." << endl;
    
    // Crear un nuevo árbol para las pruebas de concurrencia
    BTree<DemoTraits<int, long>> concurrentTree(3);
    
    // Vector para almacenar todos los threads
    vector<thread> threads;
    
    // Crear thread de escritura
    cout << "Starting writer thread..." << endl;
    threads.emplace_back([&concurrentTree]() {
        for(int i = 0; i < 100; i++) {
            concurrentTree.Insert(i, i * 10);
            this_thread::sleep_for(chrono::milliseconds(1));  // Pequeña pausa para simular trabajo
        }
    });
    
    // Crear 5 threads de lectura
    cout << "Starting 5 reader threads..." << endl;
    for(int i = 0; i < 5; i++) {
        threads.emplace_back([&concurrentTree, i]() {
            for(int j = 0; j < 100; j++) {
                long result = concurrentTree.Search(j);
                if(result != -1) {
                    cout << "Reader " << i << " found key " << j << " with value " << result << endl;
                }
                this_thread::sleep_for(chrono::milliseconds(2));  // Pequeña pausa para simular trabajo
            }
        });
    }
    
    // Esperar a que todos los threads terminen
    cout << "Waiting for all threads to finish..." << endl;
    for(auto& t : threads) {
        t.join();
    }
    
    // Verificar el estado final del árbol concurrente
    cout << "\nConcurrent tree final state:" << endl;
    cout << "Tree size: " << concurrentTree.size() << endl;
    cout << "Tree height: " << concurrentTree.height() << endl;
    
    // Verificar algunos valores
    cout << "\nVerifying some values in the concurrent tree:" << endl;
    for(int i = 0; i < 100; i += 10) {
        long value = concurrentTree.Search(i);
        cout << "Key " << i << " has value: " << value << endl;
    }
}