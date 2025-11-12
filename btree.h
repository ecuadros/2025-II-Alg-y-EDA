#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <functional>
#include <utility>
#include <fstream>
#include <string>
#include <iterator>
#include <mutex>
#include <shared_mutex>

template <typename Trait> class BTree;
template <typename Trait> class BTreeIterator;
template <typename Trait> class BTreeReverseIterator;

#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

/**
 * @brief Trait que define los tipos para un BTree
 * @tparam _keyType Tipo de dato para las claves
 * @tparam _ObjIDType Tipo de dato para los identificadores de objetos
 * @tparam _Compare Función de comparación (std::less o std::greater)
 */
template <typename _keyType, typename _ObjIDType, typename _Compare = std::less<_keyType>>
struct BTreeTrait
{
       using keyType = _keyType;      ///< Tipo de las claves
       using ObjIDType = _ObjIDType;  ///< Tipo de los identificadores
       using Compare = _Compare;      ///< Función de comparación
};

/**
 * @brief Alias para BTree con orden ascendente
 * @tparam Key Tipo de las claves
 * @tparam Value Tipo de los valores
 */
template <typename Key, typename Value>
using BTreeAscTrait = BTreeTrait<Key, Value, std::less<Key>>;

/**
 * @brief Alias para BTree con orden descendente
 * @tparam Key Tipo de las claves
 * @tparam Value Tipo de los valores
 */
template <typename Key, typename Value>
using BTreeDescTrait = BTreeTrait<Key, Value, std::greater<Key>>;

/**
 * @brief Iterador forward para recorrer un BTree de menor a mayor
 * 
 * Este iterador navega el árbol B en orden ascendente (in-order traversal).
 * El usuario controla la dirección eligiendo begin() o rbegin().
 * 
 * @tparam Trait Trait que define los tipos del BTree
 */
template <typename Trait>
class BTreeIterator
{
       friend class BTree<Trait>;
       typedef CBTreePage<Trait> BTPage;
       typedef typename BTPage::ObjectInfo ObjectInfo;
       
private:
       BTPage* m_CurrentPage;    ///< Página actual en el árbol
       size_t m_CurrentIndex;    ///< Índice de la clave actual en la página
       
       /**
        * @brief Constructor privado usado por BTree
        * @param page Puntero a la página actual
        * @param index Índice de la clave en la página
        */
       BTreeIterator(BTPage* page, size_t index) 
              : m_CurrentPage(page), m_CurrentIndex(index) {}
       
public:
       // Tipos requeridos por STL
       using iterator_category = std::forward_iterator_tag;
       using value_type = ObjectInfo;
       using difference_type = std::ptrdiff_t;
       using pointer = ObjectInfo*;
       using reference = ObjectInfo&;
       
       /** @brief Constructor por defecto, crea un iterador nulo (end) */
       BTreeIterator() : m_CurrentPage(nullptr), m_CurrentIndex(0) {}
       
       /**
        * @brief Operador de desreferencia
        * @return Referencia al ObjectInfo actual
        */
       ObjectInfo& operator*() { return m_CurrentPage->m_Keys[m_CurrentIndex]; }
       
       /**
        * @brief Operador de acceso a miembros
        * @return Puntero al ObjectInfo actual
        */
       ObjectInfo* operator->() { return &(m_CurrentPage->m_Keys[m_CurrentIndex]); }
       
       /**
        * @brief Pre-incremento: avanza al siguiente elemento (menor a mayor)
        * @return Referencia a este iterador
        */
       BTreeIterator& operator++();
       
       /**
        * @brief Post-incremento: avanza y retorna copia del estado anterior
        * @return Copia del iterador antes de avanzar
        */
       BTreeIterator operator++(int)
       {
               BTreeIterator temp = *this;
               ++(*this);
               return temp;
       }
       
       /**
        * @brief Compara dos iteradores por igualdad
        * @param other Otro iterador a comparar
        * @return true si apuntan a la misma posición
        */
       bool operator==(const BTreeIterator& other) const
       {
               if (m_CurrentPage == nullptr && other.m_CurrentPage == nullptr)
                       return true;
               return m_CurrentPage == other.m_CurrentPage && m_CurrentIndex == other.m_CurrentIndex;
       }
       
       /**
        * @brief Compara dos iteradores por desigualdad
        * @param other Otro iterador a comparar
        * @return true si apuntan a posiciones diferentes
        */
       bool operator!=(const BTreeIterator& other) const
       {
               return !(*this == other);
       }
};

/**
 * @brief Iterador reverso para recorrer un BTree de mayor a menor
 * 
 * Este iterador navega el árbol B en orden descendente (reverse in-order).
 * Se usa con rbegin() para iniciar en el elemento más grande.
 * 
 * @tparam Trait Trait que define los tipos del BTree
 */
template <typename Trait>
class BTreeReverseIterator
{
       friend class BTree<Trait>;
       typedef CBTreePage<Trait> BTPage;
       typedef typename BTPage::ObjectInfo ObjectInfo;
       
private:
       BTPage* m_CurrentPage;    ///< Página actual en el árbol
       size_t m_CurrentIndex;    ///< Índice de la clave actual en la página
       
       /**
        * @brief Constructor privado usado por BTree
        * @param page Puntero a la página actual
        * @param index Índice de la clave en la página
        */
       BTreeReverseIterator(BTPage* page, size_t index) 
              : m_CurrentPage(page), m_CurrentIndex(index) {}
       
public:
       // Tipos requeridos por STL
       using iterator_category = std::forward_iterator_tag;
       using value_type = ObjectInfo;
       using difference_type = std::ptrdiff_t;
       using pointer = ObjectInfo*;
       using reference = ObjectInfo&;
       
       /** @brief Constructor por defecto, crea un iterador nulo (rend) */
       BTreeReverseIterator() : m_CurrentPage(nullptr), m_CurrentIndex(0) {}
       
       /**
        * @brief Operador de desreferencia
        * @return Referencia al ObjectInfo actual
        */
       ObjectInfo& operator*() { return m_CurrentPage->m_Keys[m_CurrentIndex]; }
       
       /**
        * @brief Operador de acceso a miembros
        * @return Puntero al ObjectInfo actual
        */
       ObjectInfo* operator->() { return &(m_CurrentPage->m_Keys[m_CurrentIndex]); }
       
       /**
        * @brief Pre-incremento: avanza al elemento anterior (mayor a menor)
        * @return Referencia a este iterador
        */
       BTreeReverseIterator& operator++();
       
       /**
        * @brief Post-incremento: avanza y retorna copia del estado anterior
        * @return Copia del iterador antes de avanzar
        */
       BTreeReverseIterator operator++(int)
       {
               BTreeReverseIterator temp = *this;
               ++(*this);
               return temp;
       }
       
       /**
        * @brief Compara dos iteradores por igualdad
        * @param other Otro iterador a comparar
        * @return true si apuntan a la misma posición
        */
       bool operator==(const BTreeReverseIterator& other) const
       {
               if (m_CurrentPage == nullptr && other.m_CurrentPage == nullptr)
                       return true;
               return m_CurrentPage == other.m_CurrentPage && m_CurrentIndex == other.m_CurrentIndex;
       }
       
       /**
        * @brief Compara dos iteradores por desigualdad
        * @param other Otro iterador a comparar
        * @return true si apuntan a posiciones diferentes
        */
       bool operator!=(const BTreeReverseIterator& other) const
       {
               return !(*this == other);
       }
};

/**
 * @brief Árbol B (B-Tree) genérico con soporte para iteradores y concurrencia
 * 
 * Esta clase implementa un árbol B completo con las siguientes características:
 * - Operaciones de inserción, búsqueda y eliminación
 * - Iteradores forward para recorrido ascendente y descendente
 * - Thread-safety mediante std::shared_mutex (múltiples lectores o un solo escritor)
 * - Soporte para claves únicas o duplicadas
 * - Persistencia con serialización a disco
 * 
 * @tparam Trait Trait que define los tipos (keyType, ObjIDType, Compare)
 */
template <typename Trait>
class BTree // this is the full version of the BTree
{
       typedef typename Trait::keyType    keyType;
       typedef typename Trait::ObjIDType    ObjIDType;
       
       typedef CBTreePage <Trait> BTNode;// useful shorthand

public:
       typedef typename BTNode::ObjectInfo      ObjectInfo;
       
       // Soporte para iteradores forward y reverso
       friend class BTreeIterator<Trait>;
       friend class BTreeReverseIterator<Trait>;
       typedef BTreeIterator<Trait> iterator;
       typedef BTreeReverseIterator<Trait> reverse_iterator;

public:
       /**
        * @brief Constructor del BTree
        * @param order Orden del árbol (número de hijos por nodo)
        * @param unique Si true, no permite claves duplicadas
        */
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
              : m_Order(order),
                m_Root(2 * order  + 1, unique),
                m_Unique(unique),
                m_NumKeys(0)
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Height = 1;
       }

       /**
        * @brief Constructor de movimiento: transfiere la propiedad eficientemente
        * @param other Árbol a mover (queda en estado válido pero vacío)
        * @note Thread-safe: bloquea ambos objetos
        */
       BTree(BTree&& other) noexcept 
              : m_Mutex()
       {
              std::scoped_lock lock(m_Mutex, other.m_Mutex);  // Bloquear ambos
              
              m_Order = other.m_Order;
              m_Root = std::move(other.m_Root);
              m_Height = std::exchange(other.m_Height, 1);    // Toma valor y resetea a 1
              m_Unique = other.m_Unique;
              m_NumKeys = std::exchange(other.m_NumKeys, 0);   // Toma valor y resetea a 0
       }
       
       /**
        * @brief Asignación por movimiento: transfiere propiedad a objeto existente
        * @param other Árbol a mover
        * @return Referencia a este árbol
        * @note Thread-safe: bloquea ambos objetos (this y other) en orden para evitar deadlock
        */
       BTree& operator=(BTree&& other) noexcept 
       {
              if (this != &other)
              {
                     // Bloquear ambos mutex simultáneamente para evitar deadlock
                     std::scoped_lock lock(m_Mutex, other.m_Mutex);
                     
                     m_Order = other.m_Order;
                     m_Root = std::move(other.m_Root);
                     m_Height = std::exchange(other.m_Height, 1);    // Transfiere y resetea
                     m_Unique = other.m_Unique;
                     m_NumKeys = std::exchange(other.m_NumKeys, 0);  // Transfiere y resetea
              }
              return *this;
       }
       
       /** @brief Destructor del BTree */
       ~BTree() {}
       
       /**
        * @brief Inserta una clave en el árbol
        * @param key Clave a insertar
        * @param ObjID Identificador del objeto asociado
        * @return true si se insertó correctamente, false si la clave ya existe (en modo único)
        * @note Esta operación es thread-safe (bloqueo exclusivo)
        */
       bool            Insert (const keyType key, const long ObjID);
       
       /**
        * @brief Elimina una clave del árbol
        * @param key Clave a eliminar
        * @param ObjID Identificador del objeto asociado
        * @return true si se eliminó correctamente, false si no se encontró
        * @note Esta operación es thread-safe (bloqueo exclusivo)
        */
       bool            Remove (const keyType key, const long ObjID);
       
       /**
        * @brief Busca una clave en el árbol
        * @param key Clave a buscar
        * @return ObjID asociado si se encuentra, -1 si no existe
        * @note Esta operación es thread-safe (bloqueo compartido para lectura)
        */
       ObjIDType       Search (const keyType key)
       {      
              std::shared_lock<std::shared_mutex> lock(m_Mutex);  // Bloqueo compartido para lectura
              ObjIDType ObjID = -1;
              m_Root.Search(key, ObjID);
              return ObjID;
       }
       
       /**
        * @brief Retorna el número de claves en el árbol
        * @return Cantidad total de claves almacenadas
        * @note Esta operación es thread-safe (bloqueo compartido)
        */
       size_t            size()  const
       { 
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              return m_NumKeys; 
       }
       
       /**
        * @brief Retorna la altura del árbol
        * @return Altura del árbol (número de niveles)
        * @note Esta operación es thread-safe (bloqueo compartido)
        */
       size_t            height() const
       { 
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              return m_Height;      
       }
       
       /**
        * @brief Retorna el orden del árbol
        * @return Orden del BTree (número de hijos por nodo)
        */
       size_t            GetOrder() const { return m_Order;     }

       /**
        * @brief Imprime el árbol en un stream de salida
        * @param os Stream de salida donde imprimir
        * @note Esta operación es thread-safe (bloqueo compartido)
        */
       void            Print (ostream &os) const
       {               
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              m_Root.Print(os);                              
       }
       
       /**
        * @brief Guarda el árbol en un stream binario
        * @param os Stream de salida
        * @return Referencia al stream
        */
       std::ostream&   Write(std::ostream& os) const;
       
       /**
        * @brief Carga el árbol desde un stream binario
        * @param is Stream de entrada
        * @return Referencia al stream
        */
       std::istream&   Read(std::istream& is);
       
       /**
        * @brief Aplica una función a cada elemento del árbol (variadic template)
        * @tparam Func Tipo de la función a aplicar
        * @tparam Args Tipos de argumentos adicionales
        * @param func Función a aplicar a cada elemento
        * @param args Argumentos adicionales para la función
        */
       template <typename Func, typename... Args>
       void            ForEach(Func&& func, Args&&... args)
       {               m_Root.ForEach(std::forward<Func>(func), 0, std::forward<Args>(args)...);              }
       
       /**
        * @brief Encuentra el primer elemento que cumple un predicado
        * @tparam Pred Tipo del predicado
        * @tparam Args Tipos de argumentos adicionales
        * @param predicate Predicado a evaluar
        * @param args Argumentos adicionales para el predicado
        * @return Puntero al primer elemento que cumple el predicado, nullptr si no existe
        */
       template <typename Pred, typename... Args>
       ObjectInfo*     FirstThat(Pred&& predicate, Args&&... args)
       {               return m_Root.FirstThat(std::forward<Pred>(predicate), 0, std::forward<Args>(args)...);        }
       
       /**
        * @brief Operador de inserción en stream
        * @param os Stream de salida
        * @param tree Árbol a imprimir
        * @return Referencia al stream
        */
       friend std::ostream& operator<<(std::ostream& os, const BTree<Trait>& tree)
       {               tree.Print(os);  return os;              }
       
       /**
        * @brief Retorna iterador al elemento más pequeño del árbol
        * @return Iterador forward apuntando a la clave mínima (leftmost)
        * @note Thread-safe: usa bloqueo compartido (shared_lock) para lectura
        */
       iterator begin()
       {
               std::shared_lock<std::shared_mutex> lock(m_Mutex);  // Bloqueo compartido
               
               if (m_NumKeys == 0)
                       return end();
               
               // Descender hasta el hijo más a la izquierda
               BTNode* page = &m_Root;
               while (page->m_SubPages[0])
                       page = page->m_SubPages[0];
               
               return iterator(page, 0);
       }
       
       /**
        * @brief Retorna iterador centinela que marca el final del recorrido
        * @return Iterador nulo (nullptr) que indica fin de recorrido forward
        */
       iterator end()
       {
               return iterator(nullptr, 0);
       }
       
       /**
        * @brief Retorna iterador reverso al elemento más grande del árbol
        * @return Iterador reverso apuntando a la clave máxima (rightmost)
        * @note Thread-safe: usa bloqueo compartido (shared_lock) para lectura
        */
       reverse_iterator rbegin()
       {
               std::shared_lock<std::shared_mutex> lock(m_Mutex);  // Bloqueo compartido
               
               if (m_NumKeys == 0)
                       return rend();
               
               // Descender hasta el hijo más a la derecha
               BTNode* page = &m_Root;
               while (page->m_SubPages[page->m_KeyCount])
                       page = page->m_SubPages[page->m_KeyCount];
               
               return reverse_iterator(page, page->m_KeyCount - 1);
       }
       
       /**
        * @brief Retorna iterador centinela que marca el inicio del recorrido reverso
        * @return Iterador nulo (nullptr) que indica fin de recorrido backward
        */
       reverse_iterator rend()
       {
               return reverse_iterator(nullptr, 0);
       }

protected:
       BTNode          m_Root;     ///< Nodo raíz del árbol
       size_t          m_Height;   ///< Altura del árbol
       size_t          m_Order;    ///< Orden del árbol (número de hijos por nodo)
       size_t          m_NumKeys;  ///< Número total de claves en el árbol
       bool            m_Unique;   ///< Si true, no permite claves duplicadas
       
       mutable std::shared_mutex m_Mutex;  ///< Permite múltiples lectores o un solo escritor
};     

// ============================================================================
// Implementación de métodos de BTree
// ============================================================================

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const long ObjID){
       std::unique_lock<std::shared_mutex> lock(m_Mutex);  // Bloqueo exclusivo para escritura
       
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
       std::unique_lock<std::shared_mutex> lock(m_Mutex);  // Bloqueo exclusivo para escritura
       
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if( error == bt_duplicate || error == bt_nofound )
               return false;
       m_NumKeys--;

       if( error == bt_rootmerged )
               m_Height--;
       return true;
}

template <typename Trait>
std::ostream& BTree<Trait>::Write(std::ostream& os) const
{
       std::shared_lock<std::shared_mutex> lock(m_Mutex);  // Bloqueo compartido para lectura
       
       // Write metadata
       os << m_Order << " " << m_Height << " " << m_NumKeys << " " << m_Unique << "\n";
       
       // Write tree structure recursively
       const_cast<BTNode&>(m_Root).WriteStructure(os);
       
       return os;
}

template <typename Trait>
std::istream& BTree<Trait>::Read(std::istream& is)
{
       std::unique_lock<std::shared_mutex> lock(m_Mutex);  // Bloqueo exclusivo para escritura
       
       // Read metadata
       is >> m_Order >> m_Height >> m_NumKeys >> m_Unique;
       
       // Reinitialize tree
       m_Root.Reset();
       m_Root = BTNode(2 * m_Order + 1, m_Unique);
       m_Root.SetMaxKeysForChilds(m_Order);
       
       // Read tree structure recursively
       m_Root.ReadStructure(is);
       
       return is;
}

// ============================================================================
// Implementación de operadores de BTreeIterator (forward: menor a mayor)
// ============================================================================

/**
 * @brief Implementación de operator++ para BTreeIterator (navegación forward)
 * 
 * Navega el árbol B en orden ascendente (in-order traversal de menor a mayor).
 * 
 * Algoritmo:
 * 1. Si existe hijo derecho, descender al hijo más a la izquierda de ese subárbol
 * 2. Si no hay hijo derecho pero hay más keys a la derecha, avanzar en el nodo actual
 * 3. Si no hay más keys, subir al padre hasta encontrar una key pendiente
 * 4. Si llegamos al final del árbol, marcar como nullptr (end)
 */
template <typename Trait>
BTreeIterator<Trait>& BTreeIterator<Trait>::operator++()
{
       if (!m_CurrentPage) return *this;
       
       // Caso 1: Si hay hijo derecho, ir al leftmost de ese subárbol
       BTPage* rightChild = m_CurrentPage->m_SubPages[m_CurrentIndex + 1];
       if (rightChild != nullptr) {
               BTPage* leftmost = rightChild;
               while (leftmost->m_SubPages[0] != nullptr) {
                       leftmost = leftmost->m_SubPages[0];
               }
               m_CurrentPage = leftmost;
               m_CurrentIndex = 0;
               return *this;
       }
       
       // Caso 2: Avanzar en el nodo actual si hay más keys a la derecha
       if (m_CurrentIndex + 1 < m_CurrentPage->m_KeyCount) {
               m_CurrentIndex++;
               return *this;
       }
       
       // Caso 3: Subir al padre hasta encontrar una key no visitada
       BTPage* child = m_CurrentPage;
       BTPage* parent = m_CurrentPage->m_Parent;
       
       while (parent != nullptr) {
               // Buscar posición del hijo en el arreglo de SubPages del padre
               size_t childPos = 0;
               while (childPos <= parent->m_KeyCount && parent->m_SubPages[childPos] != child) {
                       childPos++;
               }
               
               // Si encontramos una key válida en el padre, esa es la siguiente
               if (childPos < parent->m_KeyCount) {
                       m_CurrentPage = parent;
                       m_CurrentIndex = childPos;
                       return *this;
               }
               
               // Continuar subiendo en el árbol
               child = parent;
               parent = parent->m_Parent;
       }
       
       // Caso 4: No hay más elementos, hemos alcanzado end()
       m_CurrentPage = nullptr;
       m_CurrentIndex = 0;
       return *this;
}

// ============================================================================
// Implementación de operadores de BTreeReverseIterator (backward: mayor a menor)
// ============================================================================

/**
 * @brief Implementación de operator++ para BTreeReverseIterator (navegación backward)
 * 
 * Navega el árbol B en orden descendente (reverse in-order traversal de mayor a menor).
 * Aunque usa operator++, navega hacia ATRÁS (de mayor a menor).
 * 
 * Algoritmo:
 * 1. Si existe hijo izquierdo, descender al hijo más a la derecha de ese subárbol
 * 2. Si no hay hijo izquierdo pero hay más keys a la izquierda, retroceder en el nodo actual
 * 3. Si no hay más keys, subir al padre hasta encontrar una key pendiente
 * 4. Si llegamos al inicio del árbol, marcar como nullptr (rend)
 */
template <typename Trait>
BTreeReverseIterator<Trait>& BTreeReverseIterator<Trait>::operator++()
{
       if (!m_CurrentPage) return *this;
       
       // Caso 1: Si hay hijo izquierdo, ir al rightmost de ese subárbol
       BTPage* leftChild = m_CurrentPage->m_SubPages[m_CurrentIndex];
       if (leftChild != nullptr) {
               BTPage* rightmost = leftChild;
               while (rightmost->m_SubPages[rightmost->m_KeyCount] != nullptr) {
                       rightmost = rightmost->m_SubPages[rightmost->m_KeyCount];
               }
               m_CurrentPage = rightmost;
               m_CurrentIndex = rightmost->m_KeyCount - 1;
               return *this;
       }
       
       // Caso 2: Retroceder en el nodo actual si hay más keys a la izquierda
       if (m_CurrentIndex > 0) {
               m_CurrentIndex--;
               return *this;
       }
       
       // Caso 3: Subir al padre hasta encontrar una key no visitada
       BTPage* child = m_CurrentPage;
       BTPage* parent = m_CurrentPage->m_Parent;
       
       while (parent != nullptr) {
               // Buscar posición del hijo en el arreglo de SubPages del padre
               size_t childPos = 0;
               while (childPos <= parent->m_KeyCount && parent->m_SubPages[childPos] != child) {
                       childPos++;
               }
               
               // Si el hijo está en una posición > 0, la key anterior del padre es la siguiente
               if (childPos > 0) {
                       m_CurrentPage = parent;
                       m_CurrentIndex = childPos - 1;
                       return *this;
               }
               
               // Continuar subiendo en el árbol
               child = parent;
               parent = parent->m_Parent;
       }
       
       // Caso 4: No hay más elementos, hemos alcanzado rend()
       m_CurrentPage = nullptr;
       m_CurrentIndex = 0;
       return *this;
}

#endif