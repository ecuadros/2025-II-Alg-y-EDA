/**
 * @file btree.h
 * @brief Árbol B genérico con concurrencia (shared_mutex), guardado/carga y recorridos.
 * @details Expone BTreeTrait, BTree, iteradores forward/reverse, y utilidades como Save/Load.
 */
#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <fstream>
#include <shared_mutex> 
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 
/**
 * @tparam _keyType   Tipo de clave.
 * @tparam _ObjIDType Tipo del valor/identificador asociado.
 * @brief Rasgos para configurar el B-Tree (tipos y comparador).
 */
template <typename _keyType, typename _ObjIDType>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       // TODO DONE: agregar funcion de comparacion

       struct Compare { 
              bool operator() (const keyType &a, const keyType &b) const {
                     return a < b;
              }
       };
};

/**
 * @class BTree
 * @brief Implementación de un B-Tree con inserción, búsqueda, borrado y concurrencia.
 * @tparam Trait Debe definir `keyType`, `ObjIDType` y `struct Compare { bool operator()(const keyType&, const keyType&) const; }`.
 *
 * @par Características
 * - Concurrencia: protecciones con std::shared_mutex en operaciones públicas.
 * - Persistencia: @ref Save y @ref Load para escribir/leer el árbol.
 * - Recorridos: @ref ForEachT (genérico), @ref Print, e iteradores forward/reverse.
 */

template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;
       
       typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
       //typedef ObjectInfo iterator;
       typedef typename BTNode::lpfnForEach2    lpfnForEach2;
       typedef typename BTNode::lpfnForEach3    lpfnForEach3;
       typedef typename BTNode::lpfnFirstThat2  lpfnFirstThat2;
       typedef typename BTNode::lpfnFirstThat3  lpfnFirstThat3;
       typedef typename BTNode::ObjectInfo      ObjectInfo;

public:
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
              : m_Root(2 * order + 1, unique),
                m_Order(order),
                m_NumKeys(0),
                m_Unique(unique)
                
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Height = 1;
       }
       ~BTree() {}
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();
       bool            Insert (const keyType key, const long ObjID);
       bool            Remove (const keyType key, const long ObjID);
       ObjIDType       Search (const keyType key)
       {      ObjIDType ObjID = -1;
              m_Root.Search(key, ObjID);
              return ObjID;
       }
       size_t            size()  { return m_NumKeys; }
       size_t            height() { return m_Height;      }
       size_t            GetOrder() { return m_Order;     }

       void            Print (ostream &os)
       {               m_Root.Print(os);                              }
       template <class Fn, class... Args>
       void ForEachT(Fn&& fn, Args&&... args) {
              m_Root.ForEachT(std::forward<Fn>(fn), /*level=*/0,
                            std::forward<Args>(args)...);
       }
       void            ForEach( lpfnForEach2 lpfn, void *pExtra1 )
       {               m_Root.ForEach(lpfn, 0, pExtra1);              }
       void            ForEach( lpfnForEach3 lpfn, void *pExtra1, void *pExtra2)
       {               m_Root.ForEach(lpfn, 0, pExtra1, pExtra2);     }
       ObjectInfo*     FirstThat( lpfnFirstThat2 lpfn, void *pExtra1 )
       {               return m_Root.FirstThat(lpfn, 0, pExtra1);     }
       ObjectInfo*     FirstThat( lpfnFirstThat3 lpfn, void *pExtra1, void *pExtra2)
       {               return m_Root.FirstThat(lpfn, 0, pExtra1, pExtra2);   }
       //typedef               ObjectInfo iterator;
//move constructor 
public: 
       BTree(BTree&& other) noexcept;
       BTree& operator=(BTree&& other) noexcept;

       //deshabilitar copia
       BTree(const BTree&) = delete;
       BTree& operator=(const BTree&) = delete;

// write and read
       bool Save(const std::string& filename) const;
       bool Load(const std::string& filename);
       // getters const (nuevos o sobrecargas)
       size_t size()   const { return m_NumKeys; }
       size_t height() const { return m_Height;  }
       size_t GetOrder() const { return m_Order; }

       void Print(std::ostream& os) const {
       const_cast<BTNode&>(m_Root).Print(os);
       }

protected:
       BTNode          m_Root;
       size_t          m_Height;  // height of tree
       size_t          m_Order;   // order of tree
       size_t          m_NumKeys; // number of keys
       bool            m_Unique;  // Accept the elements only once ?
       size_t computeHeight(const BTNode& n) const;
       size_t computeSize  (const BTNode& n) const;
       mutable std::shared_mutex m_mtx;

}; 

// Iteradores in-order (forward) y reverse in-order (backward)
class iterator {
    using BTNode = typename BTree::BTNode;
    using ObjectInfo = typename BTree::ObjectInfo;
public:
    using value_type = ObjectInfo;
    using reference = ObjectInfo&;
    using pointer = ObjectInfo*;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    iterator() : m_tree(nullptr), m_curr(nullptr), m_idx(0) {}

    reference operator*()  const { return m_curr->m_Keys[m_idx]; }
    pointer   operator->() const { return &m_curr->m_Keys[m_idx]; }

    iterator& operator++() { next(); return *this; }
    iterator  operator++(int) { iterator tmp=*this; next(); return tmp; }

    bool operator==(const iterator& o) const {
        return m_curr == o.m_curr && m_idx == o.m_idx && m_tree == o.m_tree;
    }
    bool operator!=(const iterator& o) const { return !(*this == o); }

private:
    friend class BTree;
    struct Frame { BTNode* node; size_t idx; };

    iterator(BTree* t, bool to_begin) : m_tree(t), m_curr(nullptr), m_idx(0) {
        if (!t) return;
        if (to_begin) go_begin();
        else { // end()
            m_curr = nullptr; m_idx = 0; m_stack.clear();
        }
    }

    void go_begin() {
        m_stack.clear();
        BTNode* n = &m_tree->m_Root;
        // bajar hasta la hoja más a la izquierda
        while (n && n->m_SubPages[0]) {
            m_stack.push_back({n, 0});
            n = n->m_SubPages[0];
        }
        if (!n || n->m_KeyCount == 0) {
            // árbol vacío
            m_curr = nullptr; m_idx = 0; m_stack.clear();
            return;
        }
        m_curr = n; m_idx = 0;
    }

    void next() {
        if (!m_curr) return; // ya en end
        //si hay hijo derecho del elemento actual, bajamos a su mínimo
        if (m_curr->m_SubPages[m_idx + 1]) {
            BTNode* n = m_curr->m_SubPages[m_idx + 1];
            m_stack.push_back({m_curr, m_idx + 1}); // subiremos desde aquí
            while (n->m_SubPages[0]) {
                m_stack.push_back({n, 0});
                n = n->m_SubPages[0];
            }
            m_curr = n; m_idx = 0;
            return;
        }
        // subir hasta poder avanzar en el mismo nodo
        while (!m_stack.empty()) {
            auto fr = m_stack.back(); m_stack.pop_back();
            BTNode* parent = fr.node;
            size_t  pi     = fr.idx;
            // venimos de "antes" del key de índice pi; ese key es el siguiente
            if (pi < parent->m_KeyCount) {
                m_curr = parent; m_idx = pi;
                return;
            }
            // si pi == m_KeyCount, estábamos después del último key; seguir subiendo
        }
        //no hay más
        m_curr = nullptr; m_idx = 0; // end
    }

    BTree* m_tree;
    BTNode* m_curr;
    size_t m_idx;
    std::vector<Frame> m_stack;
};

class reverse_iterator {
    using BTNode = typename BTree::BTNode;
    using ObjectInfo = typename BTree::ObjectInfo;
public:
    using value_type = ObjectInfo;
    using reference = ObjectInfo&;
    using pointer = ObjectInfo*;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag; // ++ avanza hacia atrás lógico

    reverse_iterator() : m_tree(nullptr), m_curr(nullptr), m_idx(0) {}

    reference operator*()  const { return m_curr->m_Keys[m_idx]; }
    pointer   operator->() const { return &m_curr->m_Keys[m_idx]; }

    // ++ se mueve al "anterior" en orden
    reverse_iterator& operator++() { prev(); return *this; }
    reverse_iterator  operator++(int) { auto tmp=*this; prev(); return tmp; }

    bool operator==(const reverse_iterator& o) const {
        return m_curr == o.m_curr && m_idx == o.m_idx && m_tree == o.m_tree;
    }
    bool operator!=(const reverse_iterator& o) const { return !(*this == o); }

private:
    friend class BTree;
    struct Frame { BTNode* node; size_t idx; };

    reverse_iterator(BTree* t, bool to_rbegin) : m_tree(t), m_curr(nullptr), m_idx(0) {
        if (!t) return;
        if (to_rbegin) go_rbegin();
        else { // rend()
            m_curr = nullptr; m_idx = 0; m_stack.clear();
        }
    }

    void go_rbegin() {
        m_stack.clear();
        BTNode* n = &m_tree->m_Root;
        // bajar hasta la hoja más a la derecha
        while (n && n->m_SubPages[n->m_KeyCount]) {
            m_stack.push_back({n, n->m_KeyCount}); // entramos por "después" del último
            n = n->m_SubPages[n->m_KeyCount];
        }
        if (!n || n->m_KeyCount == 0) {
            m_curr = nullptr; m_idx = 0; m_stack.clear();
            return;
        }
        m_curr = n; m_idx = n->m_KeyCount - 1;
    }

    void prev() {
        if (!m_curr) return; // ya en rend
        // si hay hijo izquierdo del elemento actual, bajar a su máximo
        if (m_curr->m_SubPages[m_idx]) {
            BTNode* n = m_curr->m_SubPages[m_idx];
            m_stack.push_back({m_curr, m_idx}); // subiremos desde aquí
            while (n->m_SubPages[n->m_KeyCount]) {
                m_stack.push_back({n, n->m_KeyCount});
                n = n->m_SubPages[n->m_KeyCount];
            }
            m_curr = n; m_idx = n->m_KeyCount - 1;
            return;
        }
        // moverse al elemento anterior en el mismo nodo si existe
        if (m_idx > 0) { --m_idx; return; }
        // subir hasta poder tomar un "índice anterior" del padre
        while (!m_stack.empty()) {
            auto fr = m_stack.back(); m_stack.pop_back();
            BTNode* parent = fr.node;
            size_t  pi     = fr.idx;
            if (pi > 0) { // el anterior en el padre
                m_curr = parent; m_idx = pi - 1;
                return;
            }
            // si pi == 0, seguir subiendo
        }
        //no hay más
        m_curr = nullptr; m_idx = 0; // rend
    }

    BTree* m_tree;
    BTNode* m_curr;
    size_t m_idx;
    std::vector<Frame> m_stack;
};

// Factories
iterator begin()  { return iterator(this, /*to_begin=*/true); }
iterator end()    { return iterator(this, /*to_begin=*/false); }
reverse_iterator rbegin() { return reverse_iterator(this, /*to_rbegin=*/true); }
reverse_iterator rend()   { return reverse_iterator(this, /*to_rbegin=*/false); }


template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const long ObjID){
       std::unique_lock<std::shared_mutex> lk(m_mtx); 
       bt_ErrorCode error = m_Root.Insert(key, ObjID);
       if( error == bt_duplicate )
               return false;
       m_NumKeys++;
       if( error == bt_overflow ){
               m_Root.SplitRoot();
               m_Height++;
       }
       return true;
}

template <typename Trait>
bool BTree<Trait>::Remove (const keyType key, const long ObjID)
{
       std::unique_lock<std::shared_mutex> lk(m_mtx); 
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if( error == bt_duplicate || error == bt_nofound )
               return false;
       m_NumKeys--;

       if( error == bt_rootmerged )
               m_Height--;
       return true;
}
//insertar template move constructor y move assignment
template <typename Trait>
BTree<Trait>::BTree(BTree&& other) noexcept
    : m_Root(std::move(other.m_Root)),
      m_Height(other.m_Height),
      m_Order(other.m_Order),
      m_NumKeys(other.m_NumKeys),
      m_Unique(other.m_Unique)
{
    other.m_Height = 0;
    other.m_NumKeys = 0;
}

template <typename Trait>
BTree<Trait>& BTree<Trait>::operator=(BTree&& other) noexcept {
    if (this != &other) {
        m_Root   = std::move(other.m_Root);  // se usa el move de CBTreePage
        m_Height = other.m_Height;
        m_Order  = other.m_Order;
        m_NumKeys= other.m_NumKeys;
        m_Unique = other.m_Unique;

        other.m_Height = 0;
        other.m_NumKeys = 0;
    }
    return *this;
}
//write and read
template <typename Trait>
size_t BTree<Trait>::computeHeight(const BTNode& n) const {
    // hoja: altura 1 
    if (!n.m_SubPages[0]) return 1;
    size_t best = 0;
    for (size_t i = 0; i <= n.m_KeyCount; ++i) {
        if (n.m_SubPages[i]) {
            best = std::max(best, computeHeight(*n.m_SubPages[i]));
        }
    }
    return best + 1;
}
template <typename Trait>
size_t BTree<Trait>::computeSize(const BTNode& n) const {
    size_t sum = n.m_KeyCount;
    for (size_t i = 0; i <= n.m_KeyCount; ++i) {
        if (n.m_SubPages[i]) sum += computeSize(*n.m_SubPages[i]);
    }
    return sum;
}

template <typename Trait>
bool BTree<Trait>::Save(const std::string& filename) const {
    std::shared_lock<std::shared_mutex> lk(m_mtx);
    std::ofstream ofs(filename);
    if (!ofs) return false;

    // encabezado mínimo del árbol por si quieres validar formato
    ofs << m_Order << ' ' << (m_Unique ? 1 : 0) << '\n';

    // delega al nodo raíz
    m_Root.Write(ofs);

    return true;
}

template <typename Trait>
bool BTree<Trait>::Load(const std::string& filename) {
    std::unique_lock<std::shared_mutex> lk(m_mtx); 
    std::ifstream ifs(filename);
    if (!ifs) return false;

    size_t order = 0; int uniq = 1;
    ifs >> order >> uniq;

    // Si cambió el “order”, recrea root con la nueva capacidad de hijos
    m_Order  = order ? order : m_Order;
    m_Unique = (uniq != 0);

    // Reconfigura root para hijos 
    m_Root.SetMaxKeysForChilds(m_Order);

    // Limpia y carga
    m_Root.Reset();
    m_Root.Read(ifs);

    // Recalcula métricas
    m_Height = computeHeight(m_Root);
    m_NumKeys = computeSize(m_Root);

    return true;
}
template <typename Trait>
std::ostream& operator<<(std::ostream& os, const BTree<Trait>& t) {
    os << "size=" << t.size() << ", height=" << t.height() << "\n";
    t.Print(os);        
    return os;
}

#endif