# R-Tree Implementation

Implementación de un R-Tree con todas las operaciones básicas.

## Estructura de archivos

- `rtreenode.h` - Implementación de los nodos del R-Tree
- `rtree.h` - Clase principal del R-Tree
- `ejemplo_rtree.cpp` - Ejemplos de uso de todas las operaciones
- `Makefile` - Archivo para compilar y ejecutar

## Funcionalidades implementadas

- ✅ Inserciones (Insert)
- ✅ Borrado (Remove)
- ✅ Range Query
- ✅ Write to Disk
- ✅ Read from Disk

## Compilar y ejecutar

### Compilar
```bash
make
```

### Ejecutar
```bash
make run
```

### Limpiar
```bash
make clean
```

## Ejemplos de uso

El archivo `ejemplo_rtree.cpp` contiene ejemplos completos de todas las operaciones:

### 1. Inserciones
```cpp
CRTree<IntRTreeTrait> rtree(4);
rtree.Insert(Rectangle<int>(0, 0, 10, 10), 1);
```

### 2. Range Query
```cpp
vector<long> results;
rtree.RangeQuery(Rectangle<int>(0, 0, 20, 20), results);
```

### 3. Write to Disk
```cpp
rtree.WriteToFile("rtree_data.bin");
```

### 4. Borrado
```cpp
rtree.Remove(Rectangle<int>(5, 5, 15, 15), 2);
```

### 5. Read from Disk
```cpp
CRTree<IntRTreeTrait> rtree2(4);
rtree2.ReadFromFile("rtree_data.bin");
```
