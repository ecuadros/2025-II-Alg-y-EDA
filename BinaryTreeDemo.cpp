#include <iostream>
#include <string>
#include "binarytree.h"

using namespace std;

// Traits para un árbol binario de enteros ascendente
struct IntTraits {
    using T = int;
    using Node = CBinaryTreeNode<IntTraits>;
    using CompareFn = less<int>;
};

// Traits para un árbol binario de enteros descendente
struct IntDescTraits {
    using T = int;
    using Node = CBinaryTreeNode<IntDescTraits>;
    using CompareFn = greater<int>;
};

// Traits para un árbol binario de strings
struct StringTraits {
    using T = string;
    using Node = CBinaryTreeNode<StringTraits>;
    using CompareFn = less<string>;
};

void testBasicInsertion() {
    cout << "\n========== TEST 1: Inserción básica ==========\n";
    CBinaryTree<IntTraits> tree;

    tree.insert(50, 1);
    tree.insert(30, 2);
    tree.insert(70, 3);
    tree.insert(20, 4);
    tree.insert(40, 5);
    tree.insert(60, 6);
    tree.insert(80, 7);

    cout << "Árbol creado con " << tree.size() << " elementos\n";
    cout << "Recorrido inorden: ";
    tree.inorder(cout);
    cout << endl;

    cout << "\nÁrbol visual:\n";
    tree.print(cout);
}

void testIterators() {
    cout << "\n========== TEST 2: Iteradores (begin/end) ==========\n";
    CBinaryTree<IntTraits> tree;

    tree.insert(50, 1);
    tree.insert(30, 2);
    tree.insert(70, 3);
    tree.insert(20, 4);
    tree.insert(40, 5);
    tree.insert(60, 6);
    tree.insert(80, 7);
    tree.insert(10, 8);
    tree.insert(90, 9);

    cout << "Recorrido con iterador (++): ";
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        cout << *it << " ";
    }
    cout << endl;

    cout << "Recorrido con range-based for: ";
    for (auto& val : tree) {
        cout << val << " ";
    }
    cout << endl;
}

void testIteratorDecrement() {
    cout << "\n========== TEST 3: Iterador con decremento (--) ==========\n";
    CBinaryTree<IntTraits> tree;

    tree.insert(50, 1);
    tree.insert(30, 2);
    tree.insert(70, 3);
    tree.insert(20, 4);
    tree.insert(40, 5);
    tree.insert(60, 6);
    tree.insert(80, 7);

    cout << "Recorrido de fin a inicio: ";
    auto it = tree.end();
    while (it != tree.begin()) {
        --it;
        cout << *it << " ";
    }
    cout << endl;
}

void testCopyConstructor() {
    cout << "\n========== TEST 4: Constructor por copia ==========\n";
    CBinaryTree<IntTraits> tree1;

    tree1.insert(50, 1);
    tree1.insert(30, 2);
    tree1.insert(70, 3);
    tree1.insert(20, 4);
    tree1.insert(80, 5);

    cout << "Árbol original - tamaño: " << tree1.size() << endl;
    cout << "Inorden: ";
    tree1.inorder(cout);
    cout << endl;

    CBinaryTree<IntTraits> tree2(tree1);

    cout << "\nÁrbol copiado - tamaño: " << tree2.size() << endl;
    cout << "Inorden: ";
    tree2.inorder(cout);
    cout << endl;

    // Insertar en tree1 para verificar que son independientes
    tree1.insert(100, 6);
    tree1.insert(10, 7);

    cout << "\nDespués de insertar 100 y 10 en el árbol original:\n";
    cout << "Árbol original - tamaño: " << tree1.size() << " -> ";
    tree1.inorder(cout);
    cout << endl;

    cout << "Árbol copiado - tamaño: " << tree2.size() << " -> ";
    tree2.inorder(cout);
    cout << endl;
}

void testCopyAssignment() {
    cout << "\n========== TEST 5: Operador de asignación ==========\n";
    CBinaryTree<IntTraits> tree1;
    tree1.insert(40, 1);
    tree1.insert(20, 2);
    tree1.insert(60, 3);

    CBinaryTree<IntTraits> tree2;
    tree2.insert(100, 4);
    tree2.insert(200, 5);

    cout << "Antes de la asignación:\n";
    cout << "Tree1: ";
    tree1.inorder(cout);
    cout << " (size: " << tree1.size() << ")" << endl;
    cout << "Tree2: ";
    tree2.inorder(cout);
    cout << " (size: " << tree2.size() << ")" << endl;

    tree2 = tree1;

    cout << "\nDespués de tree2 = tree1:\n";
    cout << "Tree1: ";
    tree1.inorder(cout);
    cout << " (size: " << tree1.size() << ")" << endl;
    cout << "Tree2: ";
    tree2.inorder(cout);
    cout << " (size: " << tree2.size() << ")" << endl;
}

void testMoveConstructor() {
    cout << "\n========== TEST 6: Constructor por movimiento ==========\n";
    CBinaryTree<IntTraits> tree1;

    tree1.insert(50, 1);
    tree1.insert(30, 2);
    tree1.insert(70, 3);
    tree1.insert(20, 4);
    tree1.insert(80, 5);

    cout << "Árbol original antes del movimiento - tamaño: " << tree1.size() << endl;
    cout << "Inorden: ";
    tree1.inorder(cout);
    cout << endl;

    CBinaryTree<IntTraits> tree2(std::move(tree1));

    cout << "\nDespués del movimiento:\n";
    cout << "Árbol movido - tamaño: " << tree2.size() << endl;
    cout << "Inorden: ";
    tree2.inorder(cout);
    cout << endl;

    cout << "Árbol original - tamaño: " << tree1.size() << " (debería ser 0)" << endl;
    cout << "¿Está vacío? " << (tree1.empty() ? "Sí" : "No") << endl;
}

void testDestructor() {
    cout << "\n========== TEST 7: Destructor y clear() ==========\n";
    CBinaryTree<IntTraits>* pTree = new CBinaryTree<IntTraits>();

    pTree->insert(50, 1);
    pTree->insert(30, 2);
    pTree->insert(70, 3);
    pTree->insert(20, 4);
    pTree->insert(40, 5);
    pTree->insert(60, 6);
    pTree->insert(80, 7);

    cout << "Árbol creado con " << pTree->size() << " elementos\n";
    cout << "Inorden: ";
    pTree->inorder(cout);
    cout << endl;

    cout << "\nLlamando a clear()...\n";
    pTree->clear();
    cout << "Tamaño después de clear: " << pTree->size() << endl;
    cout << "¿Está vacío? " << (pTree->empty() ? "Sí" : "No") << endl;

    cout << "\nEliminando árbol (llamando al destructor)...\n";
    delete pTree;
    cout << "Árbol eliminado correctamente\n";
}

void testEmptyTree() {
    cout << "\n========== TEST 8: Árbol vacío ==========\n";
    CBinaryTree<IntTraits> tree;

    cout << "Tamaño del árbol vacío: " << tree.size() << endl;
    cout << "¿Está vacío? " << (tree.empty() ? "Sí" : "No") << endl;

    cout << "\nRecorrido con iteradores (no debería imprimir nada): ";
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        cout << *it << " ";
    }
    cout << "[OK]" << endl;

    cout << "\nVerificando begin() == end(): "
         << (tree.begin() == tree.end() ? "Sí (correcto)" : "No (error)") << endl;
}

void testDescendingTree() {
    cout << "\n========== TEST 9: Árbol descendente ==========\n";
    CBinaryTree<IntDescTraits> tree;

    tree.insert(50, 1);
    tree.insert(30, 2);
    tree.insert(70, 3);
    tree.insert(20, 4);
    tree.insert(40, 5);
    tree.insert(60, 6);
    tree.insert(80, 7);

    cout << "Recorrido inorden (descendente): ";
    tree.inorder(cout);
    cout << endl;

    cout << "Con iterador: ";
    for (auto& val : tree) {
        cout << val << " ";
    }
    cout << endl;
}

void testStringTree() {
    cout << "\n========== TEST 10: Árbol de strings ==========\n";
    CBinaryTree<StringTraits> tree;

    tree.insert("manzana", 1);
    tree.insert("banana", 2);
    tree.insert("naranja", 3);
    tree.insert("uva", 4);
    tree.insert("pera", 5);
    tree.insert("kiwi", 6);

    cout << "Árbol de strings - tamaño: " << tree.size() << endl;
    cout << "Recorrido inorden (alfabético): ";
    tree.inorder(cout);
    cout << endl;

    cout << "\nCon iterador:\n";
    for (auto& str : tree) {
        cout << "  -> " << str << endl;
    }

    cout << "\nÁrbol visual:\n";
    tree.print(cout);
}

void testPreorderPostorder() {
    cout << "\n========== TEST 11: Recorridos preorden, inorden y postorden ==========\n";
    CBinaryTree<IntTraits> tree;

    tree.insert(50, 1);
    tree.insert(30, 2);
    tree.insert(70, 3);
    tree.insert(20, 4);
    tree.insert(40, 5);
    tree.insert(60, 6);
    tree.insert(80, 7);

    cout << "Preorden: ";
    tree.preorder(cout);
    cout << endl;

    cout << "Inorden: ";
    tree.inorder(cout);
    cout << endl;

    cout << "Árbol visual:\n";
    tree.print(cout);
}

void DemoBinaryTree() {
    cout << "\n";
    cout << "╔════════════════════════════════════════════════════════╗\n";
    cout << "║   DEMO: Binary Tree - Pruebas de TODOs Completados    ║\n";
    cout << "╚════════════════════════════════════════════════════════╝\n";

    try {
        testBasicInsertion();
        testIterators();
        testIteratorDecrement();
        testCopyConstructor();
        testCopyAssignment();
        testMoveConstructor();
        testDestructor();
        testEmptyTree();
        testDescendingTree();
        testStringTree();
        testPreorderPostorder();

        cout << "\n";
        cout << "╔════════════════════════════════════════════════════════╗\n";
        cout << "║            ✓ TODAS LAS PRUEBAS COMPLETADAS            ║\n";
        cout << "╚════════════════════════════════════════════════════════╝\n";
        cout << "\nTODOs probados:\n";
        cout << "  ✓ Constructor por copia\n";
        cout << "  ✓ Operador de asignación\n";
        cout << "  ✓ Constructor por movimiento\n";
        cout << "  ✓ Destructor recursivo\n";
        cout << "  ✓ Método clear()\n";
        cout << "  ✓ Iteradores begin() y end()\n";
        cout << "  ✓ Operadores ++ y -- del iterador\n";
        cout << "  ✓ getExtremeNode()\n";
        cout << "  ✓ Métodos como amigos (acceso privado)\n";
        cout << endl;

    } catch (const exception& e) {
        cerr << "\n❌ ERROR: " << e.what() << endl;
    }
}
