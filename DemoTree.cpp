#include <iostream>
#include "binarytree.h"
using namespace std;

template <typename _T>
struct GeneralTraits{
    using value_type = _T;
    
};

void DemoBinaryTree(){
    CBinaryTree<BinaryTreeAscTraits<int>> arbol;
    arbol.insert(5, 1);
    arbol.insert(3, 2);  
    arbol.insert(7, 3);

    cout << "Probando inorder:" << endl;
    arbol.inorder_print(cout);

    cout << "\nProbando preorder:" << endl;
    arbol.preorder_print(cout);

    cout << "\nProbando postorder:" << endl;
    arbol.postorder_print(cout);

    cout << "\nProbando print:" << endl;
    arbol.print(cout);
}