#include <iostream>
#include <fstream>
#include <vector>
#include <utility> // para std::pair

//#include "linkedlist.h"
#include "doublelinkedlist.h"
//#include "binarytree.h"
//#include "foreach.h"
#include "types.h"
#include "util.h"

void opex(int &n){ n++; }

template <typename T>
void PrintX(T &val, ostream &os){ os << val << " "; }

template <typename T>
void PrintY(T &val, T value1, T value2, ostream &os){ 
    val += value1 + value2; 
    os << val << " "; 
}



void DemoDoubleLinkedList(){
    std::vector< std::pair<T1, Ref> > v1 = {
        {4, 8}, {2, 5}, {7, 3}, {1, 9}, {5, 2}
    };

    std::cout << "Demo Double Linked List" << std::endl;
    CDoubleLinkedList< AscendingTrait<T1> > l1;
    for (auto &par : v1)
        l1.Insert(par.first, par.second);
    std::cout << l1 << std::endl;

    std::cin >> l1;
    std::cout << l1 << std::endl;
/*
    std::cout << "Imprimiendo con forward iterator" << std::endl;
    foreach(l1. begin(), l1. end(), ::Print<T1>);
    std::cout << std::endl;

    std::cout << "Imprimiendo con backward iterator" << std::endl;
    foreach(l1.rbegin(), l1.rend(), ::Print<T1>);
    std::cout << std::endl;

    l1.foreach(::PrintX<T1>,       std::cout);
    l1.foreach(::PrintY<T1>, 1, 3, std::cout);
    l1.foreach([](T1 &val, ostream &os){
        os << val << " "; 
    }, std::cout);
    
    std::ofstream of("DLL.txt");
    l1.foreach(::PrintY<T1>, 1, 3, of);
    of.close();
*/
    std::cout << std::endl;
}
/*
void DemoBinaryTree(){
    std::vector< std::pair<T1, Ref> > v1 = {
        {4, 8}, {2, 5}, {7, 3}, {1, 9}, {5, 2}
    };
    CBinaryTree< AscendingTrait<T1> > bt;
    for (auto &par : v1)
        bt.insert(par.first, par.second);
    std::cout << bt << std::endl;

    std::cout << "Inorder traversal:" << std::endl;
    // bt.inorder();
    std::cout << std::endl;

    std::cout << "Preorder traversal:" << std::endl;
    // bt.preorder();
    std::cout << std::endl;

    std::cout << "Postorder traversal:" << std::endl;
    // bt.postorder();
    std::cout << std::endl;

    std::cout << "Tree structure:" << std::endl;
    // bt.print();
    std::cout << std::endl;

    std::cout << "Imprimiendo con forward iterator" << std::endl;
    // foreach(bt. begin(), bt. end(), ::Print<T1>);
    std::cout << std::endl;

    std::cout << "Imprimiendo con backward iterator" << std::endl;
    // foreach(bt.rbegin(), bt.rend(), ::Print<T1>);
    std::cout << std::endl;
    
    std::ofstream of("BT.txt");
    bt.Write(of);
    of.close();

    // Next classes: AVL, BTree
}*/