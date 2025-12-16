#include <iostream>
#include <iomanip>
#include "rtree.h"

using namespace std;

int main() {
    // Crear R-Tree 2D con capacidad 4
    RTree<RTree2D> rtree(4);

    cout << "=== Demo R-Tree (Rectangle Tree) ===" << endl;
    cout << "Estructura de indexación espacial 2D\n" << endl;
    cout << "1. Insertando objetos espaciales..." << endl;

    // Crear rectángulos distribuidos en el espacio 2D
    Rectangle<2> r1({0, 0}, {10, 10});
    rtree.Insert(r1, 1);
    cout << "  Insertado: ID=1 " << r1 << endl;

    Rectangle<2> r2({15, 15}, {25, 25});
    rtree.Insert(r2, 2);
    cout << "  Insertado: ID=2 " << r2 << endl;

    Rectangle<2> r3({5, 20}, {15, 30});
    rtree.Insert(r3, 3);
    cout << "  Insertado: ID=3 " << r3 << endl;

    Rectangle<2> r4({30, 5}, {40, 15});
    rtree.Insert(r4, 4);
    cout << "  Insertado: ID=4 " << r4 << endl;

    Rectangle<2> r5({35, 20}, {45, 30});
    rtree.Insert(r5, 5);
    cout << "  Insertado: ID=5 " << r5 << endl;

    Rectangle<2> r6({50, 50}, {60, 60});
    rtree.Insert(r6, 6);
    cout << "  Insertado: ID=6 " << r6 << endl;

    Rectangle<2> r7({55, 55}, {65, 65});
    rtree.Insert(r7, 7);
    cout << "  Insertado: ID=7 " << r7 << endl;

    Rectangle<2> r8({2, 2}, {8, 8});
    rtree.Insert(r8, 8);
    cout << "  Insertado: ID=8 " << r8 << endl;

    cout << "\nObjetos insertados: " << rtree.size() << endl;
    cout << "Altura del árbol: " << rtree.height() << endl;

    cout << "\n2. Estructura jerárquica del árbol:" << endl;
    cout << string(60, '-') << endl;
    rtree.Print(cout);
    cout << string(60, '-') << endl;

    cout << "\n3. Range Query - Buscar objetos que intersectan [12,12]-[28,28]" << endl;
    Rectangle<2> query1({12, 12}, {28, 28});
    vector<tagRTreeObjectInfo<RTree2D>> results;
    rtree.Search(query1, results);

    cout << "  Query: " << query1 << endl;
    cout << "  Resultados encontrados: " << results.size() << endl;
    for (const auto& obj : results) {
        cout << "    ID=" << obj.ObjID << " " << obj.mbr
             << " Área=" << fixed << setprecision(2) << obj.mbr.area() << endl;
    }

    cout << "\n4. Point Query - Buscar objetos que contienen el punto (20,22)" << endl;
    Point<2> point1{20, 22};
    results.clear();
    rtree.SearchPoint(point1, results);

    cout << "  Punto: " << point1 << endl;
    cout << "  Resultados: " << results.size() << endl;
    for (const auto& obj : results) {
        cout << "    ID=" << obj.ObjID << " " << obj.mbr << endl;
    }

    cout << "\n5. Iterando todos los objetos (forward iterator):" << endl;
    int count = 0;
    for (auto& obj : rtree) {
        cout << "  [" << ++count << "] ID=" << obj.ObjID
             << " " << obj.mbr
             << " Centro=" << obj.mbr.center()
             << " Área=" << obj.mbr.area() << endl;
    }

    cout << "\n6. Iterando en orden inverso (backward iterator):" << endl;
    count = 0;
    for (auto it = rtree.rbegin(); it != rtree.rend(); ++it) {
        cout << "  [" << ++count << "] ID=" << it->ObjID << " " << it->mbr << endl;
    }

    cout << "\n7. ForEach genérico - Contando objetos con área > 80:" << endl;
    int largeCount = 0;
    rtree.ForEach([](const auto& obj, int& counter) {
        if (obj.mbr.area() > 80)
            counter++;
    }, largeCount);
    cout << "  Objetos con área > 80: " << largeCount << endl;

    // ForEach - Imprimir todos los centros
    cout << "\n  Centros de todos los objetos:" << endl;
    rtree.ForEach([](const auto& obj) {
        cout << "    ID=" << obj.ObjID << " Centro=" << obj.mbr.center() << endl;
    });

    cout << "\n8. FirstThat - Primer objeto con Y mínimo > 20:" << endl;
    auto* found = rtree.FirstThat([](const auto& obj) {
        return obj.mbr.min[1] > 20;
    });
    if (found) {
        cout << "  Encontrado: ID=" << found->ObjID << " " << found->mbr << endl;
    } else {
        cout << "  No encontrado." << endl;
    }

    // FirstThat con parámetro
    cout << "\n  Primer objeto con área > 150:" << endl;
    double minArea = 150.0;
    found = rtree.FirstThat([](const auto& obj, double min) {
        return obj.mbr.area() > min;
    }, minArea);
    if (found) {
        cout << "  Encontrado: ID=" << found->ObjID
             << " Área=" << found->mbr.area() << endl;
    } else {
        cout << "  No encontrado." << endl;
    }

    cout << "\n9. Guardando árbol a disco..." << endl;
    rtree.Write("arbol_rtree.txt");
    cout << "  Guardado en 'arbol_rtree.txt'" << endl;

    // También guardar usando operator<<
    ofstream archivo("arbol_rtree2.txt");
    archivo << rtree;
    archivo.close();
    cout << "  Guardado en 'arbol_rtree2.txt' (usando operator<<)" << endl;

    cout << "\n10. Cargando árbol desde disco..." << endl;
    RTree<RTree2D> rtree2(4);
    rtree2.Read("arbol_rtree.txt");
    cout << "  Cargado: " << rtree2.size() << " objetos, altura=" << rtree2.height() << endl;

    // Verificar que se cargó correctamente
    cout << "\n  Verificando datos cargados (primeros 3 objetos):" << endl;
    count = 0;
    for (auto& obj : rtree2) {
        cout << "    ID=" << obj.ObjID << " " << obj.mbr << endl;
        if (++count >= 3) break;
    }

    cout << "\n11. Range Query en árbol cargado [50,50]-[70,70]:" << endl;
    Rectangle<2> query2({50, 50}, {70, 70});
    results.clear();
    rtree2.Search(query2, results);

    cout << "  Query: " << query2 << endl;
    cout << "  Resultados: " << results.size() << endl;
    for (const auto& obj : results) {
        cout << "    ID=" << obj.ObjID << " " << obj.mbr << endl;
    }

    cout << "\n12. Eliminando objeto ID=3 " << r3 << "..." << endl;
    bool removed = rtree.Remove(r3, 3);
    cout << "  Eliminado: " << (removed ? "SI" : "NO") << endl;
    cout << "  Objetos restantes: " << rtree.size() << endl;
    cout << "  Nueva altura: " << rtree.height() << endl;

    cout << "\n13. Range Query después de eliminar [0,0]-[20,35]:" << endl;
    Rectangle<2> query3({0, 0}, {20, 35});
    results.clear();
    rtree.Search(query3, results);

    cout << "  Query: " << query3 << endl;
    cout << "  Resultados: " << results.size() << endl;
    for (const auto& obj : results) {
        cout << "    ID=" << obj.ObjID << " " << obj.mbr << endl;
    }


    cout << "\n14. Estadísticas finales:" << endl;
    cout << "  Objetos totales: " << rtree.size() << endl;
    cout << "  Altura del árbol: " << rtree.height() << endl;
    cout << "  Capacidad máxima por nodo: " << rtree.GetMaxEntries() << endl;

    // Calcular área total cubierta
    double totalArea = 0;
    rtree.ForEach([](const auto& obj, double& total) {
        total += obj.mbr.area();
    }, totalArea);
    cout << "  Área total de objetos: " << totalArea << endl;

    // Encontrar objeto más grande
    auto* largest = rtree.FirstThat([](const auto& obj) {
        return true;  // Primer objeto
    });
    if (largest) {
        double maxArea = largest->mbr.area();
        rtree.ForEach([&maxArea, &largest](auto& obj) {
            if (obj.mbr.area() > maxArea) {
                maxArea = obj.mbr.area();
                largest = &obj;
            }
        });
        cout << "  Objeto más grande: ID=" << largest->ObjID
             << " Área=" << maxArea << endl;
    }


    cout << "\n15. Prueba con inserción masiva de objetos..." << endl;
    RTree<RTree2D> rtreeBig(4);

    // Insertar 20 objetos más
    for (int i = 0; i < 20; ++i) {
        Rectangle<2> rect({i * 5.0, i * 5.0}, {i * 5.0 + 10, i * 5.0 + 10});
        rtreeBig.Insert(rect, 100 + i);
    }

    cout << "  Objetos insertados: " << rtreeBig.size() << endl;
    cout << "  Altura del árbol: " << rtreeBig.height() << endl;

    // Range query grande
    Rectangle<2> bigQuery({0, 0}, {100, 100});
    results.clear();
    rtreeBig.Search(bigQuery, results);
    cout << "  Range Query [0,0]-[100,100]: " << results.size() << " resultados" << endl;

    cout << "\n=== Demo completado exitosamente ===" << endl;

    return 0;
}
