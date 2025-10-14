#include <iostream>
#include <fstream>
#include <vector>
#include <utility> // para std::pair
#include <thread>

//#include "linkedlist.h"
#include "doublelinkedlist.h"
#include "binarytree.h"
#include "foreach.h"
#include "types.h"
#include "util.h"

void opex(int &n){ n++; }

template <typename T>
void PrintX(const T &val, ostream &os){ os << val << " "; }

template <typename T>
void PrintY(T val, T value1, T value2, ostream &os){ 
    val += value1 + value2; 
    os << val << " "; 
}

void threadInsert(CDoubleLinkedList< AscendingTrait<T1> > &list, 
                  std::vector< std::pair<T1, Ref> > &data) {
    for (auto &par : data) {
        std::this_thread::sleep_for(std::chrono::milliseconds(par.first));
        list.Insert(par.first, par.second);
    }
    cout << list << std::endl;
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

    CDoubleLinkedList< AscendingTrait<T1> > l2 = CDoubleLinkedList< AscendingTrait<T1> >(l1)    ;
    std::cout << "Lista Copiada: " << l2 << std::endl;
    //std::cin >> l1;
    //std::cout << l1 << std::endl;

    std::vector<std::pair<T1, Ref>> v2 = {
        {15, 42}, {88, 19}, {37, 73}, {6, 55}, {29, 11}
    };

    std::vector<std::pair<T1, Ref>> v3 = {
        {54, 23}, {91, 67}, {12, 31}, {78, 49}, {33, 8}
    };

    std::cout << "Iniciando insercion concurrente" << std::endl;
    std::thread t1(threadInsert, std::ref(l2), std::ref(v2));
    std::thread t2(threadInsert, std::ref(l2), std::ref(v3));

    t1.join(); t2.join();



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
    std::vector< std::pair<T1, Ref> > v1 = {
       {4, 1}, {2, 2}, {6, 3}, {1, 4}, {3, 5}, {5, 6}, {7, 7}, {8,8}
    };
    CBinaryTree< BinaryTreeAscTraits<T1> > bt;
    for (auto &par : v1)
        bt.insert(par.first, par.second);
    std::cout << bt << std::endl;

    CBinaryTree< BinaryTreeAscTraits<T1> > bt2 = CBinaryTree< BinaryTreeAscTraits<T1> >(bt);
    std::cout << "Binary Tree Copied:\n" << bt2 << std::endl;
/*    std::cout << "Inorder traversal:" << std::endl;
    //bt.inorder();
    std::cout << std::endl;

    std::cout << "Preorder traversal:" << std::endl;
    // bt.preorder();
    std::cout << std::endl;

    std::cout << "Postorder traversal:" << std::endl;
    // bt.postorder();
    std::cout << std::endl;
*/
    std::cout << "Tree structure:" << std::endl;
    bt.print(cout);
    std::cout << std::endl;

    std::cout << "Imprimiendo con forward iterator" << std::endl;
    // foreach(bt. begin(), bt. end(), ::Print<T1>);
    std::cout << std::endl;

    std::cout << "Imprimiendo con backward iterator" << std::endl;
    foreach(bt.begin(), bt.end(), ::Print<T1>);
    std::cout << std::endl;
    
    std::ofstream of("BT.txt");
    bt.Write(of);
    of.close();

    // Next classes: AVL, BTree*/
}