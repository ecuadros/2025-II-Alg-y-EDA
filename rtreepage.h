#ifndef __RTREEPAGE_H__
#define __RTREEPAGE_H__

#include <vector>
#include <iostream>
#include <cmath>
#include <limits>
#include <algorithm>
#include <functional>
#include <stack>

// Forward declarations
template <typename Trait>
class RTree;
template <typename Trait>
class RTreeForwardIterator;
template <typename Trait>
class RTreeBackwardIterator;

// Error codes para operaciones del R-Tree
enum rt_ErrorCode {
    rt_ok,
    rt_overflow,
    rt_underflow,
    rt_nofound,
    rt_duplicate
};

/**
 * @brief Punto en espacio N-dimensional
 * @tparam N Número de dimensiones
 * @tparam T Tipo de coordenadas (float, double)
 */
template <size_t N, typename T = double>
struct Point {
    T coords[N];

    /**
     * @brief Constructor por defecto - inicializa en origen
     */
    Point() {
        for (size_t i = 0; i < N; ++i)
            coords[i] = T(0);
    }

    /**
     * @brief Constructor con initializer_list
     * @param list Lista de coordenadas
     */
    Point(std::initializer_list<T> list) {
        size_t i = 0;
        for (auto val : list) {
            if (i >= N) break;
            coords[i++] = val;
        }
        while (i < N) coords[i++] = T(0);
    }

    /**
     * @brief Operador de acceso
     */
    T& operator[](size_t i) { return coords[i]; }
    const T& operator[](size_t i) const { return coords[i]; }

    /**
     * @brief Calcula distancia euclidiana a otro punto
     * @param other Otro punto
     * @return Distancia euclidiana
     */
    T distance(const Point& other) const {
        T sum = 0;
        for (size_t i = 0; i < N; ++i) {
            T diff = coords[i] - other.coords[i];
            sum += diff * diff;
        }
        return std::sqrt(sum);
    }

    /**
     * @brief Operador de igualdad
     */
    bool operator==(const Point& other) const {
        for (size_t i = 0; i < N; ++i) {
            if (coords[i] != other.coords[i])
                return false;
        }
        return true;
    }
};

/**
 * @brief Operador de salida para Point
 */
template <size_t N, typename T>
std::ostream& operator<<(std::ostream& os, const Point<N, T>& point) {
    os << "(";
    for (size_t i = 0; i < N; ++i) {
        os << point.coords[i];
        if (i < N-1) os << ",";
    }
    os << ")";
    return os;
}

/**
 * @brief Rectángulo alineado a los ejes (Axis-Aligned Bounding Box)
 * @tparam N Número de dimensiones
 * @tparam T Tipo de coordenadas
 */
template <size_t N, typename T = double>
struct Rectangle {
    Point<N, T> min;  // Esquina inferior-izquierda (mínimos)
    Point<N, T> max;  // Esquina superior-derecha (máximos)

    /**
     * @brief Constructor por defecto - rectángulo inválido
     */
    Rectangle() {
        for (size_t i = 0; i < N; ++i) {
            min[i] = std::numeric_limits<T>::max();
            max[i] = std::numeric_limits<T>::lowest();
        }
    }

    /**
     * @brief Constructor con dos puntos
     */
    Rectangle(const Point<N, T>& p1, const Point<N, T>& p2)
        : min(p1), max(p2) {}

    /**
     * @brief Calcula el área/volumen del rectángulo
     * @return Área en 2D, volumen en 3D, hiper-volumen en N-D
     */
    T area() const {
        T result = 1;
        for (size_t i = 0; i < N; ++i) {
            T extent = max[i] - min[i];
            if (extent <= 0) return 0;
            result *= extent;
        }
        return result;
    }

    /**
     * @brief Calcula el perímetro (suma de extensiones)
     * @return Suma de (max[i] - min[i]) para todas las dimensiones
     */
    T perimeter() const {
        T result = 0;
        for (size_t i = 0; i < N; ++i)
            result += (max[i] - min[i]);
        return result;
    }

    /**
     * @brief Verifica si este rectángulo intersecta con otro
     * @param other Otro rectángulo
     * @return true si hay intersección
     */
    bool intersects(const Rectangle& other) const {
        for (size_t i = 0; i < N; ++i) {
            if (max[i] < other.min[i] || min[i] > other.max[i])
                return false;
        }
        return true;
    }

    /**
     * @brief Verifica si este rectángulo contiene un punto
     * @param p Punto a verificar
     * @return true si el punto está dentro o en el borde
     */
    bool contains(const Point<N, T>& p) const {
        for (size_t i = 0; i < N; ++i) {
            if (p[i] < min[i] || p[i] > max[i])
                return false;
        }
        return true;
    }

    /**
     * @brief Verifica si este rectángulo contiene completamente a otro
     * @param other Otro rectángulo
     * @return true si other está completamente dentro
     */
    bool contains(const Rectangle& other) const {
        for (size_t i = 0; i < N; ++i) {
            if (other.min[i] < min[i] || other.max[i] > max[i])
                return false;
        }
        return true;
    }

    /**
     * @brief Expande este rectángulo para incluir un punto
     * @param p Punto a incluir
     */
    void expand(const Point<N, T>& p) {
        for (size_t i = 0; i < N; ++i) {
            if (p[i] < min[i]) min[i] = p[i];
            if (p[i] > max[i]) max[i] = p[i];
        }
    }

    /**
     * @brief Expande este rectángulo para incluir otro rectángulo
     * @param other Rectángulo a incluir
     */
    void expand(const Rectangle& other) {
        for (size_t i = 0; i < N; ++i) {
            if (other.min[i] < min[i]) min[i] = other.min[i];
            if (other.max[i] > max[i]) max[i] = other.max[i];
        }
    }

    /**
     * @brief Calcula el incremento de área al expandir con un rectángulo
     * @param other Rectángulo candidato
     * @return Incremento de área
     */
    T enlargement(const Rectangle& other) const {
        Rectangle expanded = *this;
        expanded.expand(other);
        return expanded.area() - area();
    }

    /**
     * @brief Calcula el área de intersección con otro rectángulo
     * @param other Otro rectángulo
     * @return Área de overlap
     */
    T overlap(const Rectangle& other) const {
        if (!intersects(other)) return 0;

        T result = 1;
        for (size_t i = 0; i < N; ++i) {
            T overlap_min = std::max(min[i], other.min[i]);
            T overlap_max = std::min(max[i], other.max[i]);
            result *= (overlap_max - overlap_min);
        }
        return result;
    }

    /**
     * @brief Calcula el centro del rectángulo
     * @return Punto en el centro
     */
    Point<N, T> center() const {
        Point<N, T> c;
        for (size_t i = 0; i < N; ++i)
            c[i] = (min[i] + max[i]) / 2;
        return c;
    }

    /**
     * @brief Verifica si el rectángulo es válido
     * @return true si min[i] <= max[i] para todo i
     */
    bool isValid() const {
        for (size_t i = 0; i < N; ++i) {
            if (min[i] > max[i])
                return false;
        }
        return true;
    }

    /**
     * @brief Operador de igualdad
     */
    bool operator==(const Rectangle& other) const {
        return min == other.min && max == other.max;
    }
};

/**
 * @brief Operador de salida para Rectangle
 */
template <size_t N, typename T>
std::ostream& operator<<(std::ostream& os, const Rectangle<N, T>& rect) {
    os << "[" << rect.min << "-" << rect.max << "]";
    return os;
}

/**
 * @brief Trait para R-Tree
 * @tparam _Dimensions Número de dimensiones espaciales
 * @tparam _CoordType Tipo de coordenadas (float, double)
 * @tparam _ObjIDType Tipo de identificador de objetos
 */
template <size_t _Dimensions = 2,
          typename _CoordType = double,
          typename _ObjIDType = long>
struct RTreeTrait {
    static constexpr size_t Dimensions = _Dimensions;
    using CoordType = _CoordType;
    using ObjIDType = _ObjIDType;
    using PointType = Point<Dimensions, CoordType>;
    using RectType = Rectangle<Dimensions, CoordType>;
};

// Aliases comunes
using RTree2D = RTreeTrait<2, double, long>;
using RTree3D = RTreeTrait<3, double, long>;

/**
 * @brief Información de un objeto espacial indexado
 * @tparam Trait RTreeTrait con configuración de tipos
 */
template <typename Trait>
struct tagRTreeObjectInfo {
    using RectType = typename Trait::RectType;
    using ObjIDType = typename Trait::ObjIDType;

    RectType mbr;      // Minimum Bounding Rectangle del objeto
    ObjIDType ObjID;   // Identificador único del objeto

    /**
     * @brief Constructor por defecto
     */
    tagRTreeObjectInfo() : ObjID(-1) {}

    /**
     * @brief Constructor con MBR y ID
     */
    tagRTreeObjectInfo(const RectType& _mbr, ObjIDType _id)
        : mbr(_mbr), ObjID(_id) {}

    /**
     * @brief Constructor de copia
     */
    tagRTreeObjectInfo(const tagRTreeObjectInfo& other)
        : mbr(other.mbr), ObjID(other.ObjID) {}

    /**
     * @brief Operador de conversión a Rectangle
     */
    operator RectType() const { return mbr; }
};

/**
 * @brief Página del R-Tree (hoja o interna)
 * @tparam Trait RTreeTrait con configuración
 */
template <typename Trait>
class CRTreePage {
    template <typename T>
    friend class RTree;
    template <typename T>
    friend class RTreeForwardIterator;
    template <typename T>
    friend class RTreeBackwardIterator;

public:
    using RectType = typename Trait::RectType;
    using PointType = typename Trait::PointType;
    using ObjIDType = typename Trait::ObjIDType;
    using ObjectInfo = tagRTreeObjectInfo<Trait>;
    using CoordType = typename Trait::CoordType;
    using RTPage = CRTreePage<Trait>;

private:
    std::vector<ObjectInfo> m_Entries;    // Entradas (MBRs + IDs o punteros a hijos)
    std::vector<RTPage*>    m_Children;   // Punteros a hijos (vacío si es hoja)
    RTPage*                 m_pParent;    // Puntero al padre
    RectType                m_MBR;        // MBR que engloba todas las entradas
    bool                    m_IsLeaf;     // true si es hoja
    size_t                  m_MaxEntries; // Capacidad máxima (M)
    size_t                  m_MinEntries; // Mínimo requerido (m)

public:
    /**
     * @brief Constructor
     * @param maxEntries Capacidad máxima de la página (M)
     * @param isLeaf true si es hoja, false si es interna
     */
    CRTreePage(size_t maxEntries, bool isLeaf)
        : m_pParent(nullptr),
          m_IsLeaf(isLeaf),
          m_MaxEntries(maxEntries),
          m_MinEntries(maxEntries / 2) {
        m_Entries.reserve(maxEntries + 1);
        if (!isLeaf)
            m_Children.reserve(maxEntries + 1);
    }

    /**
     * @brief Destructor
     */
    ~CRTreePage() {
        for (auto child : m_Children)
            delete child;
    }

    // Getters básicos
    bool IsLeaf() const { return m_IsLeaf; }
    bool IsFull() const { return m_Entries.size() >= m_MaxEntries; }
    bool IsUnderflow() const { return m_Entries.size() < m_MinEntries; }
    size_t GetNumEntries() const { return m_Entries.size(); }
    const RectType& GetMBR() const { return m_MBR; }
    RTPage* GetParent() const { return m_pParent; }

    /**
     * @brief Recalcula el MBR englobando todas las entradas
     */
    void UpdateMBR() {
        if (m_Entries.empty()) {
            m_MBR = RectType();
            return;
        }

        m_MBR = m_Entries[0].mbr;
        for (size_t i = 1; i < m_Entries.size(); ++i)
            m_MBR.expand(m_Entries[i].mbr);
    }

    /**
     * @brief Agrega una entrada a la página
     * @param entry Entrada a agregar
     * @param child Puntero al hijo (nullptr si es hoja)
     */
    void AddEntry(const ObjectInfo& entry, RTPage* child = nullptr) {
        m_Entries.push_back(entry);
        if (!m_IsLeaf && child != nullptr) {
            m_Children.push_back(child);
            child->m_pParent = this;
        }
        m_MBR.expand(entry.mbr);
    }

    /**
     * @brief Elimina una entrada de la página
     * @param index Índice de la entrada a eliminar
     */
    void RemoveEntry(size_t index) {
        if (index >= m_Entries.size()) return;

        m_Entries.erase(m_Entries.begin() + index);
        if (!m_IsLeaf && index < m_Children.size())
            m_Children.erase(m_Children.begin() + index);

        UpdateMBR();
    }

    /**
     * @brief Selecciona la mejor entrada para insertar un rectángulo
     * @param rect Rectángulo a insertar
     * @return Índice de la mejor entrada
     */
    size_t ChooseBestEntry(const RectType& rect) const {
        if (m_Entries.empty()) return 0;

        size_t bestIndex = 0;
        auto bestEnlargement = m_Entries[0].mbr.enlargement(rect);
        auto bestArea = m_Entries[0].mbr.area();

        for (size_t i = 1; i < m_Entries.size(); ++i) {
            auto enlargement = m_Entries[i].mbr.enlargement(rect);
            auto area = m_Entries[i].mbr.area();

            if (enlargement < bestEnlargement ||
                (enlargement == bestEnlargement && area < bestArea)) {
                bestIndex = i;
                bestEnlargement = enlargement;
                bestArea = area;
            }
        }

        return bestIndex;
    }

    /**
     * @brief Imprime la página para debugging
     */
    void Print(std::ostream& os, size_t level = 0) const {
        std::string indent(level * 2, ' ');
        os << indent << (m_IsLeaf ? "LEAF" : "INTERNAL")
           << " MBR:" << m_MBR
           << " Entries:" << m_Entries.size() << "\n";

        for (size_t i = 0; i < m_Entries.size(); ++i) {
            os << indent << "  [" << i << "] " << m_Entries[i].mbr
               << " ID:" << m_Entries[i].ObjID << "\n";

            if (!m_IsLeaf && i < m_Children.size())
                m_Children[i]->Print(os, level + 1);
        }
    }
};

/**
 * @brief Selecciona las dos entradas más separadas como semillas para split
 * @param entries Vector de entradas
 * @param seed1 [out] Índice de la primera semilla
 * @param seed2 [out] Índice de la segunda semilla
 */
template <typename Trait>
void LinearPickSeeds(const std::vector<tagRTreeObjectInfo<Trait>>& entries,
                    size_t& seed1, size_t& seed2) {
    constexpr size_t D = Trait::Dimensions;
    using CoordType = typename Trait::CoordType;

    seed1 = 0;
    seed2 = 1;
    CoordType maxSeparation = 0;

    for (size_t d = 0; d < D; ++d) {
        CoordType overallMin = entries[0].mbr.min[d];
        CoordType overallMax = entries[0].mbr.max[d];

        for (size_t i = 1; i < entries.size(); ++i) {
            if (entries[i].mbr.min[d] < overallMin)
                overallMin = entries[i].mbr.min[d];
            if (entries[i].mbr.max[d] > overallMax)
                overallMax = entries[i].mbr.max[d];
        }

        CoordType width = overallMax - overallMin;
        if (width == 0) continue;

        size_t highestMinIdx = 0;
        size_t lowestMaxIdx = 0;
        CoordType highestMin = entries[0].mbr.min[d];
        CoordType lowestMax = entries[0].mbr.max[d];

        for (size_t i = 1; i < entries.size(); ++i) {
            if (entries[i].mbr.min[d] > highestMin) {
                highestMin = entries[i].mbr.min[d];
                highestMinIdx = i;
            }
            if (entries[i].mbr.max[d] < lowestMax) {
                lowestMax = entries[i].mbr.max[d];
                lowestMaxIdx = i;
            }
        }

        CoordType separation = (highestMin - lowestMax) / width;

        if (separation > maxSeparation) {
            maxSeparation = separation;
            seed1 = lowestMaxIdx;
            seed2 = highestMinIdx;
        }
    }

    if (seed1 == seed2)
        seed2 = (seed1 + 1) % entries.size();
}

/**
 * @brief Iterador forward para R-Tree (recorrido depth-first)
 * @tparam Trait RTreeTrait
 */
template <typename Trait>
class RTreeForwardIterator {
    using RTPage = CRTreePage<Trait>;
    using ObjectInfo = tagRTreeObjectInfo<Trait>;

private:
    struct StackFrame {
        RTPage* page;
        size_t index;
        StackFrame(RTPage* p, size_t i) : page(p), index(i) {}
    };

    std::vector<StackFrame> m_Stack;
    ObjectInfo* m_pCurrent;

public:
    RTreeForwardIterator(RTPage* root) : m_pCurrent(nullptr) {
        if (root) {
            m_Stack.push_back(StackFrame(root, 0));
            advance();
        }
    }

    RTreeForwardIterator() : m_pCurrent(nullptr) {}

    void advance() {
        while (!m_Stack.empty()) {
            StackFrame& frame = m_Stack.back();

            if (frame.page->IsLeaf()) {
                if (frame.index < frame.page->GetNumEntries()) {
                    m_pCurrent = &frame.page->m_Entries[frame.index];
                    frame.index++;
                    return;
                } else {
                    m_Stack.pop_back();
                }
            } else {
                if (frame.index < frame.page->GetNumEntries()) {
                    RTPage* child = frame.page->m_Children[frame.index];
                    frame.index++;
                    m_Stack.push_back(StackFrame(child, 0));
                } else {
                    m_Stack.pop_back();
                }
            }
        }
        m_pCurrent = nullptr;
    }

    RTreeForwardIterator& operator++() {
        advance();
        return *this;
    }

    ObjectInfo& operator*() { return *m_pCurrent; }
    ObjectInfo* operator->() { return m_pCurrent; }

    bool operator==(const RTreeForwardIterator& other) const {
        return m_pCurrent == other.m_pCurrent;
    }

    bool operator!=(const RTreeForwardIterator& other) const {
        return !(*this == other);
    }
};

/**
 * @brief Iterador backward para R-Tree (recorrido depth-first reverso)
 * @tparam Trait RTreeTrait
 */
template <typename Trait>
class RTreeBackwardIterator {
    using RTPage = CRTreePage<Trait>;
    using ObjectInfo = tagRTreeObjectInfo<Trait>;

private:
    struct StackFrame {
        RTPage* page;
        int index;
        StackFrame(RTPage* p, int i) : page(p), index(i) {}
    };

    std::vector<StackFrame> m_Stack;
    ObjectInfo* m_pCurrent;

public:
    RTreeBackwardIterator(RTPage* root) : m_pCurrent(nullptr) {
        if (root) {
            m_Stack.push_back(StackFrame(root, root->GetNumEntries() - 1));
            advance();
        }
    }

    RTreeBackwardIterator() : m_pCurrent(nullptr) {}

    void advance() {
        while (!m_Stack.empty()) {
            StackFrame& frame = m_Stack.back();

            if (frame.page->IsLeaf()) {
                if (frame.index >= 0) {
                    m_pCurrent = &frame.page->m_Entries[frame.index];
                    frame.index--;
                    return;
                } else {
                    m_Stack.pop_back();
                }
            } else {
                if (frame.index >= 0) {
                    RTPage* child = frame.page->m_Children[frame.index];
                    frame.index--;
                    m_Stack.push_back(StackFrame(child, child->GetNumEntries() - 1));
                } else {
                    m_Stack.pop_back();
                }
            }
        }
        m_pCurrent = nullptr;
    }

    RTreeBackwardIterator& operator++() {
        advance();
        return *this;
    }

    ObjectInfo& operator*() { return *m_pCurrent; }
    ObjectInfo* operator->() { return m_pCurrent; }

    bool operator==(const RTreeBackwardIterator& other) const {
        return m_pCurrent == other.m_pCurrent;
    }

    bool operator!=(const RTreeBackwardIterator& other) const {
        return !(*this == other);
    }
};

#endif // __RTREEPAGE_H__
