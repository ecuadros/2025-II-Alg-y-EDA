#ifndef __RTREE_H__
#define __RTREE_H__

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <utility>
#include <cmath>
#include <algorithm>
#include <stdexcept>

// Trait para configurar el RTree N-dimensional
template <typename _CoordType, typename _DataType, int _Dim>
struct RTreeTrait
{
       using CoordType = _CoordType;
       using DataType = _DataType;
       static constexpr int Dim = _Dim;
};

template <typename Trait, int MaxNodes, int MinNodes> class RTree;
template <typename Trait> class RTreeNode;

// Hiper-rectangulo MBR (Minimum Bounding Rectangle) N-dimensional
template <typename Trait>
struct Rect
{
       typedef typename Trait::CoordType CoordType;
       static constexpr int Dim = Trait::Dim;
       
       std::array<CoordType, Dim> minCoord;
       std::array<CoordType, Dim> maxCoord;
       
       // Verifica si dos hiper-rectangulos se solapan
       bool Overlaps(const Rect& other) const
       {
              for (int i = 0; i < Dim; ++i)
                     if (minCoord[i] > other.maxCoord[i] || maxCoord[i] < other.minCoord[i])
                            return false;
              return true;
       }
       
       bool operator==(const Rect& other) const
       {
              return minCoord == other.minCoord && maxCoord == other.maxCoord;
       }
       
       // Calcula el volumen N-dimensional del hiper-rectangulo
       CoordType Area() const
       {
              CoordType vol = 1;
              for (int i = 0; i < Dim; ++i)
                     vol *= (maxCoord[i] - minCoord[i]);
              return vol;
       }
       
       // Combina dos hiper-rectangulos en uno que los contenga a ambos
       Rect Combine(const Rect& other) const
       {
              Rect result;
              for (int i = 0; i < Dim; ++i)
              {
                     result.minCoord[i] = std::min(minCoord[i], other.minCoord[i]);
                     result.maxCoord[i] = std::max(maxCoord[i], other.maxCoord[i]);
              }
              return result;
       }
       
       // Calcula cuanto creceria el volumen al agregar otro hiper-rectangulo
       CoordType Enlargement(const Rect& other) const
       {
              return Combine(other).Area() - Area();
       }
};

// Entrada en un nodo del RTree
template <typename Trait>
struct RTreeEntry
{
       typedef typename Trait::DataType DataType;
       typedef Rect<Trait> RectType;
       typedef RTreeNode<Trait>* NodePtr;
       
       RectType rect;           // MBR de la entrada
       DataType data;           // Dato (solo en hojas)
       NodePtr child = nullptr; // Hijo (solo en nodos internos)
};

// Nodo del RTree
template <typename Trait>
class RTreeNode
{
public:
       typedef typename Trait::CoordType CoordType;
       typedef Rect<Trait> RectType;
       typedef RTreeEntry<Trait> Entry;
       
       int level = 0;                 // Nivel del nodo (0 = hoja)
       std::vector<Entry> entries;   // Entradas del nodo
       
       bool IsLeaf() const { return level == 0; }
       
       // Calcula el MBR que contiene todas las entradas
       RectType ComputeMBR() const
       {
              RectType mbr = entries[0].rect;
              for (size_t i = 1; i < entries.size(); ++i)
                     mbr = mbr.Combine(entries[i].rect);
              return mbr;
       }
       
       // Encuentra el hijo cuyo MBR requiere menor expansion
       size_t ChooseBestChild(const RectType& rect) const
       {
              size_t best = 0;
              CoordType minEnl = entries[0].rect.Enlargement(rect);
              CoordType minArea = entries[0].rect.Area();
              
              for (size_t i = 1; i < entries.size(); ++i)
              {
                     CoordType enl = entries[i].rect.Enlargement(rect);
                     CoordType area = entries[i].rect.Area();
                     
                     // Preferir menor enlargement, desempatar por menor area
                     if (enl < minEnl || (enl == minEnl && area < minArea))
                     {
                            minEnl = enl;
                            minArea = area;
                            best = i;
                     }
              }
              return best;
       }
};

// Arbol R (R-Tree) N-dimensional
template <typename Trait, int MaxNodes = 4, int MinNodes = 2>
class RTree
{
       static_assert(MinNodes >= 1, "MinNodes debe ser >= 1");
       static_assert(MaxNodes >= 2 * MinNodes, "MaxNodes debe ser >= 2*MinNodes");
       static_assert(Trait::Dim >= 1, "Dimension debe ser >= 1");
       
public:
       typedef typename Trait::CoordType CoordType;
       typedef typename Trait::DataType DataType;
       typedef Rect<Trait> RectType;
       typedef RTreeEntry<Trait> Entry;
       typedef RTreeNode<Trait> Node;
       static constexpr int Dim = Trait::Dim;
       
       typedef std::pair<RectType, DataType> QueryResult;
       typedef std::array<CoordType, Dim> Point;

private:
       Node* m_Root;
       size_t m_Count;

public:
       RTree() : m_Root(new Node()), m_Count(0) {}
       ~RTree() { Destroy(m_Root); }
       
       size_t size() const { return m_Count; }
       bool empty() const { return m_Count == 0; }
       
       // Inserta un elemento con su MBR y dato asociado
       void Insert(const Point& minCoord, const Point& maxCoord, const DataType& data)
       {
              Entry entry;
              entry.rect.minCoord = minCoord;
              entry.rect.maxCoord = maxCoord;
              entry.data = data;
              entry.child = nullptr;
              
              Node* newNode = nullptr;
              if (InsertRec(entry, m_Root, newNode, 0))
              {
                     // La raiz se dividio, crear nueva raiz
                     Node* newRoot = new Node();
                     newRoot->level = m_Root->level + 1;
                     
                     Entry e1, e2;
                     e1.rect = m_Root->ComputeMBR();
                     e1.child = m_Root;
                     e2.rect = newNode->ComputeMBR();
                     e2.child = newNode;
                     
                     newRoot->entries.push_back(e1);
                     newRoot->entries.push_back(e2);
                     m_Root = newRoot;
              }
              ++m_Count;
       }
       
       // Elimina un elemento buscando por MBR y dato
       bool Remove(const Point& minCoord, const Point& maxCoord, const DataType& data)
       {
              RectType targetRect;
              targetRect.minCoord = minCoord;
              targetRect.maxCoord = maxCoord;
              std::vector<Entry> reinsertList;
              
              if (RemoveRec(targetRect, data, m_Root, reinsertList))
                     return false;
              
              // Reinsertar entradas huerfanas
              for (auto& entry : reinsertList)
                     ReinsertEntry(entry);
              
              // Reducir altura si la raiz quedo con un solo hijo
              while (!m_Root->IsLeaf() && m_Root->entries.size() == 1)
              {
                     Node* oldRoot = m_Root;
                     m_Root = m_Root->entries[0].child;
                     delete oldRoot;
              }
              --m_Count;
              return true;
       }
       
       // Retorna todos los elementos cuyo MBR intersecta con el rango dado
       std::vector<QueryResult> RangeQuery(const Point& minCoord, const Point& maxCoord) const
       {
              std::vector<QueryResult> results;
              RectType queryRect;
              queryRect.minCoord = minCoord;
              queryRect.maxCoord = maxCoord;
              SearchRec(m_Root, queryRect, results);
              return results;
       }
       
       // Escribe el arbol a disco
       bool Write(const std::string& filename) const
       {
              std::ofstream file(filename);
              if (!file) return false;
              file << m_Count << "\n";
              WriteNode(file, m_Root);
              return true;
       }
       
       // Lee el arbol desde disco
       bool Read(const std::string& filename)
       {
              std::ifstream file(filename);
              if (!file) return false;
              file >> m_Count;
              Destroy(m_Root);
              m_Root = ReadNode(file);
              return true;
       }
       
       void Print() const { Print(std::cout); }
       
       void Print(std::ostream& os) const
       {
              os << "RTree [" << m_Count << " elementos]\n";
              PrintNode(os, m_Root, 0);
       }

private:
       // Destruye un nodo y sus hijos recursivamente
       void Destroy(Node* node)
       {
              if (!node) return;
              for (auto& entry : node->entries)
                     if (entry.child) Destroy(entry.child);
              delete node;
       }
       
       // Insercion recursiva. Retorna true si hubo split
       bool InsertRec(const Entry& entry, Node* node, Node*& newNode, int targetLevel)
       {
              if (node->level > targetLevel)
              {
                     // Nodo interno: bajar al mejor hijo
                     size_t best = node->ChooseBestChild(entry.rect);
                     Node* childNew = nullptr;
                     bool split = InsertRec(entry, node->entries[best].child, childNew, targetLevel);
                     
                     // Actualizar MBR del hijo
                     node->entries[best].rect = node->entries[best].child->ComputeMBR();
                     
                     if (!split) return false;
                     
                     // El hijo se dividio, agregar el nuevo nodo
                     Entry newEntry;
                     newEntry.rect = childNew->ComputeMBR();
                     newEntry.child = childNew;
                     
                     if (node->entries.size() < MaxNodes)
                     {
                            node->entries.push_back(newEntry);
                            return false;
                     }
                     Split(node, newEntry, newNode);
                     return true;
              }
              else
              {
                     // Nodo hoja o nivel objetivo: insertar aqui
                     if (node->entries.size() < MaxNodes)
                     {
                            node->entries.push_back(entry);
                            return false;
                     }
                     Split(node, entry, newNode);
                     return true;
              }
       }
       
       // Quadratic Split
       void Split(Node* node, const Entry& entry, Node*& newNode)
       {
              std::vector<Entry> all = node->entries;
              all.push_back(entry);
              
              // PickSeeds: elegir las 2 entradas que desperdiciarian mas area juntas
              size_t seed1 = 0, seed2 = 1;
              CoordType maxWaste = all[0].rect.Combine(all[1].rect).Area()
                                 - all[0].rect.Area() - all[1].rect.Area();
              
              for (size_t i = 0; i < all.size() - 1; ++i)
              {
                     for (size_t j = i + 1; j < all.size(); ++j)
                     {
                            CoordType waste = all[i].rect.Combine(all[j].rect).Area()
                                            - all[i].rect.Area() - all[j].rect.Area();
                            if (waste > maxWaste)
                            {
                                   maxWaste = waste;
                                   seed1 = i;
                                   seed2 = j;
                            }
                     }
              }
              
              // Asignar cada semilla a un grupo
              newNode = new Node();
              newNode->level = node->level;
              node->entries.clear();
              node->entries.push_back(all[seed1]);
              newNode->entries.push_back(all[seed2]);
              
              std::vector<bool> used(all.size(), false);
              used[seed1] = used[seed2] = true;
              
              // PickNext: distribuir el resto de entradas
              for (size_t remaining = all.size() - 2; remaining > 0; --remaining)
              {
                     // Si un grupo necesita todas las restantes para tener m entradas
                     if (node->entries.size() + remaining <= MinNodes)
                     {
                            for (size_t i = 0; i < all.size(); ++i)
                                   if (!used[i]) { node->entries.push_back(all[i]); used[i] = true; }
                            break;
                     }
                     if (newNode->entries.size() + remaining <= MinNodes)
                     {
                            for (size_t i = 0; i < all.size(); ++i)
                                   if (!used[i]) { newNode->entries.push_back(all[i]); used[i] = true; }
                            break;
                     }
                     
                     // Calcular costo de poner cada entrada en cada grupo
                     RectType mbr1 = node->ComputeMBR();
                     RectType mbr2 = newNode->ComputeMBR();
                     size_t bestIdx = 0;
                     CoordType bestDiff = -1;
                     CoordType bestEnl1 = 0, bestEnl2 = 0;
                     
                     for (size_t i = 0; i < all.size(); ++i)
                     {
                            if (used[i]) continue;
                            CoordType enl1 = mbr1.Enlargement(all[i].rect);
                            CoordType enl2 = mbr2.Enlargement(all[i].rect);
                            CoordType diff = std::abs(enl1 - enl2);
                            
                            // Elegir entrada con mayor diferencia
                            if (bestDiff < 0 || diff > bestDiff)
                            {
                                   bestDiff = diff;
                                   bestIdx = i;
                                   bestEnl1 = enl1;
                                   bestEnl2 = enl2;
                            }
                     }
                     
                     // Asignar al grupo con menor enlargement
                     used[bestIdx] = true;
                     int group = ChooseGroup(bestEnl1, bestEnl2, mbr1, mbr2, node, newNode);
                     if (group == 0)
                            node->entries.push_back(all[bestIdx]);
                     else
                            newNode->entries.push_back(all[bestIdx]);
              }
       }
       
       // Desempate: menor enlargement, luego menor area, luego menos entradas
       int ChooseGroup(CoordType enl1, CoordType enl2, const RectType& mbr1, const RectType& mbr2,
                       const Node* g1, const Node* g2) const
       {
              if (enl1 < enl2) return 0;
              if (enl2 < enl1) return 1;
              
              CoordType a1 = mbr1.Area(), a2 = mbr2.Area();
              if (a1 < a2) return 0;
              if (a2 < a1) return 1;
              
              if (g1->entries.size() <= g2->entries.size()) return 0;
              return 1;
       }
       
       // Eliminacion recursiva. Retorna true si no encontro el elemento
       bool RemoveRec(const RectType& targetRect, const DataType& data, 
                      Node* node, std::vector<Entry>& reinsertList)
       {
              if (!node->IsLeaf())
              {
                     // Nodo interno: buscar en hijos que se solapen con el objetivo
                     for (size_t i = 0; i < node->entries.size(); ++i)
                     {
                            if (!node->entries[i].rect.Overlaps(targetRect))
                                   continue;
                            
                            if (!RemoveRec(targetRect, data, node->entries[i].child, reinsertList))
                            {
                                   if (node->entries[i].child->entries.empty())
                                   {
                                          // Nodo hijo quedo vacio
                                          delete node->entries[i].child;
                                          node->entries.erase(node->entries.begin() + i);
                                   }
                                   else
                                   {
                                          // Actualizar MBR del hijo
                                          node->entries[i].rect = node->entries[i].child->ComputeMBR();
                                          
                                          // Verificar underflow
                                          if (node->entries[i].child->entries.size() < MinNodes)
                                          {
                                                 for (auto& e : node->entries[i].child->entries)
                                                        reinsertList.push_back(e);
                                                 delete node->entries[i].child;
                                                 node->entries.erase(node->entries.begin() + i);
                                          }
                                   }
                                   return false;
                            }
                     }
                     return true;
              }
              else
              {
                     // Nodo hoja: buscar por MBR y dato
                     for (size_t i = 0; i < node->entries.size(); ++i)
                     {
                            if (node->entries[i].rect == targetRect && node->entries[i].data == data)
                            {
                                   node->entries.erase(node->entries.begin() + i);
                                   return false;
                            }
                     }
                     return true;
              }
       }
       
       // Reinserta una entrada al nivel correcto del arbol
       void ReinsertEntry(Entry& entry)
       {
              if (entry.child == nullptr)
              {
                     // Entrada de hoja: reinsertar normalmente
                     Insert(entry.rect.minCoord, entry.rect.maxCoord, entry.data);
                     --m_Count;
              }
              else
              {
                     // Entrada de nodo interno: insertar al nivel correcto
                     int targetLevel = entry.child->level + 1;
                     Node* newNode = nullptr;
                     
                     if (InsertRec(entry, m_Root, newNode, targetLevel))
                     {
                            Node* newRoot = new Node();
                            newRoot->level = m_Root->level + 1;
                            
                            Entry e1, e2;
                            e1.rect = m_Root->ComputeMBR();
                            e1.child = m_Root;
                            e2.rect = newNode->ComputeMBR();
                            e2.child = newNode;
                            
                            newRoot->entries.push_back(e1);
                            newRoot->entries.push_back(e2);
                            m_Root = newRoot;
                     }
              }
       }
       
       // Busqueda recursiva
       void SearchRec(Node* node, const RectType& query, std::vector<QueryResult>& results) const
       {
              for (auto& entry : node->entries)
              {
                     if (query.Overlaps(entry.rect))
                     {
                            if (node->IsLeaf())
                                   results.push_back({entry.rect, entry.data});
                            else
                                   SearchRec(entry.child, query, results);
                     }
              }
       }
       
       // Escribe un nodo y sus hijos recursivamente
       void WriteNode(std::ostream& os, Node* node) const
       {
              os << node->level << " " << node->entries.size() << "\n";
              for (auto& entry : node->entries)
              {
                     for (int i = 0; i < Dim; ++i)
                            os << entry.rect.minCoord[i] << " ";
                     for (int i = 0; i < Dim; ++i)
                            os << entry.rect.maxCoord[i] << (i < Dim - 1 ? " " : "\n");
                     
                     if (node->IsLeaf())
                            os << entry.data << "\n";
                     else
                            WriteNode(os, entry.child);
              }
       }
       
       // Lee un nodo y sus hijos recursivamente
       Node* ReadNode(std::istream& is)
       {
              Node* node = new Node();
              size_t n;
              is >> node->level >> n;
              
              for (size_t i = 0; i < n; ++i)
              {
                     Entry entry;
                     for (int d = 0; d < Dim; ++d)
                            is >> entry.rect.minCoord[d];
                     for (int d = 0; d < Dim; ++d)
                            is >> entry.rect.maxCoord[d];
                     
                     if (node->IsLeaf())
                            is >> entry.data;
                     else
                            entry.child = ReadNode(is);
                     
                     node->entries.push_back(entry);
              }
              return node;
       }
       
       // Imprime un nodo y sus hijos recursivamente
       void PrintNode(std::ostream& os, Node* node, int depth) const
       {
              std::string indent(depth * 2, ' ');
              os << indent << (node->IsLeaf() ? "LEAF" : "INTERNAL")
                 << " [level=" << node->level << ", n=" << node->entries.size() << "]\n";
              
              for (size_t i = 0; i < node->entries.size(); ++i)
              {
                     auto& entry = node->entries[i];
                     os << indent << "  (";
                     for (int d = 0; d < Dim; ++d)
                            os << entry.rect.minCoord[d] << (d < Dim - 1 ? "," : "");
                     os << ")-(";
                     for (int d = 0; d < Dim; ++d)
                            os << entry.rect.maxCoord[d] << (d < Dim - 1 ? "," : "");
                     os << ")";
                     if (node->IsLeaf())
                            os << " data=" << entry.data;
                     os << "\n";
                     if (!node->IsLeaf())
                            PrintNode(os, entry.child, depth + 1);
              }
       }
};

#endif