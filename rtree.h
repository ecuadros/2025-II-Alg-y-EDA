/**
 * @file Rtree.h
 * @brief R-Tree N-Dimensional
 * 
 * TODOs:
 *   1. Insert      
 *   2. Remove       
 *   3. RangeQuery    
 *   4. ReadFromDisk  
 *   5. WriteToDisk   
 */

#ifndef __RTREE_H__
#define __RTREE_H__

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <algorithm>
#include <limits>
#include <cmath>
#include <string>
#include <stdexcept>

/**
 * @brief Trait para configurar el RTree
 */
template <typename _CoordType, typename _DataType, size_t _N>
struct RTreeTrait {
    using CoordType = _CoordType;
    using DataType = _DataType;
    static const size_t N = _N;
};

/**
 * @brief MBR N-Dimensional (Minimum Bounding Rectangle)
 */
template <typename CoordType, size_t N>
struct MBR {
    std::array<CoordType, N> minCoord;
    std::array<CoordType, N> maxCoord;
    
    MBR() {
        minCoord.fill(std::numeric_limits<CoordType>::max());
        maxCoord.fill(std::numeric_limits<CoordType>::lowest());
    }
    
    MBR(const std::array<CoordType, N>& minC, const std::array<CoordType, N>& maxC)
        : minCoord(minC), maxCoord(maxC) {}
    
    // Área 
    CoordType Area() const {
        CoordType area = 1;
        for (size_t i = 0; i < N; ++i) {
            if (maxCoord[i] <= minCoord[i]) return 0;
            area *= (maxCoord[i] - minCoord[i]);
        }
        return area;
    }
    
    // Área combinada con otro MBR
    CoordType EnlargedArea(const MBR& o) const {
        CoordType area = 1;
        for (size_t i = 0; i < N; ++i) {
            area *= (std::max(maxCoord[i], o.maxCoord[i]) - 
                     std::min(minCoord[i], o.minCoord[i]));
        }
        return area;
    }
    
    // Enlargement: cuánto crece el área 
    CoordType Enlargement(const MBR& o) const { 
        return EnlargedArea(o) - Area(); 
    }
    
    // Expand: expande para contener otro MBR
    void Expand(const MBR& o) {
        for (size_t i = 0; i < N; ++i) {
            minCoord[i] = std::min(minCoord[i], o.minCoord[i]);
            maxCoord[i] = std::max(maxCoord[i], o.maxCoord[i]);
        }
    }
    
    // Intersects: verifica solapamiento
    bool Intersects(const MBR& o) const {
        for (size_t i = 0; i < N; ++i) {
            if (maxCoord[i] < o.minCoord[i] || minCoord[i] > o.maxCoord[i])
                return false;
        }
        return true;
    }
    
    // Contains: verifica contención completa
    bool Contains(const MBR& o) const {
        for (size_t i = 0; i < N; ++i) {
            if (minCoord[i] > o.minCoord[i] || maxCoord[i] < o.maxCoord[i])
                return false;
        }
        return true;
    }
    
    bool operator==(const MBR& o) const {
        return minCoord == o.minCoord && maxCoord == o.maxCoord;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const MBR& r) {
        os << "[(";
        for (size_t i = 0; i < N; ++i) {
            os << r.minCoord[i];
            if (i < N-1) os << ",";
        }
        os << ")-(";
        for (size_t i = 0; i < N; ++i) {
            os << r.maxCoord[i];
            if (i < N-1) os << ",";
        }
        os << ")]";
        return os;
    }
};

template <typename Trait> class RTree;

/**
 * @brief Nodo del R-Tree N-Dimensional
 */
template <typename Trait>
class RTreeNode {
public:
    using CoordType = typename Trait::CoordType;
    using DataType = typename Trait::DataType;
    static const size_t N = Trait::N;
    using Rect = MBR<CoordType, N>;
    
    struct Entry {
        Rect mbr;
        DataType data;
        RTreeNode* child;
        Entry() : data(), child(nullptr) {}
        Entry(const Rect& r, const DataType& d) : mbr(r), data(d), child(nullptr) {}
        Entry(const Rect& r, RTreeNode* c) : mbr(r), data(), child(c) {}
    };

private:
    std::vector<Entry> entries;
    RTreeNode* parent;
    size_t maxEntries, minEntries;
    bool isLeaf;
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
    
    // ChooseLeaf: Selecciona hoja para inserción
    RTreeNode* ChooseLeaf(const Rect& mbr) {
        if (isLeaf) return this;
        
        RTreeNode* best = nullptr;
        CoordType minEnl = std::numeric_limits<CoordType>::max();
        CoordType minArea = std::numeric_limits<CoordType>::max();
        
        for (auto& e : entries) {
            CoordType enl = e.mbr.Enlargement(mbr);
            CoordType area = e.mbr.Area();
            // Desempate por menor área
            if (enl < minEnl || (enl == minEnl && area < minArea)) {
                minEnl = enl; minArea = area; best = e.child;
            }
        }
        return best->ChooseLeaf(mbr);
    }
    
    //
    // Split: Divide nodo usando algoritmo cuadrático
    // Incluye PickSeeds y PickNext 
    //
    RTreeNode* Split() {
        // PickSeeds : encontrar par con mayor desperdicio
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
        
        // Crear nuevo nodo y asignar semillas a cada grupo
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
            // QS2: Si un grupo necesita todas las restantes para tener m
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
            CoordType bestD1 = 0, bestD2 = 0;
            
            for (size_t i = 0; i < allEntries.size(); ++i) {
                if (assigned[i]) continue;
                CoordType d1 = mbr1.Enlargement(allEntries[i].mbr);
                CoordType d2 = mbr2.Enlargement(allEntries[i].mbr);
                CoordType diff = std::abs(d1 - d2);
                if (diff > maxDiff) {
                    maxDiff = diff; bestIdx = i; bestD1 = d1; bestD2 = d2;
                }
            }
            
            // Asignar al grupo con menor enlargement, desempatar por área y cantidad
            if (bestD1 < bestD2) {
                entries.push_back(allEntries[bestIdx]);
            } else if (bestD2 < bestD1) {
                newNode->entries.push_back(allEntries[bestIdx]);
            } else {
                // Desempate por área, luego por cantidad de entradas
                CoordType area1 = mbr1.Area(), area2 = mbr2.Area();
                if (area1 < area2 || (area1 == area2 && entries.size() <= newNode->entries.size()))
                    entries.push_back(allEntries[bestIdx]);
                else
                    newNode->entries.push_back(allEntries[bestIdx]);
            }
            assigned[bestIdx] = true;
            --remaining;
        }
        
        // Actualizar punteros padre en nodos internos
        if (!isLeaf) {
            for (auto& e : entries) if (e.child) e.child->parent = this;
            for (auto& e : newNode->entries) if (e.child) e.child->parent = newNode;
        }
        return newNode;
    }
    
    //
    // RangeQuery: Búsqueda por rango
    //
    void RangeQuery(const Rect& query, std::vector<std::pair<Rect, DataType>>& results) const {
        for (const auto& e : entries) {
            if (e.mbr.Intersects(query)) {
                if (isLeaf) results.push_back({e.mbr, e.data});
                else e.child->RangeQuery(query, results);
            }
        }
    }
    
    //
    // FindLeaf: Encuentra hoja que contiene entrada
    //
    RTreeNode* FindLeaf(const Rect& mbr, const DataType& data, size_t& idx) {
        if (isLeaf) {
            for (size_t i = 0; i < entries.size(); ++i) {
                if (entries[i].mbr == mbr && entries[i].data == data) {
                    idx = i; return this;
                }
            }
            return nullptr;
        }
        // Usar Intersects como dice el paper
        for (auto& e : entries) {
            if (e.mbr.Intersects(mbr)) {
                RTreeNode* found = e.child->FindLeaf(mbr, data, idx);
                if (found) return found;
            }
        }
        return nullptr;
    }
    
    // CollectEntries: recolecta entradas para reinserción 
    void CollectEntries(std::vector<std::pair<Rect, DataType>>& result) {
        if (isLeaf) {
            for (const auto& e : entries) result.push_back({e.mbr, e.data});
        } else {
            for (auto& e : entries) e.child->CollectEntries(result);
        }
    }
    
    void Write(std::ostream& os) const {
        os << isLeaf << " " << entries.size() << "\n";
        for (const auto& e : entries) {
            for (size_t i = 0; i < N; ++i) os << e.mbr.minCoord[i] << " ";
            for (size_t i = 0; i < N; ++i) os << e.mbr.maxCoord[i] << " ";
            if (isLeaf) os << e.data << "\n";
            else { os << "\n"; e.child->Write(os); }
        }
    }
    
    void Read(std::istream& is) {
        size_t count;
        is >> isLeaf >> count;
        entries.clear();
        
        for (size_t i = 0; i < count; ++i) {
            Entry e;
            for (size_t d = 0; d < N; ++d) is >> e.mbr.minCoord[d];
            for (size_t d = 0; d < N; ++d) is >> e.mbr.maxCoord[d];
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
    
    void Print(std::ostream& os, int level = 0) const {
        std::string indent(level * 2, ' ');
        os << indent << "[" << (isLeaf ? "Hoja" : "Interno") << "] " 
           << ComputeMBR() << " (n=" << entries.size() << ")\n";
        
        for (size_t i = 0; i < entries.size(); ++i) {
            os << indent << "  " << entries[i].mbr;
            if (isLeaf) os << " -> " << entries[i].data << "\n";
            else { os << "\n"; entries[i].child->Print(os, level + 2); }
        }
    }
};

/**
 * @brief R-Tree N-Dimensional
 */
template <typename Trait>
class RTree {
public:
    using CoordType = typename Trait::CoordType;
    using DataType = typename Trait::DataType;
    static const size_t N = Trait::N;
    using Rect = MBR<CoordType, N>;
    using Node = RTreeNode<Trait>;
    using Entry = typename Node::Entry;
    using Point = std::array<CoordType, N>;

private:
    Node* root;
    size_t maxEntries, minEntries, height, numEntries;

public:
    
    // Constructor
    RTree(size_t M = 4, size_t m = 2) 
        : maxEntries(M), minEntries(m), height(1), numEntries(0) {
        if (m > M/2) 
            throw std::invalid_argument("Error: m debe ser <= M/2 ");
        if (M < 2)
            throw std::invalid_argument("Error: M debe ser >= 2");
        root = new Node(M, m, true);
    }
    
    ~RTree() { delete root; }
    
    size_t Size() const { return numEntries; }
    size_t Height() const { return height; }

    //
    // Insert: Inserta nueva entrada
    //
    void Insert(const Point& minP, const Point& maxP, const DataType& data) {
        Rect mbr(minP, maxP);
        Node* leaf = root->ChooseLeaf(mbr);  
        leaf->entries.push_back(Entry(mbr, data)); 
        AdjustTree(leaf);  
        // Crecimiento de raíz se maneja en AdjustTree
        ++numEntries;
    }

private:
    //
    // AdjustTree: Propaga cambios hacia la raíz
    //
    void AdjustTree(Node* node) {
        Node* splitNode = node->IsOverflow() ? node->Split() : nullptr;
        
        // Si es raíz, verificar si hay split
        if (node == root) {
            if (splitNode) {
                // Crear nueva raíz
                Node* newRoot = new Node(maxEntries, minEntries, false);
                newRoot->entries.push_back(Entry(node->ComputeMBR(), node));
                newRoot->entries.push_back(Entry(splitNode->ComputeMBR(), splitNode));
                node->parent = splitNode->parent = newRoot;
                root = newRoot;
                ++height;
            }
            return;
        }
        
        // Ajustar MBR del padre
        Node* parent = node->parent;
        for (auto& e : parent->entries)
            if (e.child == node) { e.mbr = node->ComputeMBR(); break; }
        
        // Propagar split
        if (splitNode) {
            parent->entries.push_back(Entry(splitNode->ComputeMBR(), splitNode));
            splitNode->parent = parent;
        }
        
        // Subir al siguiente nivel
        AdjustTree(parent);
    }

public:
    //
    // Remove: Elimina entrada del árbol
    //
    bool Remove(const Point& minP, const Point& maxP, const DataType& data) {
        Rect mbr(minP, maxP);
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
        
        // Reinsertar entradas de nodos eliminados
        for (const auto& o : orphans) {
            Insert(o.first.minCoord, o.first.maxCoord, o.second);
            --numEntries;  // Compensar incremento de Insert
        }
        --numEntries;
        return true;
    }

private:
    //
    // CondenseTree: Rebalancea después de eliminación
    //
    void CondenseTree(Node* node, std::vector<std::pair<Rect, DataType>>& orphans) {
        if (node == root) return;  
        Node* parent = node->parent;
        
        // Eliminar nodo con underflow
        if (node->IsUnderflow()) {
            node->CollectEntries(orphans);
            for (auto it = parent->entries.begin(); it != parent->entries.end(); ++it) {
                if (it->child == node) { parent->entries.erase(it); break; }
            }
            if (!node->isLeaf) node->entries.clear();
            delete node;
        } else {
            // Ajustar MBR
            for (auto& e : parent->entries)
                if (e.child == node) { e.mbr = node->ComputeMBR(); break; }
        }
        // Subir nivel
        CondenseTree(parent, orphans);
    }

public:
    //
    // RangeQuery: Búsqueda espacial
    //
    std::vector<std::pair<Rect, DataType>> RangeQuery(const Point& minP, const Point& maxP) const {
        std::vector<std::pair<Rect, DataType>> results;
        root->RangeQuery(Rect(minP, maxP), results);
        return results;
    }

    void ReadFromDisk(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) throw std::runtime_error("No se pudo abrir: " + filename);
        
        size_t dims;
        delete root;
        file >> dims >> maxEntries >> minEntries >> height >> numEntries;
        root = new Node(maxEntries, minEntries, true);
        root->Read(file);
        file.close();
        std::cout << "R-Tree " << N << "D cargado (" << numEntries << " entradas)\n";
    }

    void WriteToDisk(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) throw std::runtime_error("No se pudo crear: " + filename);
        
        file << N << " " << maxEntries << " " << minEntries << " " << height << " " << numEntries << "\n";
        root->Write(file);
        file.close();
        std::cout << "R-Tree " << N << "D guardado (" << numEntries << " entradas)\n";
    }

    void Print(std::ostream& os = std::cout) const {
        os << "\n=== R-Tree " << N << "D [M=" << maxEntries << ", m=" << minEntries 
           << ", h=" << height << ", n=" << numEntries << "] ===\n";
        root->Print(os);
    }
};

#endif