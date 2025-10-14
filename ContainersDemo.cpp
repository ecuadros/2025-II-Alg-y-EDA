#include <iostream>
#include <fstream>
#include <vector>
#include <utility> // para std::pair

#include "linkedlist.h"
#include "doublelinkedlist.h"
#include "binarytree.h"
#include "avl.h"
#include "foreach.h"
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

    std::cout << "Demo Double Linked List" << std::endl;

    CDoubleLinkedList< AscendingTrait<T1> > l1;
    for (auto &par : v1)
        l1.Insert(par.first, par.second);
    std::cout << l1 << std::endl;

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

    std::cout << std::endl;
}

void DemoBinaryTree(){
    // Use 8 elements to match expected demo output (order chosen to get 4 as root)
    std::vector< std::pair<T1, Ref> > v1 = {
        // {4, 8}, {2, 5}, {1, 2}, {3, 6}, {6, 4}, {5, 1}, {7, 3}, {8, 7}
        {4, 8}, {2, 5}, {7, 3}, {1, 9}, {5, 2}
    };
    
    CBinaryTree< BinaryTreeAscTraits<T1> > bt;
    for (auto &par : v1)
        bt.insert(par.first, par.second);
    
    std::cout << "Hello Alg y EDA-UNI" << std::endl;
    std::cout << bt << std::endl;

    std::cout << "Inorder traversal:" << std::endl;
    bt.inorder(std::cout);
    std::cout << std::endl;

    std::cout << "Preorder traversal:" << std::endl;
    bt.preorder(std::cout);
    std::cout << std::endl;

    std::cout << "Postorder traversal:" << std::endl;
    bt.postorder(std::cout);
    std::cout << std::endl;

    std::cout << "Tree structure:" << std::endl;
    bt.print(std::cout);
    std::cout << std::endl;

    std::cout << "Imprimiendo con forward iterator" << std::endl;
    for (auto it = bt.begin(); it != bt.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    std::cout << "Imprimiendo con backward iterator" << std::endl;
    for (auto it = bt.rbegin(); it != bt.rend(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    std::ofstream of("BT.txt");
    bt.Write(of);
    of.close();
}

void DemoAVL(){
    std::vector< std::pair<T1, Ref> > v = {
        {30, 0}, {20, 0}, {40, 0}, {10, 0}, {25, 0}, {5, 0}, {35, 0}, {50, 0}
    };

    std::cout << "Demo AVL Tree" << std::endl;
    CAVLTree<AVLAscTraits<T1>> at;
    for (auto &p : v) at.insert(p.first, p.second);

    std::cout << "AVL inorder traversal:" << std::endl;
    at.inorder(std::cout);
    std::cout << std::endl;

    std::cout << "AVL tree structure:" << std::endl;
    at.print(std::cout);
    std::cout << std::endl;

    std::cout << "Imprimiendo con forward iterator" << std::endl;
    for (auto it = at.begin(); it != at.end(); ++it) std::cout << *it << " ";
    std::cout << std::endl;
}