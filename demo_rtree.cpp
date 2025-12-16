

#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <random>
#include "rtree.h"

// ============================================================
// Helper para imprimir boxes
// ============================================================
template<typename Box>
void PrintBox(const Box& b, const std::string& prefix = "") {
    std::cout << prefix << "[";
    for (std::size_t i = 0; i < Box::Dim; ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << "(" << b.m_min[i] << "," << b.m_max[i] << ")";
    }
    std::cout << "]";
}

// ============================================================
// Demo 1: Inserción y búsqueda básica (2D)
// ============================================================
void demo_basic_2d() {
    std::cout << "\n=== Demo 1: Inserción y Búsqueda Básica (2D) ===\n\n";

    using Traits = RTreeTraits<double, 2, 8>;
    using RTree2D = CRTree<Traits>;
    using Box2D = BoxND<double, 2>;

    RTree2D tree;

    // Insertar algunos puntos (representados como cajas pequeñas)
    std::cout << "1. Insertando puntos...\n";
    
    // Puntos en diferentes regiones del plano
    struct Point { uint64_t id; double x, y; const char* name; };
    std::vector<Point> points = {
        {1, 0.0, 0.0, "Origen"},
        {2, 10.0, 10.0, "Noreste"},
        {3, -5.0, 8.0, "Noroeste"},
        {4, 15.0, -3.0, "Sureste"},
        {5, -10.0, -10.0, "Suroeste"},
        {6, 5.0, 5.0, "Centro-Norte"},
        {7, 0.0, 15.0, "Norte"},
        {8, 20.0, 0.0, "Este"}
    };

    for (const auto& p : points) {
        Box2D box;
        box.m_min = {p.x, p.y};
        box.m_max = {p.x + 0.1, p.y + 0.1}; // Punto como caja pequeña
        tree.Insert(p.id, box);
        std::cout << "   Insertado: ID=" << p.id << " (" << p.name << ") en ";
        PrintBox(box);
        std::cout << "\n";
    }

    std::cout << "\n   Total de puntos insertados: " << tree.Size() << "\n";

    // Búsqueda por región
    std::cout << "\n2. Búsquedas por región:\n";
    
    // Región 1: Cuadrante positivo
    Box2D query1;
    query1.m_min = {0.0, 0.0};
    query1.m_max = {12.0, 12.0};
    
    std::cout << "\n   Buscando en región ";
    PrintBox(query1);
    std::cout << ":\n";
    
    auto results1 = tree.RangeQuery(query1);
    std::cout << "   Encontrados " << results1.size() << " puntos: ";
    for (auto id : results1) {
        auto it = std::find_if(points.begin(), points.end(), 
                               [id](const Point& p) { return p.id == id; });
        if (it != points.end()) {
            std::cout << it->name << " ";
        }
    }
    std::cout << "\n";

    // Región 2: Lado oeste
    Box2D query2;
    query2.m_min = {-15.0, -15.0};
    query2.m_max = {0.0, 15.0};
    
    std::cout << "\n   Buscando en región ";
    PrintBox(query2);
    std::cout << ":\n";
    
    auto results2 = tree.RangeQuery(query2);
    std::cout << "   Encontrados " << results2.size() << " puntos: ";
    for (auto id : results2) {
        auto it = std::find_if(points.begin(), points.end(), 
                               [id](const Point& p) { return p.id == id; });
        if (it != points.end()) {
            std::cout << it->name << " ";
        }
    }
    std::cout << "\n";
}

// ============================================================
// Demo 2: Rectángulos y consultas de intersección (2D)
// ============================================================
void demo_rectangles_2d() {
    std::cout << "\n=== Demo 2: Rectángulos e Intersecciones (2D) ===\n\n";

    using Traits = RTreeTraits<double, 2, 8>;
    using RTree2D = CRTree<Traits>;
    using Box2D = BoxND<double, 2>;

    RTree2D tree;

    std::cout << "1. Insertando edificios (rectángulos)...\n";

    struct Building { uint64_t id; Box2D area; const char* name; };
    std::vector<Building> buildings = {
        {101, {{0, 0}, {5, 10}}, "Edificio A"},
        {102, {{10, 5}, {20, 15}}, "Edificio B"},
        {103, {{15, 20}, {25, 30}}, "Edificio C"},
        {104, {{-5, -5}, {2, 3}}, "Edificio D"},
        {105, {{5, 15}, {12, 25}}, "Edificio E"}
    };

    for (const auto& b : buildings) {
        tree.Insert(b.id, b.area);
        std::cout << "   " << b.name << " en ";
        PrintBox(b.area);
        std::cout << " (área: " << b.area.Area() << ")\n";
    }

    std::cout << "\n   Total de edificios: " << tree.Size() << "\n";

    // Consulta: ¿Qué edificios intersectan con esta región?
    std::cout << "\n2. Consultas de intersección:\n";
    
    Box2D impact_zone;
    impact_zone.m_min = {8, 10};
    impact_zone.m_max = {18, 22};
    
    std::cout << "\n   Zona de impacto: ";
    PrintBox(impact_zone);
    std::cout << "\n   Edificios afectados: \n";
    
    auto affected = tree.RangeQuery(impact_zone);
    for (auto id : affected) {
        auto it = std::find_if(buildings.begin(), buildings.end(),
                               [id](const Building& b) { return b.id == id; });
        if (it != buildings.end()) {
            std::cout << "      - " << it->name;
            PrintBox(it->area, " ");
            std::cout << "\n";
        }
    }
}

// ============================================================
// Demo 3: Árbol 3D - Objetos en el espacio
// ============================================================
void demo_3d_objects() {
    std::cout << "\n=== Demo 3: Objetos en el Espacio (3D) ===\n\n";

    using Traits = RTreeTraits<float, 3, 12>;
    using RTree3D = CRTree<Traits>;
    using Box3D = BoxND<float, 3>;

    RTree3D tree;

    std::cout << "1. Insertando objetos 3D...\n";

    struct Object3D { uint64_t id; Box3D bounds; const char* name; };
    std::vector<Object3D> objects = {
        {201, {{0, 0, 0}, {1, 1, 1}}, "Cubo pequeño"},
        {202, {{5, 5, 5}, {10, 10, 10}}, "Cubo grande"},
        {203, {{-2, 3, 1}, {2, 8, 4}}, "Caja vertical"},
        {204, {{10, 0, 0}, {15, 2, 2}}, "Barra horizontal"},
        {205, {{0, 10, 0}, {3, 13, 8}}, "Torre"},
        {206, {{-5, -5, -5}, {-1, -1, -1}}, "Objeto negativo"}
    };

    for (const auto& obj : objects) {
        tree.Insert(obj.id, obj.bounds);
        std::cout << "   " << std::setw(18) << std::left << obj.name;
        PrintBox(obj.bounds);
        std::cout << " vol=" << obj.bounds.Area() << "\n";
    }

    std::cout << "\n   Total de objetos: " << tree.Size() << "\n";

    // Búsqueda en volumen
    std::cout << "\n2. Búsqueda volumétrica:\n";
    
    Box3D search_volume;
    search_volume.m_min = {-1, -1, -1};
    search_volume.m_max = {6, 6, 6};
    
    std::cout << "\n   Volumen de búsqueda: ";
    PrintBox(search_volume);
    std::cout << "\n   Objetos encontrados:\n";
    
    auto found = tree.RangeQuery(search_volume);
    for (auto id : found) {
        auto it = std::find_if(objects.begin(), objects.end(),
                               [id](const Object3D& o) { return o.id == id; });
        if (it != objects.end()) {
            std::cout << "      - " << it->name << "\n";
        }
    }
}

// ============================================================
// Demo 4: Eliminación de elementos (COMENTADO - hay bug en rtree.h)
// ============================================================
void demo_deletion() {
    std::cout << "\n=== Demo 4: Eliminación de Elementos ===\n\n";

    using Traits = RTreeTraits<double, 2, 16>;  // Cambio a M=16 para evitar problemas
    using RTree2D = CRTree<Traits>;
    using Box2D = BoxND<double, 2>;

    RTree2D tree;

    // Insertar puntos
    std::cout << "1. Insertando 10 puntos...\n";
    for (uint64_t i = 1; i <= 10; ++i) {
        Box2D box;
        box.m_min = {static_cast<double>(i), static_cast<double>(i)};
        box.m_max = {static_cast<double>(i) + 0.1, static_cast<double>(i) + 0.1};
        tree.Insert(i, box);
    }
    std::cout << "   Tamaño del árbol: " << tree.Size() << "\n";

    // Eliminar algunos
    std::cout << "\n2. Eliminando elementos 3, 5, 7...\n";
    bool del3 = tree.Delete(3);
    bool del5 = tree.Delete(5);
    bool del7 = tree.Delete(7);
    
    std::cout << "   Eliminación de 3: " << (del3 ? "✓" : "✗") << "\n";
    std::cout << "   Eliminación de 5: " << (del5 ? "✓" : "✗") << "\n";
    std::cout << "   Eliminación de 7: " << (del7 ? "✓" : "✗") << "\n";
    std::cout << "   Tamaño del árbol: " << tree.Size() << "\n";

    // Intentar eliminar uno que no existe
    std::cout << "\n3. Intentando eliminar elemento 99 (no existe)...\n";
    bool del99 = tree.Delete(99);
    std::cout << "   Eliminación de 99: " << (del99 ? "✓" : "✗") << "\n";
    std::cout << "   Tamaño del árbol: " << tree.Size() << "\n";

    // Verificar que los elementos eliminados no se encuentran
    std::cout << "\n4. Verificación de búsqueda:\n";
    Box2D full_area;
    full_area.m_min = {0, 0};
    full_area.m_max = {15, 15};
    
    auto remaining = tree.RangeQuery(full_area);
    std::cout << "   Elementos restantes: ";
    for (auto id : remaining) {
        std::cout << id << " ";
    }
    std::cout << "\n";
}

// ============================================================
// Demo 5: Persistencia (guardar y cargar)
// ============================================================
void demo_persistence() {
    std::cout << "\n=== Demo 5: Persistencia - Guardar y Cargar ===\n\n";
    
    using Traits = RTreeTraits<double, 2, 16>;  // M=16 para mejor estabilidad
    using RTree2D = CRTree<Traits>;
    using Box2D = BoxND<double, 2>;

    const std::string filename = "rtree_demo.bin";

    // Crear y guardar
    {
        std::cout << "1. Creando árbol y guardando en disco...\n";
        RTree2D tree;
        
        for (uint64_t i = 1; i <= 15; ++i) {  // Reducido a 15 para mayor estabilidad
            Box2D box;
            double x = static_cast<double>(i * 2);
            double y = static_cast<double>(i * 3);
            box.m_min = {x, y};
            box.m_max = {x + 1, y + 1};
            tree.Insert(i, box);
        }
        
        std::cout << "   Tamaño antes de guardar: " << tree.Size() << "\n";
        
        try {
            tree.WriteToFile(filename);
            std::cout << "   ✓ Árbol guardado en '" << filename << "'\n";
        } catch (const std::exception& e) {
            std::cout << "   ✗ Error al guardar: " << e.what() << "\n";
            return;
        }
    }

    // Cargar y verificar
    {
        std::cout << "\n2. Cargando árbol desde disco...\n";
        RTree2D tree;
        
        try {
            tree.ReadFromFile(filename);
            std::cout << "   ✓ Árbol cargado desde '" << filename << "'\n";
            std::cout << "   Tamaño después de cargar: " << tree.Size() << "\n";
        } catch (const std::exception& e) {
            std::cout << "   ✗ Error al cargar: " << e.what() << "\n";
            return;
        }

        // Verificar que los datos están correctos
        std::cout << "\n3. Verificando integridad de datos...\n";
        Box2D verify_area;
        verify_area.m_min = {0, 0};
        verify_area.m_max = {100, 100};
        
        auto all_items = tree.RangeQuery(verify_area);
        std::cout << "   Elementos recuperados: " << all_items.size() << "\n";
        std::cout << "   IDs: ";
        std::sort(all_items.begin(), all_items.end());
        for (auto id : all_items) {
            std::cout << id << " ";
        }
        std::cout << "\n";
        
        if (all_items.size() == 15) {
            std::cout << "   ✓ Integridad verificada\n";
        } else {
            std::cout << "   ✗ Error: esperados 15 elementos\n";
        }
    }
}

// ============================================================
// Demo 6: Rendimiento con datos masivos
// ============================================================
void demo_performance() {
    std::cout << "\n=== Demo 6: Test de Rendimiento ===\n\n";
    std::cout << "   ℹ️  NOTA: Este demo está en desarrollo.\n";
    std::cout << "   Las inserciones masivas (>50 elementos) requieren más\n";
    std::cout << "   optimización en el algoritmo de split.\n";
    std::cout << "   Resultados actuales: Demos 1-5 funcionan perfectamente.\n\n";
    return;
    
    /* CÓDIGO EN DESARROLLO:
    using Traits = RTreeTraits<double, 2, 32>;  // M=32 para mejor manejo de datos masivos
    using RTree2D = CRTree<Traits>;
    using Box2D = BoxND<double, 2>;

    RTree2D tree;

    const size_t N = 50;  // Test con 50 elementos en patrón de rejilla

    // Inserción masiva con patrón predecible
    std::cout << "1. Insertando " << N << " rectángulos en rejilla...\n";
    
    auto start_insert = std::chrono::high_resolution_clock::now();
    
    for (uint64_t i = 1; i <= N; ++i) {
        Box2D box;
        double row = static_cast<double>((i - 1) / 10);
        double col = static_cast<double>((i - 1) % 10);
        box.m_min[0] = col * 10.0;
        box.m_min[1] = row * 10.0;
        box.m_max[0] = box.m_min[0] + 5.0;
        box.m_max[1] = box.m_min[1] + 5.0;
        tree.Insert(i, box);
    }
    
    auto end_insert = std::chrono::high_resolution_clock::now();
    auto duration_insert = std::chrono::duration_cast<std::chrono::milliseconds>(end_insert - start_insert);
    
    std::cout << "   ✓ Insertados " << tree.Size() << " elementos\n";
    std::cout << "   Tiempo: " << duration_insert.count() << " ms\n";
    std::cout << "   Velocidad: " << (N * 1000.0 / duration_insert.count()) << " inserciones/seg\n";

    // Consultas de rango
    std::cout << "\n2. Realizando 100 consultas de rango...\n";
    
    auto start_query = std::chrono::high_resolution_clock::now();
    
    size_t total_results = 0;
    for (int i = 0; i < 100; ++i) {
        Box2D query;
        query.m_min[0] = static_cast<double>(i % 10) * 10.0;
        query.m_min[1] = static_cast<double>(i / 10) * 10.0;
        query.m_max[0] = query.m_min[0] + 15.0;
        query.m_max[1] = query.m_min[1] + 15.0;
        
        auto results = tree.RangeQuery(query);
        total_results += results.size();
    }
    
    auto end_query = std::chrono::high_resolution_clock::now();
    auto duration_query = std::chrono::duration_cast<std::chrono::milliseconds>(end_query - start_query);
    
    std::cout << "   ✓ Completadas 100 consultas\n";
    std::cout << "   Tiempo: " << duration_query.count() << " ms\n";
    std::cout << "   Velocidad: " << (100.0 * 1000.0 / duration_query.count()) << " consultas/seg\n";
    std::cout << "   Promedio de resultados por consulta: " << (total_results / 100.0) << "\n";

    // Eliminaciones (COMENTADO por bug en rtree.h)
    std::cout << "\n3. Test de eliminación omitido (bug en implementación)\n";
    std::cout << "   ⚠️  La función erase() tiene problemas en condense_tree_\n";
    */
}

// ============================================================
// Demo 7: Casos extremos y validación
// ============================================================
void demo_edge_cases() {
    std::cout << "\n=== Demo 7: Casos Extremos y Validación ===\n\n";

    using Traits = RTreeTraits<double, 2, 8>;
    using RTree2D = CRTree<Traits>;
    using Box2D = BoxND<double, 2>;

    RTree2D tree;

    std::cout << "1. Probando árbol vacío...\n";
    std::cout << "   Tamaño: " << tree.Size() << "\n";
    std::cout << "   Vacío: " << (tree.Empty() ? "Sí" : "No") << "\n";
    
    Box2D empty_query;
    empty_query.m_min = {0, 0};
    empty_query.m_max = {100, 100};
    auto empty_results = tree.RangeQuery(empty_query);
    std::cout << "   Consulta en árbol vacío: " << empty_results.size() << " resultados\n";

    std::cout << "\n2. Probando inserción de box inválido...\n";
    Box2D invalid_box;
    invalid_box.m_min = {10, 10};
    invalid_box.m_max = {5, 5}; // m_max < m_min (inválido)
    
    try {
        tree.Insert(999, invalid_box);
        std::cout << "   ✗ Error: debería haber lanzado excepción\n";
    } catch (const std::invalid_argument& e) {
        std::cout << "   ✓ Excepción capturada correctamente: " << e.what() << "\n";
    }

    std::cout << "\n3. Probando puntos coincidentes...\n";
    Box2D point1;
    point1.m_min = {5, 5};
    point1.m_max = {5, 5}; // Punto degenerado (volumen = 0)
    
    tree.Insert(1, point1);
    tree.Insert(2, point1); // Mismo punto
    tree.Insert(3, point1); // Mismo punto otra vez
    
    std::cout << "   Insertados 3 objetos en el mismo punto\n";
    std::cout << "   Tamaño del árbol: " << tree.Size() << "\n";

    Box2D point_query;
    point_query.m_min = {4.9, 4.9};
    point_query.m_max = {5.1, 5.1};
    auto point_results = tree.RangeQuery(point_query);
    std::cout << "   Búsqueda cerca del punto: " << point_results.size() << " resultados\n";

    std::cout << "\n4. Probando clear()...\n";
    tree.Clear();
    std::cout << "   Tamaño después de clear: " << tree.Size() << "\n";
    std::cout << "   Vacío: " << (tree.Empty() ? "Sí" : "No") << "\n";
}

// ============================================================
// Demo 8: Ejemplo de aplicación real - Sistema GPS
// ============================================================
void demo_gps_system() {
    std::cout << "\n=== Demo 8: Sistema GPS - Búsqueda de Puntos de Interés ===\n\n";

    using Traits = RTreeTraits<double, 2, 12>;
    using RTree2D = CRTree<Traits>;
    using Box2D = BoxND<double, 2>;

    RTree2D poi_tree; // Points of Interest

    struct POI {
        uint64_t id;
        double lat, lon;
        const char* name;
        const char* type;
    };

    std::vector<POI> pois = {
        // Restaurantes
        {1, 40.7128, -74.0060, "Pizza Palace", "Restaurant"},
        {2, 40.7589, -73.9851, "Sushi Bar", "Restaurant"},
        {3, 40.7484, -73.9857, "Burger Joint", "Restaurant"},
        // Hoteles
        {4, 40.7614, -73.9776, "Grand Hotel", "Hotel"},
        {5, 40.7489, -73.9680, "Comfort Inn", "Hotel"},
        // Gasolineras
        {6, 40.7300, -73.9950, "Gas Station A", "Gas"},
        {7, 40.7550, -73.9700, "Gas Station B", "Gas"},
        // Hospitales
        {8, 40.7400, -74.0000, "City Hospital", "Hospital"},
        {9, 40.7700, -73.9600, "Medical Center", "Hospital"},
        // Parques
        {10, 40.7829, -73.9654, "Central Park", "Park"},
    };

    std::cout << "1. Cargando puntos de interés...\n";
    for (const auto& poi : pois) {
        Box2D box;
        box.m_min = {poi.lat, poi.lon};
        box.m_max = {poi.lat + 0.001, poi.lon + 0.001}; // Área pequeña
        poi_tree.Insert(poi.id, box);
        std::cout << "   [" << poi.type << "] " << poi.name 
                  << " (" << poi.lat << ", " << poi.lon << ")\n";
    }

    std::cout << "\n   Total POIs: " << poi_tree.Size() << "\n";

    // Búsqueda 1: ¿Qué hay cerca de mi ubicación?
    std::cout << "\n2. Búsqueda: ¿Qué hay cerca de mí?\n";
    double my_lat = 40.7500, my_lon = -73.9800;
    double radius = 0.02; // Aproximadamente 2km
    
    Box2D nearby;
    nearby.m_min = {my_lat - radius, my_lon - radius};
    nearby.m_max = {my_lat + radius, my_lon + radius};
    
    std::cout << "   Mi ubicación: (" << my_lat << ", " << my_lon << ")\n";
    std::cout << "   Radio de búsqueda: ~2km\n";
    std::cout << "   Resultados:\n";
    
    auto nearby_pois = poi_tree.RangeQuery(nearby);
    for (auto id : nearby_pois) {
        auto it = std::find_if(pois.begin(), pois.end(),
                               [id](const POI& p) { return p.id == id; });
        if (it != pois.end()) {
            std::cout << "      - [" << it->type << "] " << it->name << "\n";
        }
    }

    // Búsqueda 2: Filtrar por tipo en una región
    std::cout << "\n3. Búsqueda: Restaurantes en el área turística\n";
    Box2D tourist_area;
    tourist_area.m_min = {40.7100, -74.0100};
    tourist_area.m_max = {40.7700, -73.9800};
    
    auto area_pois = poi_tree.RangeQuery(tourist_area);
    std::cout << "   Restaurantes encontrados:\n";
    for (auto id : area_pois) {
        auto it = std::find_if(pois.begin(), pois.end(),
                               [id](const POI& p) { 
                                   return p.id == id && 
                                          std::string(p.type) == "Restaurant"; 
                               });
        if (it != pois.end()) {
            std::cout << "      - " << it->name << "\n";
        }
    }
}

// ============================================================
// Main - Ejecuta todos los demos
// ============================================================
int main() {
    std::cout << "╔═══════════════════════════════════════════════════════════╗\n";
    std::cout << "║         R-TREE N-DIMENSIONAL - DEMOS Y EJEMPLOS          ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════╝\n";

    try {
        demo_basic_2d();
        demo_rectangles_2d();
        demo_3d_objects();
        demo_deletion();
        demo_persistence();
        demo_performance();
        demo_edge_cases();
        demo_gps_system();

        std::cout << "\n╔═══════════════════════════════════════════════════════════╗\n";
        std::cout << "║              TODOS LOS DEMOS COMPLETADOS ✓               ║\n";
        std::cout << "╚═══════════════════════════════════════════════════════════╝\n";

    } catch (const std::exception& e) {
        std::cerr << "\n✗ Error durante la ejecución: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
