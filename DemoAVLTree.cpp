#include <iostream>
#include "binarytree.h"
#include "avl.h"
using namespace std;


void DemoAVLTree(){
    // Arbol no balanceado
    cout << "\nArbol no balanceado:" << endl;
    CBinaryTree<BinaryTreeAscTraits<int>> bst;
    bst.insert(10, 1);
    bst.insert(20, 2);
    bst.insert(30, 3); // This creates a right-heavy tree

    cout << "Probando inorder en BST:" << endl;
    bst.inorder_print(cout);

    cout << "\nProbando print en BST:" << endl;
    bst.print(cout);

    // Arbol balanceado
    cout << "\nArbol balanceado (AVL):" << endl;
    CAVLTree<AVLAscTraits<int>> avl;
    avl.insert(10, 1);
    avl.insert(20, 2);  
    avl.insert(30, 3); // This should trigger a left rotation
    

    cout << "Probando inorder en AVL:" << endl;
    avl.inorder_print(cout);

    cout << "\nProbando print en AVL:" << endl;
    avl.print(cout);
}