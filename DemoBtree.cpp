#include <iostream>
#include <vector>
#include <cassert>
#include "DemoVector.h"
#include "btree.h"
#include "traits.h"

const int BTreeSize = 3;
const char * keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
const char * keys2 = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char * keys3 = "9876543210zyxwvutsrqponmlkjihgfedcba";

void printVector(std::vector<int>& v, std::ostream& os){
    for(int n : v) os << n << " ";
    os << "\n";
}

void DemoBtree(){
    /*std::vector<int> v = {1, 3, 5, 7, 9};
    std::cout << "vector inicial: ";
    printVector(v, std::cout);
    // Probar funciones
    int target = 3;
    size_t index = binary_search(v, 0, v.size() - 1, target);
    std::cout << "indice de "<<target<<" : " << index << "\n";

    insert_at(v, 6, 3);
    std::cout << "Vector despues de insert: ";
    printVector(v, std::cout);

    remove(v, 2); 
    std::cout << "Vector despues de remove: ";
    printVector(v, std::cout);
    */

    std::vector<int> v1 = {1, 3, 5, 7, 9};
    cout<< "\nDemo BTree\n";
    int result, i=2;
    BTree <BTreeDescTraits<char,long>> bt (BTreeSize);
    
    for(i=0; i<7; i++)
        result = bt.Insert(keys3[i], i*i);

}
