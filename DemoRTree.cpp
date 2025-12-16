#include <iostream>
#include <vector>
#include "rtree.h"

using namespace std;

//---------------------------------------------------------
// Trait para RTree<long>
struct LongTrait {
    using ObjIDType = long;
};
//---------------------------------------------------------

int DemoRTree() {
    cout << "=== Demo R-Tree ===" << endl << endl;
    
    // Crear R-tree con máximo 4 entradas por nodo
    RTree<LongTrait> rtree(4);
    
    cout << "1. Inserción de rectángulos" << endl;
    cout << "----------------------------" << endl;
    
    // Insertar varios rectángulos (x_min, y_min, x_max, y_max)
    Rectangle rects[] = {
        Rectangle(0, 0, 10, 10),    // Rectángulo 1
        Rectangle(15, 15, 25, 25),  // Rectángulo 2
        Rectangle(5, 5, 15, 15),    // Rectángulo 3 (se superpone con 1)
        Rectangle(20, 20, 30, 30),  // Rectángulo 4 (se superpone con 2)
        Rectangle(35, 35, 45, 45),  // Rectángulo 5
        Rectangle(40, 40, 50, 50),  // Rectángulo 6 (se superpone con 5)
        Rectangle(10, 30, 20, 40),  // Rectángulo 7
        Rectangle(25, 5, 35, 15),   // Rectángulo 8
        Rectangle(0, 20, 10, 30),   // Rectángulo 9
        Rectangle(45, 10, 55, 20)   // Rectángulo 10
    };
    
    for (int i = 0; i < 10; i++) {
        bool ok = rtree.Insert(rects[i], i + 100);
        cout << "Insert rect " << (i + 1) << " [(" 
             << rects[i].x_min << "," << rects[i].y_min << ")-(" 
             << rects[i].x_max << "," << rects[i].y_max << ")]: " 
             << (ok ? "OK" : "Duplicado") << endl;
    }
    
    cout << "\nEstadísticas del árbol:" << endl;
    cout << "  Altura: " << rtree.height() << endl;
    cout << "  Número de objetos: " << rtree.size() << endl;
    
    cout << "\n2. Estructura del árbol" << endl;
    cout << "------------------------" << endl;
    rtree.Print(cout);
    
    cout << "\n3. Búsqueda de rectángulos específicos" << endl;
    cout << "---------------------------------------" << endl;
    
    // Buscar algunos rectángulos
    int searchIndices[] = {0, 4, 9};
    for (int idx : searchIndices) {
        long objID = rtree.Search(rects[idx]);
        if (objID != -1) {
            cout << "Encontrado rect " << (idx + 1) << " -> ObjID=" << objID << endl;
        } else {
            cout << "Rect " << (idx + 1) << " no encontrado" << endl;
        }
    }
    
    cout << "\n4. Consultas de rango (Range Queries)" << endl;
    cout << "--------------------------------------" << endl;
    
    // Query 1: Rectángulo que cubre la esquina superior izquierda
    Rectangle query1(0, 0, 20, 20);
    vector<RTreeObjectInfo<long>> results1;
    rtree.RangeQuery(query1, results1);
    
    cout << "Query 1: Rectángulos que intersectan con (" 
         << query1.x_min << "," << query1.y_min << ")-(" 
         << query1.x_max << "," << query1.y_max << ")" << endl;
    cout << "  Resultados encontrados: " << results1.size() << endl;
    for (const auto& result : results1) {
        cout << "    ObjID=" << result.ObjID << " [(" 
             << result.rect.x_min << "," << result.rect.y_min << ")-(" 
             << result.rect.x_max << "," << result.rect.y_max << ")]" << endl;
    }
    
    // Query 2: Rectángulo que cubre el centro
    Rectangle query2(10, 10, 30, 30);
    vector<RTreeObjectInfo<long>> results2;
    rtree.RangeQuery(query2, results2);
    
    cout << "\nQuery 2: Rectángulos que intersectan con (" 
         << query2.x_min << "," << query2.y_min << ")-(" 
         << query2.x_max << "," << query2.y_max << ")" << endl;
    cout << "  Resultados encontrados: " << results2.size() << endl;
    for (const auto& result : results2) {
        cout << "    ObjID=" << result.ObjID << " [(" 
             << result.rect.x_min << "," << result.rect.y_min << ")-(" 
             << result.rect.x_max << "," << result.rect.y_max << ")]" << endl;
    }
    
    // Query 3: Rectángulo en la esquina inferior derecha
    Rectangle query3(40, 40, 60, 60);
    vector<RTreeObjectInfo<long>> results3;
    rtree.RangeQuery(query3, results3);
    
    cout << "\nQuery 3: Rectángulos que intersectan con (" 
         << query3.x_min << "," << query3.y_min << ")-(" 
         << query3.x_max << "," << query3.y_max << ")" << endl;
    cout << "  Resultados encontrados: " << results3.size() << endl;
    for (const auto& result : results3) {
        cout << "    ObjID=" << result.ObjID << " [(" 
             << result.rect.x_min << "," << result.rect.y_min << ")-(" 
             << result.rect.x_max << "," << result.rect.y_max << ")]" << endl;
    }
    
    cout << "\n5. Escritura a disco" << endl;
    cout << "--------------------" << endl;
    
    string filename = "rtree_data.bin";
    bool writeOk = rtree.WriteToDisk(filename);
    cout << "Escribir árbol a disco (" << filename << "): " 
         << (writeOk ? "OK" : "ERROR") << endl;
    
    cout << "\n6. Lectura desde disco" << endl;
    cout << "----------------------" << endl;
    
    // Crear un nuevo árbol y leer desde disco
    RTree<LongTrait> rtree2(4);
    bool readOk = rtree2.ReadFromDisk(filename);
    cout << "Leer árbol desde disco (" << filename << "): " 
         << (readOk ? "OK" : "ERROR") << endl;
    
    if (readOk) {
        cout << "\nEstadísticas del árbol leído:" << endl;
        cout << "  Altura: " << rtree2.height() << endl;
        cout << "  Número de objetos: " << rtree2.size() << endl;
        
        // Verificar que los datos son correctos
        cout << "\nVerificación: Buscar algunos rectángulos en el árbol leído" << endl;
        for (int idx : searchIndices) {
            long objID = rtree2.Search(rects[idx]);
            cout << "  Rect " << (idx + 1) << ": " 
                 << (objID != -1 ? "Encontrado (ObjID=" + to_string(objID) + ")" : "No encontrado") 
                 << endl;
        }
    }
    
    cout << "\n7. Eliminación de rectángulos" << endl;
    cout << "-----------------------------" << endl;
    
    // Eliminar algunos rectángulos
    int deleteIndices[] = {0, 2, 5, 7};
    for (int idx : deleteIndices) {
        bool ok = rtree.Remove(rects[idx], idx + 100);
        cout << "Remove rect " << (idx + 1) << ": " 
             << (ok ? "OK" : "No encontrado") << endl;
    }
    
    cout << "\nEstadísticas después de eliminaciones:" << endl;
    cout << "  Altura: " << rtree.height() << endl;
    cout << "  Número de objetos: " << rtree.size() << endl;
    
    cout << "\n8. Estructura del árbol después de eliminaciones" << endl;
    cout << "-------------------------------------------------" << endl;
    rtree.Print(cout);
    
    cout << "\n9. Range Query después de eliminaciones" << endl;
    cout << "----------------------------------------" << endl;
    
    vector<RTreeObjectInfo<long>> results4;
    rtree.RangeQuery(query1, results4);
    cout << "Query con (" << query1.x_min << "," << query1.y_min << ")-(" 
         << query1.x_max << "," << query1.y_max << ")" << endl;
    cout << "  Resultados encontrados: " << results4.size() << endl;
    for (const auto& result : results4) {
        cout << "    ObjID=" << result.ObjID << " [(" 
             << result.rect.x_min << "," << result.rect.y_min << ")-(" 
             << result.rect.x_max << "," << result.rect.y_max << ")]" << endl;
    }
    
    cout << "\n=== Fin del Demo R-Tree ===" << endl;
    
    return 0;
}
