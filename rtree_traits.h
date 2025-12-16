#ifndef __RTREE_TRAITS_H__
#define __RTREE_TRAITS_H__

#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>

// Estructura básica de un Punto N-Dimensional
template <typename T, size_t Dim>
struct Point {
    T coords[Dim];

    const T& operator[](size_t i) const { return coords[i]; }
    T& operator[](size_t i) { return coords[i]; }
};

// Estructura de Rectángulo (MBR - Minimum Bounding Rectangle)
template <typename T, size_t Dim>
struct Rectangle {
    Point<T, Dim> minP;
    Point<T, Dim> maxP;

    // Calcular el área (o volumen en 3D)
    double Area() const {
        double area = 1.0;
        for (size_t i = 0; i < Dim; ++i) {
            area *= (maxP[i] - minP[i]);
        }
        return area;
    }

    // Verificar si contiene otro rectángulo
    bool Contains(const Rectangle& other) const {
        for (size_t i = 0; i < Dim; ++i) {
            if (other.minP[i] < minP[i] || other.maxP[i] > maxP[i])
                return false;
        }
        return true;
    }

    // Calcular el incremento de área necesario para incluir otro rectángulo
    double Enlargement(const Rectangle& other) const {
        double enlargedArea = 1.0;
        for (size_t i = 0; i < Dim; ++i) {
            T minV = std::min(minP[i], other.minP[i]);
            T maxV = std::max(maxP[i], other.maxP[i]);
            enlargedArea *= (maxV - minV);
        }
        return enlargedArea - Area();
    }

    // Unir este rectángulo con otro (expandirse)
    void Merge(const Rectangle& other) {
        for (size_t i = 0; i < Dim; ++i) {
            minP[i] = std::min(minP[i], other.minP[i]);
            maxP[i] = std::max(maxP[i], other.maxP[i]);
        }
    }
};

// Traits para configurar el R-Tree (similar a BTreeTrait)
template <typename _CoordType, size_t _Dim, typename _ObjIDType>
struct RTreeTrait {
    using CoordType = _CoordType;
    using ObjIDType = _ObjIDType;
    static const size_t Dimension = _Dim;
    using PointType = Point<CoordType, Dimension>;
    using RectType = Rectangle<CoordType, Dimension>;
};

#endif