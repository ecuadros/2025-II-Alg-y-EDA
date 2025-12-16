#include <iostream>
#include "rtree.h"

using namespace std;

using MyTrait = RTreeTrait<float, int>;
using MyRTree = RTree<MyTrait, 4, 2>;

int main()
{
       cout << "=== Demo RTree ===" << endl;
       
       MyRTree arbol;
       
       // 1. Insert
       arbol.Insert(0, 0, 10, 10, 1);
       arbol.Insert(15, 15, 25, 25, 2);
       arbol.Insert(30, 5, 40, 15, 3);
       arbol.Insert(5, 30, 15, 40, 4);
       arbol.Insert(50, 50, 60, 60, 5);
       
       cout << "\n[Insert] 5 elementos" << endl;
       cout << "Size: " << arbol.size() << endl;
       arbol.Print();
       
       // 2. Range Query
       cout << "\n[RangeQuery] Region (0,0)-(30,30)" << endl;
       auto resultados = arbol.RangeQuery(0, 0, 30, 30);
       cout << "Encontrados: " << resultados.size() << endl;
       for (auto& r : resultados)
       {
              cout << "  (" << r.first.xMin << "," << r.first.yMin << ")-("
                   << r.first.xMax << "," << r.first.yMax << ") data=" << r.second << endl;
       }
       
       // 3. Remove
       cout << "\n[Remove] Elemento con MBR (15,15)-(25,25) y data=2" << endl;
       bool ok = arbol.Remove(15, 15, 25, 25, 2);
       cout << (ok ? "OK" : "No encontrado") << ", Size: " << arbol.size() << endl;
       
       // 4. Write
       cout << "\n[Write] Guardando en datos.txt" << endl;
       arbol.Write("datos.txt");
       
       // 5. Read
       cout << "\n[Read] Leyendo en nuevo arbol" << endl;
       MyRTree arbol2;
       arbol2.Read("datos.txt");
       cout << "Size: " << arbol2.size() << endl;
       arbol2.Print();
       
       return 0;
}
