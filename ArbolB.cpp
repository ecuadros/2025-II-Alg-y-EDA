#include <time.h>
#include <stdlib.h>
#include "btree.h"
#include <fstream>
#include <string>
#include <vector>

const int BTreeSize = 3;
void DemoRTree() {
    cout << "\n R-Tree: " << endl;


    RTree<RTreeTrait<int, 2>> rtree(BTreeSize);
    
    // ejemplo
    std::vector<std::pair<HyperRect<2>, int>> data = {
        {{{10, 10}, {20, 20}}, 1}, {{{15, 15}, {25, 25}}, 2}, {{{30, 30}, {40, 40}}, 3},
        {{{70, 70}, {80, 80}}, 4}, {{{5, 45}, {15, 55}}, 5},  {{{35, 5}, {45, 15}}, 6},
        {{{80, 10}, {90, 20}}, 7}, {{{10, 80}, {20, 90}}, 8}, {{{50, 50}, {60, 60}}, 9},
        {{{55, 55}, {65, 65}}, 10},{{{90, 90}, {100, 100}}, 11} 
    };

    cout << "Rectangulos en el R-Tree" << endl;
    for (const auto& pair : data) {
        rtree.Insert(pair.first, pair.second);
    }

    cout << "Estructura del R-Tree:" << endl;
    cout << rtree;

    cout << "\n Busqueda en el R-Tree" << endl;
    HyperRect<2> areaDeBusqueda = {{18, 18}, {35, 35}};
    cout << "Buscando rectangulos que intersectan con el area: " << areaDeBusqueda << endl;

    std::vector<int> resultados = rtree.Search(areaDeBusqueda);

    cout << "Resultados (IDs): ";
    for (int id : resultados) { cout << id << " "; }
    cout << endl;
}

void DemoRTree_Delete_Read_Write() {
    cout << "\nBorrado, Escritura y Lectura: " << endl;
    RTree<RTreeTrait<int, 2>> rtree(BTreeSize);

    std::vector<std::pair<HyperRect<2>, int>> data = {
        {{{10, 10}, {20, 20}}, 1}, {{{15, 15}, {25, 25}}, 2}, {{{30, 30}, {40, 40}}, 3},
        {{{70, 70}, {80, 80}}, 4}, {{{5, 45}, {15, 55}}, 5},  {{{35, 5}, {45, 15}}, 6},
        {{{50, 50}, {60, 60}}, 9}
    };
    for (const auto& pair : data) {
        rtree.Insert(pair.first, pair.second);
    }

    cout << "Arbol antes de borrar:" << endl;
    cout << rtree;

    // Borrado
    HyperRect<2> to_delete_rect = {{50, 50}, {60, 60}};
    int to_delete_id = 9;
    cout << "\nBorrando rectangulo con ID " << to_delete_id << " y area " << to_delete_rect << endl;
    if (rtree.Remove(to_delete_rect, to_delete_id)) {
        cout << "Borrado exitoso." << endl;
    } else {
        cout << "No se pudo borrar." << endl;
    }
    cout << "Arbol despues de borrar:" << endl;
    cout << rtree;

    // Write en disco
    cout << "\nGuardando arbol en 'rtree.dat'" << endl;
    ofstream outFile("rtree.dat");
    rtree.Write(outFile);
    outFile.close();
    RTree<RTreeTrait<int, 2>> rtree_loaded(BTreeSize);
    cout << "Cargando arbol desde 'rtree.dat'..." << endl;
    ifstream inFile("rtree.dat");
    rtree_loaded.Read(inFile);
    inFile.close();
    cout << "Arbol cargado:" << endl;
    cout << rtree_loaded;
}

int main (int argc, char ** argv){
    DemoRTree();
    DemoRTree_Delete_Read_Write();
    return 1;
}