#ifndef __BTREE_H__
#define __BTREE_H__

/**
 * @file btree.h
 * @brief Define la clase BTree, una estructura de datos de árbol balanceado.
 */

#include <shared_mutex>
#include <iostream>
#include <utility>
#include <mutex> 
#include <iterator>
#include <vector>
#include <fstream>
#define DEFAULT_BTREE_ORDER 3

/**
 * @struct Rect
 * @brief Representa un rectángulo en 2D, la clave para el R-Tree.
 */
struct Rect {
    int x1, y1, x2, y2;

    int area() const { return (x2 - x1) * (y2 - y1); }

    // para el area necesaria
    static int expansionNecesaria(const Rect& contenedor, const Rect& nuevo) {
        Rect r = unir(contenedor, nuevo);
        return r.area() - contenedor.area();
    }

    // MBR de dos rectángulos.
    static Rect unir(const Rect& a, const Rect& b) {
        return {std::min(a.x1, b.x1), std::min(a.y1, b.y1),
                std::max(a.x2, b.x2), std::max(a.y2, b.y2)};
    }

    // comprueba si hay solapamiento con otro.
    bool intersecta(const Rect& otro) const {
        return !(x2 < otro.x1 || x1 > otro.x2 || y2 < otro.y1 || y1 > otro.y2);
    }
};

inline std::ostream& operator<<(std::ostream& os, const Rect& r) {
    os << "{" << r.x1 << "," << r.y1 << "," << r.x2 << "," << r.y2 << "}";
    return os;
};

inline std::istream& operator>>(std::istream& is, Rect& r) {
    char c1, c2, c3, c4, c5;
    // formato {x1,y1,x2,y2}
    is >> c1 >> r.x1 >> c2 >> r.y1 >> c3 >> r.x2 >> c4 >> r.y2 >> c5;
    // por si hay un error en el formtao
    if (c1 != '{' || c2 != ',' || c3 != ',' || c4 != ',' || c5 != '}') {
        is.setstate(std::ios_base::failbit); 
    }
    return is;
}

const size_t MaxHeight = 5; 

/**
 * @struct BTreeTrait
 * @brief Define los tipos y la función de comparación para el B-Tree.
 * @tparam _keyType El tipo de dato para las claves.
 * @tparam _ObjIDType El tipo de dato para los IDs de objeto (valores).
 * @tparam _Compare La función de comparación (ej. std::less o std::greater).
 */
template <typename _keyType, typename _ObjIDType, typename _Compare = std::less<_keyType>>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       // TODO: agregar funcion de comparacion (DONE)
       using Compare = _Compare;
};

/**
 * @struct BTreeDescTrait
 * @brief Trait de ejemplo para un B-Tree descendente.
 * @tparam _keyType El tipo de dato para las claves.
 * @tparam _ObjIDType El tipo de dato para los IDs de objeto.
 */
template <typename _keyType, typename _ObjIDType>
struct BTreeDescTrait : public BTreeTrait<_keyType, _ObjIDType, std::greater<_keyType>> {};

/**
 * @struct RTreeTrait
 * @brief Define los tipos para un R-Tree. La clave es un Rect.
 * @tparam _ObjIDType El tipo de dato para los IDs de objeto.
 */
template <typename _ObjIDType>
struct RTreeTrait {
    using keyType   = Rect;       // La clave es un Rectángulo
    using ObjIDType = _ObjIDType; // El ID del objeto
    struct NoCompare {}; using Compare = NoCompare;
};

/**
 * @class BTree
 * @brief Implementa una estructura de datos B-Tree.
 * @tparam Trait Un struct que define los tipos usados por el B-Tree.
 */

#include "btreepage.h"

template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;
       
       typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
       /**
        * @class ForwardBTreeIterator
        * @brief Un iterador hacia adelante para el B-Tree.
        */
       class ForwardBTreeIterator {
       public:
              using iterator_category = std::forward_iterator_tag;
              using value_type = typename BTNode::ObjectInfo;
              using pointer = value_type*;
              using reference = value_type&;

              ForwardBTreeIterator(BTree* pTree, BTNode* pNode = nullptr, size_t keyIndex = 0)
                     : m_pTree(pTree), m_pNode(pNode), m_keyIndex(keyIndex) {}

              reference operator*() const { return m_pNode->m_Keys[m_keyIndex]; }
              pointer operator->() const { return &m_pNode->m_Keys[m_keyIndex]; }

              ForwardBTreeIterator& operator++() {
                     if (!m_pNode) {
                         return *this;
                     }
 
                     if (m_pNode->m_SubPages[0] != nullptr) {
                         BTNode* pCursor = m_pNode->m_SubPages[m_keyIndex + 1];
                         while (pCursor && pCursor->m_SubPages[0] != nullptr) {
                             pCursor = pCursor->m_SubPages[0];
                         }
                         m_pNode = pCursor;
                         m_keyIndex = 0;
                     } else { // Es un nodo hoja o el último hijo de un nodo interno
                         m_keyIndex++;
                         if (m_keyIndex < m_pNode->m_KeyCount) {
                             return *this;
                         }
                         BTNode* pCurrent = m_pNode;
                         BTNode* pParent = pCurrent->m_pParent;
                         size_t pos = 0;
                         if (pParent) {
                            while(pos <= pParent->m_KeyCount && pParent->m_SubPages[pos] != pCurrent) pos++;
                         }

                         while (pParent != nullptr && pos == pParent->m_KeyCount + 1) {
                             pCurrent = pParent;
                             pParent = pParent->m_pParent;
                             if (pParent) {
                                pos = 0;
                                while(pos <= pParent->m_KeyCount && pParent->m_SubPages[pos] != pCurrent) pos++;
                             }
                         }

                         if (pParent == nullptr) {
                             m_pNode = nullptr;
                         } else {
                             m_pNode = pParent;
                             m_keyIndex = pos;
                         }
                     }
                     return *this;
              }

              bool operator==(const ForwardBTreeIterator& other) const { return m_pNode == other.m_pNode && m_keyIndex == other.m_keyIndex; }
              bool operator!=(const ForwardBTreeIterator& other) const { return !(*this == other); }

       private:
              BTree*  m_pTree;
              BTNode* m_pNode;
              size_t m_keyIndex;
       };

       /**
        * @class BackwardBTreeIterator
        * @brief Un iterador hacia atrás para el B-Tree.
        */
       class BackwardBTreeIterator {
       public:
              using iterator_category = std::forward_iterator_tag;
              using value_type = typename BTNode::ObjectInfo;
              using pointer = value_type*;
              using reference = value_type&;

              BackwardBTreeIterator(BTree* pTree, BTNode* pNode = nullptr, size_t keyIndex = 0)
                     : m_pTree(pTree), m_pNode(pNode), m_keyIndex(keyIndex) {}

              reference operator*() const { return m_pNode->m_Keys[m_keyIndex]; }
              pointer operator->() const { return &m_pNode->m_Keys[m_keyIndex]; }

              BackwardBTreeIterator& operator++() {
                     if (!m_pNode) { // Si estamos en rend(), no hacemos nada.
                         return *this;
                     }

                     if (m_pNode->m_SubPages[0] != nullptr) {
                         BTNode* pCursor = m_pNode->m_SubPages[m_keyIndex];
                         while (pCursor && pCursor->m_SubPages[pCursor->m_KeyCount]) {
                             pCursor = pCursor->m_SubPages[pCursor->m_KeyCount];
                         }
                         m_pNode = pCursor;
                         m_keyIndex = pCursor ? pCursor->m_KeyCount - 1 : 0;
                     } else {
                         if (m_keyIndex > 0) {
                             m_keyIndex--;
                         } else {
                             BTNode* pCurrent = m_pNode;
                             BTNode* pParent = pCurrent->m_pParent;
                             size_t pos = 0;
                             if (pParent) {
                                while(pos <= pParent->m_KeyCount && pParent->m_SubPages[pos] != pCurrent) pos++;
                             }

                             while (pParent != nullptr && pos == 0) {
                                 pCurrent = pParent;
                                 pParent = pParent->m_pParent;
                                 if (pParent) {
                                    pos = 0;
                                    while(pos <= pParent->m_KeyCount && pParent->m_SubPages[pos] != pCurrent) pos++;
                                 }
                             }

                             if (pParent == nullptr) {
                                 m_pNode = nullptr;
                             } else {
                                 m_pNode = pParent;
                                 m_keyIndex = pos - 1;
                             }
                         }
                     }
                     return *this;
              }

              bool operator==(const BackwardBTreeIterator& other) const { return m_pNode == other.m_pNode && m_keyIndex == other.m_keyIndex; }
              bool operator!=(const BackwardBTreeIterator& other) const { return !(*this == other); }

       private:
              BTree*  m_pTree;
              BTNode* m_pNode;
              size_t m_keyIndex;
       };

       using iterator = ForwardBTreeIterator;
       using reverse_iterator = BackwardBTreeIterator;
       typedef typename BTNode::ObjectInfo      ObjectInfo;

public:
       /**
        * @brief Constructor del B-Tree.
        * @param order El orden del árbol.
        * @param unique Verdadero si las claves deben ser únicas.
        */
public:
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
              : m_Order(order),
                m_Root(2 * order  + 1, unique),
                m_NumKeys(0),
                m_Unique(unique)
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Height = 1;
       }
       /**
        * @brief Constructor por movimiento.
        * @param other El B-Tree a mover.
        */
       BTree(BTree&& other) noexcept
       {
              std::scoped_lock lock(m_Mutex, other.m_Mutex);
 
              m_Root    = std::exchange(other.m_Root, BTNode(0, false));
              m_Height  = std::exchange(other.m_Height, 1);
              m_Order   = std::exchange(other.m_Order, 0);
              m_NumKeys = std::exchange(other.m_NumKeys, 0);
              m_Unique  = std::exchange(other.m_Unique, false);
       }
       /// @brief Destructor.
       ~BTree() {}
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();
       /// @brief Escribe el árbol a un flujo de salida para serialización.
       /// @param os El flujo de salida.
       void            Write(ostream& os);
       /// @brief Lee el árbol desde un flujo de entrada para deserialización.
       /// @param is El flujo de entrada.
       void            Read(istream& is);

       /**
        * @brief Inserta un par clave-valor en el árbol.
        * @param key La clave a insertar.
        * @param ObjID El valor (ID de objeto) a asociar con la clave.
        * @return Verdadero si la inserción fue exitosa, falso si la clave era un duplicado.
        */
       bool            Insert (const keyType key, const ObjIDType ObjID);
       /**
        * @brief Elimina una clave del árbol.
        * @param key La clave a eliminar.
        * @param ObjID El ID de objeto asociado (actualmente no se usa en la lógica de eliminación).
        * @return Verdadero si la eliminación fue exitosa.
        */
       bool            Remove (const keyType key, const ObjIDType ObjID);
       /**
        * @brief Busca una clave en el árbol.
        * @param key La clave a buscar.
        * @return Devolver una lista de resultados.
        */
       std::vector<ObjIDType> Search(const keyType& areaBusqueda);

       /// Devuelve el número total de claves en el árbol.
       size_t            size()  const { std::shared_lock<std::shared_mutex> lock(m_Mutex); return m_NumKeys; }
       /// Devuelve la altura del árbol.
       size_t            height() const { std::shared_lock<std::shared_mutex> lock(m_Mutex); return m_Height;      }
       /// Devuelve el orden del árbol.
       size_t            GetOrder() const { std::shared_lock<std::shared_mutex> lock(m_Mutex); return m_Order;     }

       /// Imprime la estructura del árbol en un flujo de salida.
       void            Print (ostream &os) const
       {               
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              m_Root.Print(os);
       }

      iterator begin() {
              std::lock_guard<std::shared_mutex> lock(m_Mutex);
              BTNode* pNode = &m_Root;
              if (!pNode || pNode->m_KeyCount == 0) {
                  return end();
              }
              while (pNode && pNode->m_SubPages[0] != nullptr) {
                     pNode = pNode->m_SubPages[0];
              }
              return iterator(this, pNode, 0);
       }
       /// Devuelve un iterador al elemento siguiente al último.
       iterator end() { return iterator(this, nullptr, 0); }

       /// Devuelve un iterador inverso al último elemento.
       reverse_iterator rbegin() {
              std::lock_guard<std::shared_mutex> lock(m_Mutex);
              BTNode* pNode = &m_Root;
              if (!pNode || pNode->m_KeyCount == 0) {
                  return rend();
              }
              while (pNode && pNode->m_SubPages[pNode->m_KeyCount]) {
                  pNode = pNode->m_SubPages[pNode->m_KeyCount];
              }
              if (pNode && pNode->m_KeyCount > 0) {
                  return reverse_iterator(this, pNode, pNode->m_KeyCount - 1);
              }
              return rend();
       }
       /// Devuelve un iterador inverso al elemento anterior al primero.
       reverse_iterator rend() { return reverse_iterator(this, nullptr, 0); }


       /**
        * @brief Aplica una función a cada elemento del árbol en orden.
        * @tparam Func El tipo de la función.
        * @tparam Args Los tipos de los argumentos adicionales para la función.
        * @param level Nivel inicial para el recorrido (usualmente 0).
        * @param func La función a aplicar.
        * @param args Argumentos adicionales para la función.
        */
       template<typename Func, typename... Args>
       void ForEach(size_t level, Func&& func, Args&&... args) {
              m_Root.ForEach(level, std::forward<Func>(func), std::forward<Args>(args)...);
       }

       /**
        * @brief Busca el primer elemento que satisface una condición.
        * @return Un puntero al ObjectInfo si se encuentra, de lo contrario nullptr.
        */
       template<typename Func, typename... Args>
       ObjectInfo* FirstThat(size_t level, Func&& func, Args&&... args) {
              return m_Root.FirstThat(level, std::forward<Func>(func), std::forward<Args>(args)...);
       }
protected:
       BTNode          m_Root;    ///< El nodo raíz del B-Tree.
       size_t          m_Height;  ///< Altura del árbol.
       size_t          m_Order;   ///< Orden del árbol.
       size_t          m_NumKeys; ///< Número total de claves en el árbol.
       bool            m_Unique;  ///< Verdadero si las claves deben ser únicas.
       mutable std::shared_mutex m_Mutex; ///< Mutex de lectura-escritura para seguridad en hilos.
};     

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const ObjIDType ObjID){
       std::lock_guard<std::shared_mutex> lock(m_Mutex);
       
       BTNode* pNewNode = nullptr;
       bt_ErrorCode error = m_Root.Insert(key, ObjID, &pNewNode);

       m_NumKeys++;

       if( error == bt_overflow ){
               BTNode* pLeftChild = new BTNode(std::move(m_Root));
               
               // reinicializar m_Root
               m_Root.m_MaxKeys = 2 * m_Order + 1;
               m_Root.m_Unique = m_Unique;
               m_Root.Create();
               m_Root.SetMaxKeysForChilds(m_Order);

               // el nodo es el hijo derecho.
               BTNode* pRightChild = pNewNode;

               // establecer la nueva raíz como padre de los hijos
               pLeftChild->SetParent(&m_Root);
               pRightChild->SetParent(&m_Root);
               m_Root.AddChild(pLeftChild);
               m_Root.AddChild(pRightChild);
               m_Height++;
       }
       return true;
}

template <typename Trait>
std::vector<typename Trait::ObjIDType> BTree<Trait>::Search(const keyType& areaBusqueda)
{
    std::shared_lock<std::shared_mutex> lock(m_Mutex);
    std::vector<ObjIDType> resultados;
    m_Root.Search(areaBusqueda, resultados);
    return resultados;
}

template <typename Trait>
void BTree<Trait>::Write(ostream& os) {
    std::lock_guard<std::shared_mutex> lock(m_Mutex);
    os << m_Order << "\n";
    os << m_Unique << "\n";
    os << m_NumKeys << "\n";
    os << m_Height << "\n";
    m_Root.Write(os);
}

template <typename Trait>
void BTree<Trait>::Read(istream& is) {
    std::lock_guard<std::shared_mutex> lock(m_Mutex);
    is >> m_Order >> m_Unique >> m_NumKeys >> m_Height;

    m_Root.m_MaxKeys = 2 * m_Order + 1;
    m_Root.m_Unique = m_Unique;
    m_Root.Create();

    m_Root.Read(is);
}

template <typename Trait>
bool BTree<Trait>::Remove (const keyType key, const ObjIDType ObjID)
{
    std::lock_guard<std::shared_mutex> lock(m_Mutex);
    
    std::vector<ObjectInfo> reinsert_list;
    bool found = m_Root.Remove(key, ObjID, reinsert_list);

    if (!found) return false;

    m_NumKeys--;

    // reinsertar los nodos que tuvieron underflow
    for (const auto& entry : reinsert_list) {
        Insert(entry.key, entry.ObjID);
    }

    // si hay solo 1 hijo, se convierte en la nueva raíz
    if (m_Root.m_KeyCount == 1 && m_Height > 1 && m_Root.m_SubPages[0] != nullptr) {
        m_Root = std::move(*m_Root.m_SubPages[0]);
        m_Height--;
    }

    return true;
}

/// Sobrecarga del operador << para imprimir el R-Tree.
template <typename Trait>
std::ostream& operator<<(std::ostream& os, const BTree<Trait>& tree) {
    tree.Print(os);
    return os;
}

#endif