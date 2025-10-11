#include <iostream>
#include <fstream>
#include <vector>
#include <utility> // para std::pair
#include "linkedlist.h"
#include "doublelinkedlist.h"
#include "foreach.h"
#include "types.h"
#include "util.h"

void opex(int &n){ n++; }

void DemoLinkedList(){
    std::vector< std::pair<T1, Ref> > v1 = {
        {4, 8}, {2, 5}, {7, 3}, {1, 9}, {5, 2}
    };
    CLinkedList< AscendingTrait<T1> > l1;
    for (auto &par : v1)
        l1.Insert(par.first, par.second);
    std::cout << l1 << std::endl;

    std::vector< std::pair<T2, Ref> > v2 = {
        {4.5, 8}, {2.3, 5}, {7.8, 3}, {1.1, 9}, {5.7, 2}
    };
    CLinkedList< DescendingTrait<T2> > l2;
    // Insertar desde el vector
    for (auto &par : v2) {
        l2.Insert(par.first, par.second);
    }
    std::cout << l2 << std::endl;

    std::cout << "Ahora utilizando foreach #1..." << std::endl;
    foreach(l1, ::opex);
    std::cout << "Imprimiendo: l1 (debe haber aumentado en 1) ..." << std::endl;
    foreach(l1, ::Print<T1>);
    cout <<endl;
    std::cout << "l1 aplicando funcion lambda +2 ..." << std::endl;
    foreach(l1, [](T1 &n){ n += 2;  } );
    std::cout << "Imprimiendo: l1 (debe haber aumentado en 1) ..." << std::endl;
    foreach(l1, ::Print<T1>);

    std::cout << "Imprimiendo: l1 a través de begin() y end() ..." << std::endl;
    foreach(l1.begin(), l1.end(), ::Print<T1>);
    cout <<endl;

    std::cout << "Imprimiendo l1 con Write ..." << std::endl;
    l1.Write(cout);
    cout <<endl;

    std::ofstream of("LL.txt");
    l1.Write(of);
    of.close();
}

void DemoDoubleLinkedList(){
    std::vector< std::pair<T1, Ref> > v1 = {
        {4, 8}, {2, 5}, {7, 3}, {1, 9}, {5, 2}
    };
    CDoubleLinkedList< AscendingTrait<T1> > l1;
    for (auto &par : v1)
        l1.Insert(par.first, par.second);
    std::cout << "Lista ascendente ..." << std::endl;
    std::cout << l1 << std::endl;

    CDoubleLinkedList< DescendingTrait<T1> > l2;
    for (auto &par : v1)
        l2.Insert(par.first, par.second);
    std::cout << "Lista descendente ..." << std::endl;
    std::cout << l2 << std::endl;
    // std::cout << " Imprimiendo DoubleLinkedList ..." << std::endl;
    // foreach(l1. begin(), l1. end(), ::Print<T1>);

    // foreach(l1.rbegin(), l1.rend(), ::Print<T1>);

    std::ofstream of("DLL.txt");
    l1.Write(of);
    of.close();

    CDoubleLinkedList< AscendingTrait<T1> > list2;
    std::ifstream inFile("DLL.txt");
    list2.Read(inFile);
    inFile.close();
    std::cout << " Leyendo DoubleLinkedList desde archivo ..." << std::endl;
    std::cout << "Imprimiendo list2 ..." << std::endl;
    std::cout << list2 << std::endl;
    cout << endl;

    std::cout << "Probando move constructor ..." << std::endl;
    CDoubleLinkedList< AscendingTrait<T1> > list3 = std::move(list2);
    std::cout << "Imprimiendo list3 (debe tener los elementos) " << std::endl;
    std::cout << list3 << std::endl;
    std::cout << "Imprimiendo list2 (debe estar vacia) " << std::endl;
    std::cout << list2 << std::endl;

    std::cout<< "Probando copy constructor ..." << std::endl;
    CDoubleLinkedList< AscendingTrait<T1> > list4 = list3;
    std::cout << "Imprimiendo list4 (debe tener los elementos) " << std::endl;
    std::cout << list4 << std::endl;
    std::cout << "Imprimiendo list3 (debe tener los elementos) " << std::endl;
    std::cout << list3 << std::endl;
    
    std::cout << "Probando iterators ... "   << std::endl;
    CDoubleLinkedList<AscendingTrait<int>> list;
    
    // Insertar elementos
    int val1 = 1, val2 = 2, val3 = 3;
    list.Insert(val1, 1);
    list.Insert(val2, 2);
    list.Insert(val3, 3);
    
    std::cout << "Forward: ";
    for(auto it = list.begin(); it != list.end(); ++it)
        std::cout << *it << " ";  // Esperado: 1 2 3
    std::cout << std::endl;
    
    std::cout << "Backward: ";
    for(auto it = list.rbegin(); it != list.rend(); ++it)
        std::cout << *it << " ";  // Esperado: 3 2 1
    std::cout << std::endl;


}

