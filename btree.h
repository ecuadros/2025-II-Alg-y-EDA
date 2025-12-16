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
#include <array>
#define DEFAULT_BTREE_ORDER 3

/**
 * @struct HyperRect
 * @brief Representa un hiper-rectángulo en N dimensiones.
 * @tparam Dim El número de dimensiones.
 */
template <size_t Dim>
struct HyperRect {
    std::array<int, Dim> min_coords;
    std::array<int, Dim> max_coords;

    // calculo el volumen
    int volume() const {
        int vol = 1;
        for (size_t i = 0; i < Dim; ++i) {
            vol *= (max_coords[i] - min_coords[i]);
        }
        return vol;
    }

    // calculo el volumen de expansión 
    static int expansionNecesaria(const HyperRect& contenedor, const HyperRect& nuevo) {
        HyperRect r = unir(contenedor, nuevo);
        return r.volume() - contenedor.volume();
    }

    // creo MBR de dos hiperrectangulos.
    static HyperRect unir(const HyperRect& a, const HyperRect& b) {
        HyperRect result;
        for (size_t i = 0; i < Dim; ++i) {
            result.min_coords[i] = std::min(a.min_coords[i], b.min_coords[i]);
            result.max_coords[i] = std::max(a.max_coords[i], b.max_coords[i]);
        }
        return result;
    }

    // comprueba si este hiper-rectángulo se solapa con otro
    bool intersecta(const HyperRect& otro) const {
        for (size_t i = 0; i < Dim; ++i) {
            if (max_coords[i] < otro.min_coords[i] || min_coords[i] > otro.max_coords[i]) {
                return false; // no hay solapamiento y no hay intersección
            }
        }
        return true;
         // hay solapamiento
    }

    // comprueba la igualdad exacta 
    bool operator==(const HyperRect& otro) const {
        return min_coords == otro.min_coords && max_coords == otro.max_coords;
    }
};

template <size_t Dim>
inline std::ostream& operator<<(std::ostream& os, const HyperRect<Dim>& r) {
    os << "{";
    for(size_t i = 0; i < Dim; ++i) os << r.min_coords[i] << (i == Dim - 1 ? "" : ",");
    os << "},{";
    for(size_t i = 0; i < Dim; ++i) os << r.max_coords[i] << (i == Dim - 1 ? "" : ",");
    os << "}";
    return os;
};

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
 * @tparam Dim El número de dimensiones.
 */
template <typename _ObjIDType, size_t Dim>
struct RTreeTrait {
    using keyType   = HyperRect<Dim>; // La clave es un HiperRectángulo
    using ObjIDType = _ObjIDType; // El ID del objeto
    struct NoCompare {}; using Compare = NoCompare;
};

/**
 * @class RTree
 * @brief Implementa una estructura de datos R-Tree, adaptada de una base de B-Tree.
 * @tparam Trait Un struct que define los tipos usados por el R-Tree.
 */

#include "btreepage.h"

template <typename Trait>
class RTree
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;
       
       typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
       typedef typename BTNode::ObjectInfo      ObjectInfo;

public:
       /**
        * @brief Constructor del R-Tree.
        * @param order El orden del árbol.
        * @param unique Verdadero si las claves deben ser únicas.
        */
public:
       RTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
              : m_Order(order),
                m_Root(2 * order  + 1, unique),
                m_NumKeys(0),
                m_Unique(unique)
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Height = 1;
       }
       /**
        * @brief Constructor por movimiento
        * @param other El R-Tree a mover.
        */
       RTree(RTree&& other) noexcept
       {
              std::scoped_lock lock(m_Mutex, other.m_Mutex);
 
              m_Root    = std::exchange(other.m_Root, BTNode(0, false));
              m_Height  = std::exchange(other.m_Height, 1);
              m_Order   = std::exchange(other.m_Order, 0);
              m_NumKeys = std::exchange(other.m_NumKeys, 0);
              m_Unique  = std::exchange(other.m_Unique, false);
       }
       /// @brief Destructor.
       ~RTree() {}
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
bool RTree<Trait>::Insert(const keyType key, const ObjIDType ObjID){
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
std::vector<typename Trait::ObjIDType> RTree<Trait>::Search(const keyType& areaBusqueda)
{
    std::shared_lock<std::shared_mutex> lock(m_Mutex);
    std::vector<ObjIDType> resultados;
    m_Root.Search(areaBusqueda, resultados);
    return resultados;
}

template <typename Trait>
void RTree<Trait>::Write(ostream& os) {
    std::lock_guard<std::shared_mutex> lock(m_Mutex);
    os << m_Order << "\n";
    os << m_Unique << "\n";
    os << m_NumKeys << "\n";
    os << m_Height << "\n";
    m_Root.Write(os);
}

template <typename Trait>
void RTree<Trait>::Read(istream& is) {
    std::lock_guard<std::shared_mutex> lock(m_Mutex);
    is >> m_Order >> m_Unique >> m_NumKeys >> m_Height;

    m_Root.m_MaxKeys = 2 * m_Order + 1;
    m_Root.m_Unique = m_Unique;
    m_Root.Create();

    m_Root.Read(is);
}

template <typename Trait>
bool RTree<Trait>::Remove (const keyType key, const ObjIDType ObjID)
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
std::ostream& operator<<(std::ostream& os, const RTree<Trait>& tree) {
    tree.Print(os);
    return os;
}

#endif