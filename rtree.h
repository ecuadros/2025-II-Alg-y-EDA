/**
 * @file rtree.h
 * @brief R-Tree
 * 
 * TODOs:
 *   1. Insert      
 *   2. Remove       
 *   3. RangeQuery    
 *   4. ReadFromDisk  
 *   5. WriteToDisk   
 * 
 */

#ifndef __RTREE_H__
#define __RTREE_H__

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include <string>

/**
 * @brief Trait para configurar tipos del RTree
 * @tparam _CoordType Tipo de coordenadas (int, float, double)
 * @tparam _DataType  Tipo de datos a almacenar
 */
template <typename _CoordType, typename _DataType>
struct RTreeTrait {
    using CoordType = _CoordType;
    using DataType = _DataType;
};

/**
 * @brief MBR - Minimum Bounding Rectangle 
 */
template <typename CoordType>
struct MBR {
    CoordType x_min, y_min, x_max, y_max;
    
    MBR() : x_min(std::numeric_limits<CoordType>::max()),
            y_min(std::numeric_limits<CoordType>::max()),
            x_max(std::numeric_limits<CoordType>::lowest()),
            y_max(std::numeric_limits<CoordType>::lowest()) {}
    
    MBR(CoordType xmin, CoordType ymin, CoordType xmax, CoordType ymax)
        : x_min(xmin), y_min(ymin), x_max(xmax), y_max(ymax) {}
    
    CoordType Area() const {
        return (x_max > x_min && y_max > y_min) ? 
               (x_max - x_min) * (y_max - y_min) : 0;
    }
    
    CoordType EnlargedArea(const MBR& o) const {
        return (std::max(x_max, o.x_max) - std::min(x_min, o.x_min)) *
               (std::max(y_max, o.y_max) - std::min(y_min, o.y_min));
    }
    
    CoordType Enlargement(const MBR& o) const { return EnlargedArea(o) - Area(); }
    
    void Expand(const MBR& o) {
        x_min = std::min(x_min, o.x_min); y_min = std::min(y_min, o.y_min);
        x_max = std::max(x_max, o.x_max); y_max = std::max(y_max, o.y_max);
    }
    
    bool Intersects(const MBR& o) const {
        return !(x_max < o.x_min || x_min > o.x_max ||
                 y_max < o.y_min || y_min > o.y_max);
    }
    
    bool Contains(const MBR& o) const {
        return x_min <= o.x_min && x_max >= o.x_max &&
               y_min <= o.y_min && y_max >= o.y_max;
    }
    
    bool operator==(const MBR& o) const {
        return x_min == o.x_min && y_min == o.y_min &&
               x_max == o.x_max && y_max == o.y_max;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const MBR& r) {
        return os << "[(" << r.x_min << "," << r.y_min << ")-(" 
                  << r.x_max << "," << r.y_max << ")]";
    }
};

// Forward declaration
template <typename Trait> class RTree;

/**
 * @brief Nodo del R-Tree
 * Contiene entries que pueden ser hojas (con datos)
 */
template <typename Trait>
class RTreeNode {
public:
    using CoordType = typename Trait::CoordType;
    using DataType = typename Trait::DataType;
    using Rect = MBR<CoordType>;
    
    struct Entry {
        Rect mbr;
        DataType data;
        RTreeNode* child;
        Entry() : data(), child(nullptr) {}
        Entry(const Rect& r, const DataType& d) : mbr(r), data(d), child(nullptr) {}
        Entry(const Rect& r, RTreeNode* c) : mbr(r), data(), child(c) {}
    };

private:
    std::vector<Entry> entries;     ///< Entradas del nodo
    RTreeNode* parent;              ///< Puntero al padre (nullptr si es raíz)
    size_t maxEntries;              ///< M: máximo de entradas
    size_t minEntries;              ///< m: mínimo de entradas
    bool isLeaf;                    ///< true si es hoja, false si es interno
    friend class RTree<Trait>;

public:
    RTreeNode(size_t M, size_t m, bool leaf = true)
        : parent(nullptr), maxEntries(M), minEntries(m), isLeaf(leaf) {
        entries.reserve(M + 1);
    }
    
    ~RTreeNode() {
        if (!isLeaf) for (auto& e : entries) delete e.child;
    }
    
    bool IsLeaf() const { return isLeaf; }
    bool IsOverflow() const { return entries.size() > maxEntries; }
    bool IsUnderflow() const { return entries.size() < minEntries; }
    size_t Size() const { return entries.size(); }
    
    Rect ComputeMBR() const {
        Rect mbr;
        for (const auto& e : entries) mbr.Expand(e.mbr);
        return mbr;
    }
    
    // ChooseLeaf: selecciona hoja con menor expansión de área
    RTreeNode* ChooseLeaf(const Rect& mbr) {
        if (isLeaf) return this;
        
        RTreeNode* best = nullptr;
        CoordType minEnl = std::numeric_limits<CoordType>::max();
        CoordType minArea = std::numeric_limits<CoordType>::max();
        
        for (auto& e : entries) {
            CoordType enl = e.mbr.Enlargement(mbr);
            CoordType area = e.mbr.Area();
            if (enl < minEnl || (enl == minEnl && area < minArea)) {
                minEnl = enl; minArea = area; best = e.child;
            }
        }
        return best->ChooseLeaf(mbr);
    }
    
    // Split: divide el nodo usando algoritmo cuadrático de Guttman
    RTreeNode* Split() {
        // PickSeeds: encontrar par que maximiza espacio desperdiciado
        size_t seed1 = 0, seed2 = 1;
        CoordType maxWaste = std::numeric_limits<CoordType>::lowest();
        
        for (size_t i = 0; i < entries.size(); ++i) {
            for (size_t j = i + 1; j < entries.size(); ++j) {
                Rect combined = entries[i].mbr;
                combined.Expand(entries[j].mbr);
                CoordType waste = combined.Area() - entries[i].mbr.Area() - entries[j].mbr.Area();
                if (waste > maxWaste) { maxWaste = waste; seed1 = i; seed2 = j; }
            }
        }
        
        // Crear nuevo nodo y distribuir semillas
        RTreeNode* newNode = new RTreeNode(maxEntries, minEntries, isLeaf);
        newNode->parent = parent;
        std::vector<Entry> allEntries = std::move(entries);
        entries.clear();
        
        entries.push_back(allEntries[seed1]);
        newNode->entries.push_back(allEntries[seed2]);
        
        std::vector<bool> assigned(allEntries.size(), false);
        assigned[seed1] = assigned[seed2] = true;
        size_t remaining = allEntries.size() - 2;
        
        // PickNext: distribuir resto de entradas
        while (remaining > 0) {
            // Si un grupo necesita todas las restantes para minEntries
            if (entries.size() + remaining == minEntries) {
                for (size_t i = 0; i < allEntries.size(); ++i)
                    if (!assigned[i]) { entries.push_back(allEntries[i]); assigned[i] = true; }
                break;
            }
            if (newNode->entries.size() + remaining == minEntries) {
                for (size_t i = 0; i < allEntries.size(); ++i)
                    if (!assigned[i]) { newNode->entries.push_back(allEntries[i]); assigned[i] = true; }
                break;
            }
            
            // Elegir entrada con mayor diferencia de preferencia
            Rect mbr1 = ComputeMBR(), mbr2 = newNode->ComputeMBR();
            size_t bestIdx = 0;
            CoordType maxDiff = std::numeric_limits<CoordType>::lowest();
            bool assignToFirst = true;
            
            for (size_t i = 0; i < allEntries.size(); ++i) {
                if (assigned[i]) continue;
                CoordType d1 = mbr1.Enlargement(allEntries[i].mbr);
                CoordType d2 = mbr2.Enlargement(allEntries[i].mbr);
                if (std::abs(d1 - d2) > maxDiff) {
                    maxDiff = std::abs(d1 - d2); bestIdx = i; assignToFirst = (d1 <= d2);
                }
            }
            
            (assignToFirst ? entries : newNode->entries).push_back(allEntries[bestIdx]);
            assigned[bestIdx] = true;
            --remaining;
        }
        
        // Actualizar padres si es nodo interno
        if (!isLeaf) {
            for (auto& e : entries) if (e.child) e.child->parent = this;
            for (auto& e : newNode->entries) if (e.child) e.child->parent = newNode;
        }
        return newNode;
    }
    
    // RangeQuery: búsqueda por rango
    void RangeQuery(const Rect& query, std::vector<std::pair<Rect, DataType>>& results) const {
        for (const auto& e : entries) {
            if (e.mbr.Intersects(query)) {
                if (isLeaf) results.push_back({e.mbr, e.data});
                else e.child->RangeQuery(query, results);
            }
        }
    }
    
    // FindLeaf: encuentra nodo hoja que contiene una entrada
    RTreeNode* FindLeaf(const Rect& mbr, const DataType& data, size_t& idx) {
        if (isLeaf) {
            for (size_t i = 0; i < entries.size(); ++i) {
                if (entries[i].mbr == mbr && entries[i].data == data) {
                    idx = i; return this;
                }
            }
            return nullptr;
        }
        for (auto& e : entries) {
            if (e.mbr.Contains(mbr)) {
                RTreeNode* found = e.child->FindLeaf(mbr, data, idx);
                if (found) return found;
            }
        }
        return nullptr;
    }
    
    // CollectEntries: recolecta entradas hoja para reinserción
    void CollectEntries(std::vector<std::pair<Rect, DataType>>& result) {
        if (isLeaf) {
            for (const auto& e : entries) result.push_back({e.mbr, e.data});
        } else {
            for (auto& e : entries) e.child->CollectEntries(result);
        }
    }
    
    // Write: escribe nodo a stream
    void Write(std::ostream& os) const {
        os << isLeaf << " " << entries.size() << "\n";
        for (const auto& e : entries) {
            os << e.mbr.x_min << " " << e.mbr.y_min << " "
               << e.mbr.x_max << " " << e.mbr.y_max << " ";
            if (isLeaf) os << e.data << "\n";
            else { os << "\n"; e.child->Write(os); }
        }
    }
    
    // Read: lee nodo desde stream
    void Read(std::istream& is) {
        size_t count;
        is >> isLeaf >> count;
        entries.clear();
        
        for (size_t i = 0; i < count; ++i) {
            Entry e;
            is >> e.mbr.x_min >> e.mbr.y_min >> e.mbr.x_max >> e.mbr.y_max;
            if (isLeaf) {
                is >> e.data; e.child = nullptr;
            } else {
                e.child = new RTreeNode(maxEntries, minEntries, true);
                e.child->parent = this;
                e.child->Read(is);
            }
            entries.push_back(e);
        }
    }
    
    // Print: imprime nodo recursivamente
    void Print(std::ostream& os, int level = 0) const {
        std::string indent(level * 2, ' ');
        os << indent << "[" << (isLeaf ? "HOJA" : "INTERNO") << "] " 
           << ComputeMBR() << " (n=" << entries.size() << ")\n";
        
        for (size_t i = 0; i < entries.size(); ++i) {
            os << indent << "  " << entries[i].mbr;
            if (isLeaf) os << " -> data=" << entries[i].data << "\n";
            else { os << "\n"; entries[i].child->Print(os, level + 2); }
        }
    }
};

/**
 * @brief R-Tree 
 * @tparam Trait Configuración de tipos (RTreeTrait)
*/
template <typename Trait>
class RTree {
public:
    using CoordType = typename Trait::CoordType;
    using DataType = typename Trait::DataType;
    using Rect = MBR<CoordType>;
    using Node = RTreeNode<Trait>;
    using Entry = typename Node::Entry;

private:
    Node* root;           ///< Raíz del árbol
    size_t maxEntries;    ///< M: máximo entradas por nodo
    size_t minEntries;    ///< m: mínimo entradas por nodo  
    size_t height;        ///< Altura del árbol
    size_t numEntries;    ///< Total de elementos insertados

public:
    // Constructor: M = max entradas, m = min entradas
    RTree(size_t M = 4, size_t m = 2) 
        : maxEntries(M), minEntries(m), height(1), numEntries(0) {
        root = new Node(M, m, true);
    }
    
    ~RTree() { delete root; }
    
    size_t Size() const { return numEntries; }
    size_t Height() const { return height; }

    // 1. INSERCIÓN
    void Insert(CoordType x1, CoordType y1, CoordType x2, CoordType y2, 
                const DataType& data) {
        Rect mbr(x1, y1, x2, y2);
        Node* leaf = root->ChooseLeaf(mbr);
        leaf->entries.push_back(Entry(mbr, data));
        AdjustTree(leaf);
        ++numEntries;
    }

private:
    // AdjustTree: propaga cambios hacia la raíz
    void AdjustTree(Node* node) {
        Node* splitNode = node->IsOverflow() ? node->Split() : nullptr;
        
        if (node == root) {
            if (splitNode) {
                Node* newRoot = new Node(maxEntries, minEntries, false);
                newRoot->entries.push_back(Entry(node->ComputeMBR(), node));
                newRoot->entries.push_back(Entry(splitNode->ComputeMBR(), splitNode));
                node->parent = splitNode->parent = newRoot;
                root = newRoot;
                ++height;
            }
            return;
        }
        
        Node* parent = node->parent;
        for (auto& e : parent->entries)
            if (e.child == node) { e.mbr = node->ComputeMBR(); break; }
        
        if (splitNode) {
            parent->entries.push_back(Entry(splitNode->ComputeMBR(), splitNode));
            splitNode->parent = parent;
        }
        AdjustTree(parent);
    }

public:
    // 2. BORRADO
     bool Remove(CoordType x1, CoordType y1, CoordType x2, CoordType y2, 
                const DataType& data) {
        Rect mbr(x1, y1, x2, y2);
        size_t idx;
        Node* leaf = root->FindLeaf(mbr, data, idx);
        if (!leaf) return false;
        
        leaf->entries.erase(leaf->entries.begin() + idx);
        
        std::vector<std::pair<Rect, DataType>> orphans;
        CondenseTree(leaf, orphans);
        
        // Reducir altura si raíz tiene un solo hijo
        while (!root->IsLeaf() && root->Size() == 1) {
            Node* oldRoot = root;
            root = root->entries[0].child;
            root->parent = nullptr;
            oldRoot->entries.clear();
            delete oldRoot;
            --height;
        }
        
        // Reinsertar huérfanos
        for (const auto& o : orphans) {
            Insert(o.first.x_min, o.first.y_min, o.first.x_max, o.first.y_max, o.second);
            --numEntries;
        }
        --numEntries;
        return true;
    }

private:
    // CondenseTree: rebalancea después de eliminación
    void CondenseTree(Node* node, std::vector<std::pair<Rect, DataType>>& orphans) {
        if (node == root) return;
        Node* parent = node->parent;
        
        if (node->IsUnderflow()) {
            node->CollectEntries(orphans);
            for (auto it = parent->entries.begin(); it != parent->entries.end(); ++it) {
                if (it->child == node) { parent->entries.erase(it); break; }
            }
            if (!node->isLeaf) node->entries.clear();
            delete node;
        } else {
            for (auto& e : parent->entries)
                if (e.child == node) { e.mbr = node->ComputeMBR(); break; }
        }
        CondenseTree(parent, orphans);
    }

public:
    // 3. RANGE QUERY
    std::vector<std::pair<Rect, DataType>> RangeQuery(
            CoordType x1, CoordType y1, CoordType x2, CoordType y2) const {
        std::vector<std::pair<Rect, DataType>> results;
        root->RangeQuery(Rect(x1, y1, x2, y2), results);
        return results;
    }
    
    std::vector<std::pair<Rect, DataType>> SearchPoint(CoordType x, CoordType y) const {
        return RangeQuery(x, y, x, y);
    }

    // 4. READ FROM DISK
    void ReadFromDisk(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) throw std::runtime_error("No se pudo abrir: " + filename);
        
        delete root;
        file >> maxEntries >> minEntries >> height >> numEntries;
        root = new Node(maxEntries, minEntries, true);
        root->Read(file);
        file.close();
        std::cout << "R-Tree cargado desde '" << filename << "' (" << numEntries << " entradas)\n";
    }

    // 5. WRITE TO DISK
    void WriteToDisk(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) throw std::runtime_error("No se pudo crear: " + filename);
        
        file << maxEntries << " " << minEntries << " " << height << " " << numEntries << "\n";
        root->Write(file);
        file.close();
        std::cout << "R-Tree guardado en '" << filename << "' (" << numEntries << " entradas)\n";
    }

    // Imprimir árbol
    void Print(std::ostream& os = std::cout) const {
        os << "\n=== R-TREE [M=" << maxEntries << ", m=" << minEntries 
           << ", h=" << height << ", n=" << numEntries << "] ===\n";
        root->Print(os);
        os << "==================\n";
    }
};

#endif 