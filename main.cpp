#include <iostream>
#include "RTree.h"

using namespace std;

struct GeoTraits { 
    using CoordType = double;
    using ObjIDType = int;
    using PointType = Point2D<CoordType>;
    using RectType  = Rectangle<CoordType>;
};

int main(){
    RTree<GeoTraits> rtree(4);
    std::cout << "RTREE TEST" << std::endl;
    
    // Insercion en dos espacios separados
    rtree.Insert({10 , 10}, 101);
    rtree.Insert({11 , 11}, 102);
    rtree.Insert({10.5 , 10.5}, 103);
    rtree.Insert({50 , 50}, 104);
    rtree.Insert({51 , 52}, 105);
    rtree.Insert({49 , 48}, 106);

    // Informacion del arbol
    std::cout << "Elementos insertados: " << rtree.size() << std::endl;
    std::cout << "Altura del arbol: " << rtree.height() << std::endl;

    // Busqueda en una region
    Rectangle<double> searchRegion(0, 0, 20, 20);
    std::cout << "\nBuscando en caja [0,0] - [20,20]" << std::endl;

    auto found = rtree.RangeQuery(searchRegion);
    for (const auto& item : found) {
        std::cout << " -> Encontrado ID: " << item.second 
              << " en (" << item.first.x << ", " << item.first.y << ")" 
              << std::endl;
    }
    std::cout << "Esperado: 101, 102, 103" << std::endl;

    // Busqueda en area vacia
    Rectangle<double> emptyRegion(30, 30, 40, 40);
    std::cout << "\nBuscando en caja [30,30] - [40,40]" << std::endl;

    found = rtree.RangeQuery(emptyRegion);
    for (const auto& item : found) {
        std::cout << " -> Encontrado ID: " << item.second << std::endl;
    }
    std::cout << "Esperado: ninguno" << std::endl;

    // Busqueda KNN
    auto knnResults = rtree.KNN({8, 9}, 3);
    std::cout << "\nBuscando 3-NN para el punto (8,9)" << std::endl;
    for (const auto& neighbour : knnResults) {
        std::cout << " -> ID: " << neighbour.dataId << " Distancia^2: " << neighbour.distSq << std::endl;
    }
    std::cout << "Esperado: IDs 101, 103, 102" << std::endl;

    // Test Borrado
    rtree.Insert({67,67}, 999);
    std::cout << "\nAntes de borrar, size del arbol: " << rtree.size() << std::endl;

    bool removed = rtree.Remove({67,67}, 999);
    std::cout << "Borrado del punto (67,67) con ID 999: " << (removed ? "Exitoso" : "Fallido") << std::endl;
    std::cout << "Despues de borrar, size del arbol: " << rtree.size() << std::endl;

    // Guardado
    std::string filename = "index.txt";
    std::cout << "\nGuardando en archivo: " << filename << "..." << std::endl;
    if (rtree.Save(filename)) {
        std::cout << " -> EXITO: Archivo guardado correctamente." << std::endl;
    } else {
        std::cerr << " -> ERROR: No se pudo crear el archivo." << std::endl;
        return 0;
    }

    // Lectura
    std::cout << "\nCreando arbol vacio" << std::endl;
    RTree<GeoTraits> treeB(4); 

    std::cout << "Size antes de cargar: " << treeB.size() << std::endl;

    if (treeB.Load(filename)) {
        std::cout << " -> EXITO: Archivo cargado." << std::endl;
    } else {
        std::cerr << " -> ERROR: No se pudo leer el archivo (quizas no existe o formato incorrecto)." << std::endl;
        return 1;
    }
    std::cout << "Size despues de cargar: " << treeB.size();

    return 0;
}

// int main(int nArgs, char *pArgs[]){
//     cout << "Hello Alg y EDA-UNI (forma #2)" << endl;
//     int i;
//     for(i = 0 ; i < nArgs ; ++i){
//         cout << pArgs[i] << endl;
//     }
// }



