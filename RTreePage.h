#ifndef __RTREEPAGE_H__
#define __RTREEPAGE_H__

#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <string>

template <typename T>
struct Point2D {
    T x, y;
    bool operator==(const Point2D& other) const {
        return x == other.x && y == other.y;
    }

    double distSq(const Point2D& other) const {
        T dx = x - other.x;
        T dy = y - other.y;
        return static_cast<double>(dx * dx + dy * dy);
    }
};

template <typename T>
struct Rectangle {  
    T x_min, y_min, x_max, y_max;
    
    Rectangle() { reset(); }
    Rectangle(T x1, T y1, T x2, T y2) 
        : x_min(std::min(x1, x2)), y_min(std::min(y1, y2)),
          x_max(std::max(x1, x2)), y_max(std::max(y1, y2)) {}
    Rectangle(const Point2D<T>& p) 
        : x_min(p.x), y_min(p.y), x_max(p.x), y_max(p.y) {}

    void reset() {
        x_min = std::numeric_limits<T>::max();
        y_min = std::numeric_limits<T>::max();
        x_max = std::numeric_limits<T>::lowest();
        y_max = std::numeric_limits<T>::lowest();
    }

    T area() const {
        return (x_max - x_min) * (y_max - y_min);
    }

    double distSq(const Point2D<T>& point) const {
        T dx = std::max({x_min - point.x, point.x - x_max});
        T dy = std::max({y_min - point.y, point.y - y_max});
        return static_cast<double>(dx * dx + dy * dy);
    }

    bool intersects(const Rectangle& other) const {
        return !(x_max < other.x_min || x_min > other.x_max ||
                 y_max < other.y_min || y_min > other.y_max);
    }

    bool contains(const Point2D<T>& point) const {
        return point.x >= x_min && point.x <= x_max &&
               point.y >= y_min && point.y <= y_max;
    }

    void extends(const Rectangle& other) {
        x_min = std::min(x_min, other.x_min);
        y_min = std::min(y_min, other.y_min);
        x_max = std::max(x_max, other.x_max);
        y_max = std::max(y_max, other.y_max);
    }

    T enlargment(const Rectangle& other) const {
        Rectangle expanded = *this;
        expanded.extends(other);
        return expanded.area() - this->area();
    }
};

// Codigos
enum rt_ErrorCode{
    rt_ok,
    rt_overflow
};

// Operadores de entrada y salida
template <typename T>
std::ostream& operator<<(std::ostream& os, const Point2D<T>& p) {
    os << p.x << " " << p.y;
    return os;
}

template <typename T>
std::istream& operator>>(std::istream& is, Point2D<T>& p) {
    is >> p.x >> p.y;
    return is;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Rectangle<T>& r) {
    os << r.x_min << " " << r.y_min << " " << r.x_max << " " << r.y_max;
    return os;
}

template <typename T>
std::istream& operator>>(std::istream& is, Rectangle<T>& r) {
    is >> r.x_min >> r.y_min >> r.x_max >> r.y_max;
    return is;
}

// Traits

template <typename _CoordType, typename _ObjIDType>
struct RTreeTraits {
    using CoordType     = _CoordType;
    using ObjIDType     = _ObjIDType;
    using PointType     = Point2D<CoordType>;
    using RectType      = Rectangle<CoordType>;
};

// R-TREE PAGE 

template <typename Traits> class RTree;

template <typename Traits>
class RTreePage {
    public:
    using PointType = typename Traits::PointType;
    using RectType  = typename Traits::RectType;
    using ObjIDType = typename Traits::ObjIDType;

    using ObjectInfo = std::pair<PointType, ObjIDType>;

    struct Neighbour { // Para KNN
        double distSq;
        ObjIDType dataId;
        PointType point;

        bool operator<(const Neighbour& other) const {
            return distSq < other.distSq;
        }
    };

    struct Branch { // Nodo
        RectType mbr;
        RTreePage* child;
        ObjIDType dataId;
        PointType point;

        Branch() : child(nullptr) {}
        Branch(PointType p, ObjIDType id) : mbr(p), child(nullptr), dataId(id), point(p) {}
        Branch(RTreePage* c, RectType r) : mbr(r), child(c), dataId(0), point({0,0}) {}
    };

    bool m_isLeaf;
    std::vector<Branch> m_branches;
    RectType m_NodeMBR;
    size_t m_maxEntries;

    public:
    RTreePage(bool isLeaf, size_t order) : m_isLeaf(isLeaf), m_maxEntries(order) {
        m_branches.reserve(order + 1);
        m_NodeMBR.reset();
    }

    ~RTreePage() {
        for (auto& branch : m_branches) {
            if (branch.child) {
                delete branch.child;
            }
        }
    }

    void updateMBR() {
        m_NodeMBR.reset();
        for (const auto& branch : m_branches) {
            m_NodeMBR.extends(branch.mbr);
        }
    }

    rt_ErrorCode    insert(const Branch& branch, RTreePage** newPage, RectType* newMBR);
    void            RangeQuery(const RectType& region, std::vector<ObjectInfo>& results);
    void            KNN(const PointType& point, size_t k, std::vector<Neighbour>& results);

    bool            Remove (const PointType& point, const ObjIDType& id, bool& isFound);
    void            WriteToDisk(std::ostream& os, std::string indent);
    void            ReadFromText(std::istream& is);
    
    private:
    int PickBranch(const RectType& rect);
    void Split(RTreePage** newPage, RectType& newMBR);

    template <typename T> friend class RTree;
};

template <typename Traits>
rt_ErrorCode RTreePage<Traits>::insert(const Branch& branch, RTreePage** newPage, RectType* newMBR) {
    if (m_isLeaf){
        m_branches.push_back(branch);
        updateMBR();
    } else {
        int best = PickBranch(branch.mbr);
        RTreePage* pagePair = nullptr;
        RectType pairMBR;

        rt_ErrorCode res = m_branches[best].child->insert(branch, &pagePair, &pairMBR);
        m_branches[best].mbr = m_branches[best].child->m_NodeMBR;
        updateMBR();

        if (res == rt_overflow) {
            Branch newBranch(pagePair, pairMBR);
            m_branches.push_back(newBranch);
            updateMBR();
        } else {
            return rt_ok;
        }
    }

    if (m_branches.size() > m_maxEntries) {
        Split(newPage, *newMBR);
        return rt_overflow;
    }
    return rt_ok;
}

template <typename Traits>
void RTreePage<Traits>::RangeQuery(const RectType& region, std::vector<ObjectInfo>& results) {
    if (!m_NodeMBR.intersects(region)) {
        return;
    }

    for (const auto& branch : m_branches) {
        if (m_isLeaf) {
            if (region.contains(branch.point)) {
                results.push_back({branch.point, branch.dataId});
            }
        } else {
            if (branch.mbr.intersects(region)) {
                branch.child->RangeQuery(region, results);
            }
        }
    }
}

template <typename Traits>
void RTreePage<Traits>::KNN(const PointType& point, size_t k, std::vector<Neighbour>& results) {
    if (results.size() == k) {
        double distToNode = m_NodeMBR.distSq(point);
        if (distToNode > results.back().distSq) {
            return;
        }
    }

    // Ordenar hijos
    std::vector<std::pair<double, size_t>> childrenDist;
    childrenDist.reserve(m_branches.size());
    for (size_t i = 0; i < m_branches.size(); ++i) {
        double dist;
        if (m_isLeaf) {
            dist = m_branches[i].point.distSq(point);
        } else {
            dist = m_branches[i].mbr.distSq(point);
        }
        childrenDist.push_back({dist, i});
    }

    std::sort(childrenDist.begin(), childrenDist.end());
        
    // Busqueda recursiva
    for (const auto& child : childrenDist) {
        double dist = child.first;
        size_t idx = child.second;
            
        if (m_isLeaf) {
            if (results.size() < k || dist < results.back().distSq) {
                Neighbour n;
                n.distSq = dist;
                n.dataId = m_branches[idx].dataId;
                n.point = m_branches[idx].point;

                results.push_back(n);

                std::sort(results.begin(), results.end());

                if (results.size() > k) {
                    results.pop_back();
                }
            }
        } else {
            if (results.size() < k || dist < results.back().distSq) {
                m_branches[idx].child->KNN(point, k, results);
            }
        }
    }
}

template <typename Traits>
bool RTreePage<Traits>::Remove(const PointType& point, const ObjIDType& id, bool& isFound) {
    if (!m_NodeMBR.contains(point)) {
        return false;
    }

    if (m_isLeaf) {
        for (auto it = m_branches.begin(); it != m_branches.end(); ++it) {
            if (it->point == point && it->dataId == id) {
                m_branches.erase(it);
                updateMBR();
                isFound = true;
                return m_branches.empty();
            }
        }
    } else {
        for (auto   it = m_branches.begin(); it != m_branches.end(); ) {
            if (it ->mbr.contains(point)) {
                bool childEmpty = it->child->Remove(point, id, isFound);
                
                if (isFound) {
                    if (childEmpty) {
                        delete it->child;
                        it = m_branches.erase(it);
                    } else {
                        it -> mbr = it->child->m_NodeMBR;
                        ++it;
                    }

                    updateMBR();
                    return m_branches.empty();
                } else {
                    ++it;
                }
            } else {
                ++it;
            }
        }
    }
    return m_branches.empty();
}

template <typename Traits>
void RTreePage<Traits>::WriteToDisk(std::ostream& os, std::string indent) {
        os << indent << "Node {" << "\n";
        os << indent << "\tisLeaf: " << m_isLeaf << "\n";
        os << indent << "\tNumBranches: " << m_branches.size() << "\n";

        for (const auto& branch : m_branches) {
            os << indent << "\tBranch {" << "\n";
            os << indent << "\t\tMBR: " << branch.mbr << "\n";
            if (m_isLeaf) {
                os << indent << "\t\tPoint: " << branch.point << "\n";
                os << indent << "\t\tDataID: " << branch.dataId << "\n";
            } else {
                os << indent << "\t\tChildNode: " << "\n";
                branch.child->WriteToDisk(os, indent + "\t\t\t");
            }
            os << indent << "\t}" << "\n";
        }
        os << indent << "}" << "\n";
    }

template <typename Traits>
void RTreePage<Traits>::ReadFromText(std::istream& is) {
    std::string label, relleno; 
    
    // "Node" + "{"
    is >> label >> relleno; 

    // "IsLeaf:" + bool
    is >> label >> m_isLeaf;
    
    // "Count:" + numero
    size_t count;
    is >> label >> count;

    m_branches.clear();
    m_branches.reserve(count);

    for (size_t i = 0; i < count; ++i) {
        Branch b;
        // "Branch" + "{"
        is >> label >> relleno;

        // "MBR:" + rect
        is >> label >> b.mbr;

        if (m_isLeaf) {
            // "Point:" + x y 
            is >> label >> b.point;
            // "ID:" + id
            is >> label >> b.dataId;
            b.child = nullptr;
        } else {
            // "Child:
            is >> label;
            b.child = new RTreePage(false, m_maxEntries);
            b.child->ReadFromText(is);
            b.dataId = 0;
        }
        // "}"
        is >> relleno;
        m_branches.push_back(b);
    }
    // "}"
    is >> relleno;
    updateMBR();
}

template <typename Traits>
int RTreePage<Traits>::PickBranch(const RectType& rect) {
    double minEnlargement = std::numeric_limits<double>::max();
    double minArea = std::numeric_limits<double>::max();
    int best = 0;
    for (size_t i = 0; i < m_branches.size(); ++i) {
        double enlargement = m_branches[i].mbr.enlargment(rect);
        double area = m_branches[i].mbr.area();
        if (enlargement < minEnlargement || (enlargement == minEnlargement && area < minArea)) {
            minEnlargement = enlargement;
            minArea = area;
            best = i;
        }
    }
    return best;
}

template <typename Traits>
void RTreePage<Traits>::Split(RTreePage** newPage, RectType& newMBR) {
    *newPage = new RTreePage(m_isLeaf, m_maxEntries);
    size_t splitPoint = m_branches.size() / 2;
    for (size_t i = splitPoint; i < m_branches.size(); ++i) {
        (*newPage)->m_branches.push_back(m_branches[i]);
    }
    m_branches.resize(splitPoint);
    updateMBR();
    (*newPage)->updateMBR();
    newMBR = (*newPage)->m_NodeMBR;
}

#endif // __RTREEPAGE_H__