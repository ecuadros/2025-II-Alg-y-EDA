#include <iostream>
#include "rtree.h"

using namespace std;

// RTree 2D
using MyTrait2D = RTreeTrait<float, int, 2>;
using MyRTree2D = RTree<MyTrait2D, 4, 2>;

// RTree 3D
using MyTrait3D = RTreeTrait<float, int, 3>;
using MyRTree3D = RTree<MyTrait3D, 4, 2>;

// RTree 4D
using MyTrait4D = RTreeTrait<double, string, 4>;
using MyRTree4D = RTree<MyTrait4D, 4, 2>;

int main()
{
       cout << "=== Demo RTree N-Dimensional ===" << endl;
       
       // ===================== DEMO 2D =====================
       cout << "\n========== RTree 2D ==========" << endl;
       MyRTree2D arbol2D;
       
       // Insertar rectangulos 2D: {xMin, yMin}, {xMax, yMax}
       arbol2D.Insert({0, 0}, {10, 10}, 1);
       arbol2D.Insert({15, 15}, {25, 25}, 2);
       arbol2D.Insert({30, 5}, {40, 15}, 3);
       arbol2D.Insert({5, 30}, {15, 40}, 4);
       arbol2D.Insert({50, 50}, {60, 60}, 5);
       
       cout << "\n[Insert] 5 elementos 2D" << endl;
       cout << "Size: " << arbol2D.size() << endl;
       arbol2D.Print();
       
       // Range Query 2D
       cout << "\n[RangeQuery] Region (0,0)-(30,30)" << endl;
       auto resultados2D = arbol2D.RangeQuery({0, 0}, {30, 30});
       cout << "Encontrados: " << resultados2D.size() << endl;
       for (auto& r : resultados2D)
       {
              cout << "  (" << r.first.minCoord[0] << "," << r.first.minCoord[1] << ")-("
                   << r.first.maxCoord[0] << "," << r.first.maxCoord[1] << ") data=" << r.second << endl;
       }
       
       // Remove 2D
       cout << "\n[Remove] Elemento con MBR (15,15)-(25,25) y data=2" << endl;
       bool ok = arbol2D.Remove({15, 15}, {25, 25}, 2);
       cout << (ok ? "OK" : "No encontrado") << ", Size: " << arbol2D.size() << endl;
       
       // Write/Read 2D
       cout << "\n[Write] Guardando en datos_2d.txt" << endl;
       arbol2D.Write("datos_2d.txt");
       
       cout << "[Read] Leyendo en nuevo arbol 2D" << endl;
       MyRTree2D arbol2D_copia;
       arbol2D_copia.Read("datos_2d.txt");
       cout << "Size leido: " << arbol2D_copia.size() << endl;
       
       // ===================== DEMO 3D =====================
       cout << "\n========== RTree 3D ==========" << endl;
       MyRTree3D arbol3D;
       
       // Insertar cuboides 3D: {xMin, yMin, zMin}, {xMax, yMax, zMax}
       arbol3D.Insert({0, 0, 0}, {10, 10, 10}, 100);
       arbol3D.Insert({15, 15, 15}, {25, 25, 25}, 200);
       arbol3D.Insert({5, 5, 20}, {15, 15, 30}, 300);
       arbol3D.Insert({-5, -5, -5}, {5, 5, 5}, 400);
       
       cout << "\n[Insert] 4 elementos 3D" << endl;
       cout << "Size: " << arbol3D.size() << endl;
       arbol3D.Print();
       
       // Range Query 3D
       cout << "\n[RangeQuery] Region (0,0,0)-(20,20,20)" << endl;
       auto resultados3D = arbol3D.RangeQuery({0, 0, 0}, {20, 20, 20});
       cout << "Encontrados: " << resultados3D.size() << endl;
       for (auto& r : resultados3D)
       {
              cout << "  (" << r.first.minCoord[0] << "," << r.first.minCoord[1] << "," << r.first.minCoord[2] << ")-("
                   << r.first.maxCoord[0] << "," << r.first.maxCoord[1] << "," << r.first.maxCoord[2] << ") data=" << r.second << endl;
       }
       
       // Write 3D
       cout << "\n[Write] Guardando en datos_3d.txt" << endl;
       arbol3D.Write("datos_3d.txt");
       
       // ===================== DEMO 4D =====================
       cout << "\n========== RTree 4D  ==========" << endl;
       MyRTree4D arbol4D;
       
       // Insertar hiper-rectangulos 4D: {x, y, z, t}
       arbol4D.Insert({0, 0, 0, 0}, {10, 10, 10, 100}, "evento_A");
       arbol4D.Insert({5, 5, 5, 50}, {15, 15, 15, 150}, "evento_B");
       arbol4D.Insert({20, 20, 20, 0}, {30, 30, 30, 200}, "evento_C");
       
       cout << "\n[Insert] 3 elementos 4D" << endl;
       cout << "Size: " << arbol4D.size() << endl;
       arbol4D.Print();
       
       // Range Query 4D
       cout << "\n[RangeQuery] Region (0,0,0,0)-(12,12,12,120)" << endl;
       auto resultados4D = arbol4D.RangeQuery({0, 0, 0, 0}, {12, 12, 12, 120});
       cout << "Encontrados: " << resultados4D.size() << endl;
       for (auto& r : resultados4D)
       {
              cout << "  (" << r.first.minCoord[0] << "," << r.first.minCoord[1] << "," 
                   << r.first.minCoord[2] << "," << r.first.minCoord[3] << ")-("
                   << r.first.maxCoord[0] << "," << r.first.maxCoord[1] << "," 
                   << r.first.maxCoord[2] << "," << r.first.maxCoord[3] << ") data=" << r.second << endl;
       }
       
       return 0;
}
