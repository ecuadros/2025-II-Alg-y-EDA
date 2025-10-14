#include <iostream>
#include "avl.h"

using namespace std;

int main() {
    // Test basico de compilacion
    CAVLTree<AVLAscTraits<int>> avl;
    
    avl.insert(80, 1);
    avl.insert(60, 2);
    avl.insert(100, 3);
    
    cout << "AVL con " << avl.size() << " elementos\n";
    avl.inorder(cout);
    cout << "\n";
    
    return 0;
}
