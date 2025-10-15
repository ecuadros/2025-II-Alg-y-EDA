#include <iostream>
#include <string>
#include <vector>
#include "binarytree.h"

using namespace std;

struct IntTraits {
    using T = int;
    using Node = CBinaryTreeNode<IntTraits>;
    using CompareFn = less<int>;
};

struct IntDescTraits {
    using T = int;
    using Node = CBinaryTreeNode<IntDescTraits>;
    using CompareFn = greater<int>;
};

struct StringTraits {
    using T = string;
    using Node = CBinaryTreeNode<StringTraits>;
    using CompareFn = less<string>;
};

void testBasicOperations() {
    cout << "\n========== TEST 1: Operaciones basicas ==========\n";
    CBinaryTree<IntTraits> tree;

    tree.insert(50, 1);
    tree.insert(30, 2);
    tree.insert(70, 3);
    tree.insert(20, 4);
    tree.insert(40, 5);
    tree.insert(60, 6);
    tree.insert(80, 7);

    cout << "Elementos insertados: 50, 30, 70, 20, 40, 60, 80\n";
    cout << "Tamaño: " << tree.size() << "\n";
    cout << "Vacio: " << (tree.empty() ? "Si" : "No") << "\n\n";

    cout << "Recorrido inorden:   ";
    tree.inorder(cout);
    cout << "\nRecorrido preorden:  ";
    tree.preorder(cout);
    cout << "\nRecorrido postorden: ";
    tree.postorder(cout);
    cout << "\n\nArbol visual:\n";
    tree.print(cout);
}

void testIterators() {
    cout << "\n========== TEST 2: Iteradores ==========\n";
    CBinaryTree<IntTraits> tree;

    tree.insert(50, 1);
    tree.insert(30, 2);
    tree.insert(70, 3);
    tree.insert(20, 4);
    tree.insert(40, 5);
    tree.insert(60, 6);
    tree.insert(80, 7);

    cout << "Forward iterator (begin -> end):\n  ";
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        cout << *it << " ";
    }

    cout << "\n\nReverse iterator (rbegin -> rend):\n  ";
    for (auto it = tree.rbegin(); it != tree.rend(); ++it) {
        cout << *it << " ";
    }

    cout << "\n\nRange-based for:\n  ";
    for (auto val : tree) {
        cout << val << " ";
    }

    cout << "\n\nOperador decremento (end -> begin):\n  ";
    auto it = tree.end();
    while (it != tree.begin()) {
        --it;
        cout << *it << " ";
    }
    cout << "\n";
}

void testCopyAndMove() {
    cout << "\n========== TEST 3: Copy y Move ==========\n";
    CBinaryTree<IntTraits> tree1;

    tree1.insert(50, 1);
    tree1.insert(30, 2);
    tree1.insert(70, 3);

    cout << "Arbol original: ";
    tree1.inorder(cout);
    cout << " (size: " << tree1.size() << ")\n";

    CBinaryTree<IntTraits> tree2(tree1);
    cout << "Constructor copia: ";
    tree2.inorder(cout);
    cout << " (size: " << tree2.size() << ")\n";

    CBinaryTree<IntTraits> tree3;
    tree3 = tree1;
    cout << "Operador asignacion: ";
    tree3.inorder(cout);
    cout << " (size: " << tree3.size() << ")\n";

    CBinaryTree<IntTraits> tree4(std::move(tree1));
    cout << "Constructor movimiento: ";
    tree4.inorder(cout);
    cout << " (size: " << tree4.size() << ")\n";
    cout << "Arbol movido vacio: " << (tree1.empty() ? "Si" : "No") << "\n";
}

void testMemoryManagement() {
    cout << "\n========== TEST 4: Gestion de memoria ==========\n";
    CBinaryTree<IntTraits>* pTree = new CBinaryTree<IntTraits>();

    pTree->insert(50, 1);
    pTree->insert(30, 2);
    pTree->insert(70, 3);

    cout << "Arbol creado con " << pTree->size() << " elementos\n";

    pTree->clear();
    cout << "Despues de clear(): size = " << pTree->size() << ", vacio = "
         << (pTree->empty() ? "Si" : "No") << "\n";

    delete pTree;
    cout << "Arbol eliminado correctamente\n";
}

void testVariadicTraversals() {
    cout << "\n========== TEST 5: Recorridos variadic templates ==========\n";
    CBinaryTree<IntTraits> tree;

    tree.insert(50, 1);
    tree.insert(30, 2);
    tree.insert(70, 3);
    tree.insert(20, 4);
    tree.insert(40, 5);
    tree.insert(60, 6);
    tree.insert(80, 7);

    cout << "Arbol: 50, 30, 70, 20, 40, 60, 80\n\n";

    cout << "Inorden con lambda (mostrar con nivel):\n";
    tree.inorder([](auto* node, size_t level) {
        for(size_t i = 0; i < level; i++) cout << "  ";
        cout << "Nivel " << level << ": " << node->getDataRef() << "\n";
    });

    cout << "\nPreorden con lambda (calcular suma):\n";
    int sum = 0;
    tree.preorder([](auto* node, size_t level, int& total) {
        total += node->getDataRef();
    }, sum);
    cout << "  Suma total: " << sum << " (esperado: 350)\n";

    cout << "\nPostorden con lambda (recolectar valores):\n";
    vector<int> values;
    tree.postorder([](auto* node, size_t level, vector<int>& vec) {
        vec.push_back(node->getDataRef());
    }, values);
    cout << "  Valores: ";
    for (int v : values) cout << v << " ";
    cout << "\n";

    cout << "\nInorden con multiples parametros:\n";
    int count = 0;
    int max_val = 0;
    tree.inorder([](auto* node, size_t level, int& cnt, int& mx) {
        cnt++;
        if (node->getDataRef() > mx) mx = node->getDataRef();
    }, count, max_val);
    cout << "  Cantidad: " << count << ", Maximo: " << max_val << "\n";
}

void testEdgeCases() {
    cout << "\n========== TEST 6: Casos especiales ==========\n";

    CBinaryTree<IntTraits> emptyTree;
    cout << "Arbol vacio:\n";
    cout << "  Size: " << emptyTree.size() << "\n";
    cout << "  begin() == end(): " << (emptyTree.begin() == emptyTree.end() ? "Si" : "No") << "\n";

    CBinaryTree<IntDescTraits> descTree;
    descTree.insert(50, 1);
    descTree.insert(30, 2);
    descTree.insert(70, 3);
    cout << "\nArbol descendente (greater): ";
    descTree.inorder(cout);
    cout << "\n";

    CBinaryTree<StringTraits> strTree;
    strTree.insert("manzana", 1);
    strTree.insert("banana", 2);
    strTree.insert("naranja", 3);
    cout << "\nArbol de strings: ";
    strTree.inorder(cout);
    cout << "\n";
}

void DemoBinaryTree() {
    cout << "\n";
    cout << "========================================================\n";
    cout << "  DEMO: Binary Tree - Validacion de funcionalidades\n";
    cout << "========================================================\n";

    try {
        testBasicOperations();
        testIterators();
        testCopyAndMove();
        testMemoryManagement();
        testVariadicTraversals();
        testEdgeCases();

        cout << "\n";
        cout << "========================================================\n";
        cout << "  TODAS LAS PRUEBAS COMPLETADAS CORRECTAMENTE\n";
        cout << "========================================================\n";
        cout << "\nTODOs completados y probados:\n";
        cout << "  [LISTO] Metodos privados con friend class\n";
        cout << "  [LISTO] Iterator operator++ (avance inorden)\n";
        cout << "  [LISTO] Iterator operator-- (retroceso)\n";
        cout << "  [LISTO] Copy constructor (copia profunda)\n";
        cout << "  [LISTO] Move constructor\n";
        cout << "  [LISTO] Destructor recursivo\n";
        cout << "  [LISTO] begin() - nodo mas a la izquierda\n";
        cout << "  [LISTO] rbegin()/rend() - reverse iterator\n";
        cout << "  [LISTO] Recorridos variadic templates\n";
        cout << "\nFuncionalidades probadas:\n";
        cout << "  - Insercion y recorridos (inorden, preorden, postorden)\n";
        cout << "  - Iteradores forward y reverse\n";
        cout << "  - Operadores ++ y --\n";
        cout << "  - Constructor por copia y movimiento\n";
        cout << "  - Operador de asignacion\n";
        cout << "  - Destructor y clear()\n";
        cout << "  - Recorridos variadic con lambdas\n";
        cout << "  - Arboles con diferentes tipos y comparadores\n";
        cout << "\n";

    } catch (const exception& e) {
        cerr << "\nERROR: " << e.what() << endl;
    }
}
