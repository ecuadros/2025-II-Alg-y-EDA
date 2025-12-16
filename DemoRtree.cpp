
#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include "rtree.h"

using namespace std;

using TestTrait = RTreeTrait<double, std::string>;
using TestTree = RTree<TestTrait>;



void DemoRTree() {
    
    // Insert y Print
    TestTree tree;
    tree.Insert(0, 0, 2, 2, "A");
    tree.Insert(1, 1, 3, 3, "B");
    tree.Insert(5, 5, 7, 7, "C");
    
    std::cout << "Tree después de inserts:" << std::endl;
    tree.Print();
    std::cout << "Size: " << tree.size() << std::endl;
    
    // Range Query
    std::cout << "\nRange Query (0, 0, 10, 10):" << std::endl;
    auto results = tree.RangeQuery(0, 0, 10, 10);
    std::cout << "Encontrados: " << results.size() << " elementos" << std::endl;
    for (const auto& result : results) {
        std::cout << "- " << result.second << std::endl;
    }
    
    //  ForEach
    std::cout << "\nTest ForEach:" << std::endl;
    tree.ForEach([](const auto& rect, const auto& data) {
        std::cout << "- " << data << " (área: " << rect.Area() << ")" << std::endl;
    });
    
    // PrintWith
    std::cout << "\nTest PrintWith:" << std::endl;
    tree.PrintWith([](const auto& rect, const std::string& data) {
        std::cout << data << " en (" << rect.xMin << "," << rect.yMin 
                  << ")-(" << rect.xMax << "," << rect.yMax << ")" << std::endl;
    });
    

    // File I/O
    std::cout << "\nTest File I/O:" << std::endl;
    bool written = tree.Write("test_rtree.txt");
    std::cout << "Write: " << (written ? "yes" : "no") << std::endl;
    
    TestTree tree2;
    bool read = tree2.Read("test_rtree.txt");
    std::cout << "Read: " << (read ? "yes" : "no") << std::endl;
    
    if (read) {
        std::cout << "Tree cargado:" << std::endl;
        tree2.Print();
        std::cout << "Size cargado: " << tree2.size() << std::endl;
    }
    
    // Delete
    std::cout << "\nTest Delete:" << std::endl;
    bool removed = tree.Remove("Tienda");
    std::cout << "Eliminación de 'Tienda': " << (removed ? "yes" : "no") << std::endl;
    tree.Print();
    std::cout << "Size después de delete: " << tree.size() << std::endl;
    
    
	
}