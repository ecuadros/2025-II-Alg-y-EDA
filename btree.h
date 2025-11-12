#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

/**
 * @struct BTreeTrait
 * @brief Estructura que define los tipos de datos y la función de comparación para el BTree.
 * 
 * Esta estructura es una especie de "parámetro de tipo" para el árbol B (BTree),
 * donde se especifica el tipo de las claves, los identificadores de objeto, y la función de comparación.
 * 
 * @tparam _keyType Tipo de las claves.
 * @tparam _ObjIDType Tipo del identificador de objeto.
 */
template <typename _keyType, typename _ObjIDType, typename _CompareFunction = std::less<_keyType>>
struct BTreeTrait
{
    using keyType = _keyType; ///< Tipo de las claves.
    using ObjIDType = _ObjIDType; ///< Tipo de los identificadores de objeto.
    
    // TODO: agregar función de comparación
    using CompareFunction = _CompareFunction;
};


//forward iterator
template <typename Trait>
class BTreeForwardIterator {
public:
    typedef std::forward_iterator_tag iterator_category;
    typedef std::ptrdiff_t difference_type;

    typedef typename CBTreePage<Trait>::ObjectInfo value_type;
    typedef CBTreePage<Trait> BTNode;

    typedef value_type* pointer;
    typedef value_type& reference;

private: 
    BTNode* current_node;
    size_t current_pos;

    // Constructores
    BTreeForwardIterator() {
        current_node = nullptr;
        current_pos = 0;
    }
    
    BTreeForwardIterator(BTNode* node, size_t position) {
        current_node = node;
        current_pos = position;
    }

    // Operadores de acceso
    reference operator*() const {
        return current_node->m_Keys[current_pos];
    }
    
    pointer operator->() const {
        return &(current_node->m_Keys[current_pos]);
    }
    // Operadores de comparación
    bool operator==(const BTreeForwardIterator& other) const {
        if(current_node == nullptr && other.current_node == nullptr)
            return true;
        return current_node == other.current_node && current_pos == other.current_pos;
    }
    
    bool operator!=(const BTreeForwardIterator& other) const {
        return !(*this == other);
    }
    
    // Operador de incremento 
    BTreeForwardIterator& operator++() {
        if (!current_node) return *this;
        
        // Si hay subárbol derecho, ir al mínimo de ese subárbol
        if (current_node->m_SubPages[current_pos + 1]) {
            current_node = current_node->m_SubPages[current_pos + 1];
            MoveToFirst();
        }
        // Si no hay subárbol derecho, avanzar en el nodo actual
        else if (current_pos + 1 < current_node->m_KeyCount) {
            current_pos++;
        }
        // Si terminamos el nodo actual, subir al padre
        else {
            MoveToNextParent();
        }
        
        return *this;
    }
    
    // Operador de incremento (it++)
    BTreeForwardIterator operator++(int) {
        BTreeForwardIterator temp = *this;
        ++(*this);
        return temp;
    }
    
    // Moverse al primer elemento del subárbol actual (más a la izquierda)
    void MoveToFirst() {
        while (current_node && current_node->m_SubPages[0]) {
            current_node = current_node->m_SubPages[0];
        }
        current_pos = 0;
    }
    
    // Subir al siguiente nodo padre que tenga elementos no visitados
    void MoveToNextParent() {
        if (!current_node || !current_node->m_Parent) {
            // Llegamos al final del rbol
            current_node = nullptr;
            current_pos = 0;
            return;
        }
        
        BTNode* parent = current_node->m_Parent;
        
        // Encontrar la posición del nodo actual en el padre
        size_t pos = 0;
        for (; pos <= parent->m_KeyCount; pos++) {
            if (parent->m_SubPages[pos] == current_node) {
                break;
            }
        }
        
        // Si el nodo actual está en la posición pos, el siguiente elemento
        // está en parent->m_Keys[pos] (si pos < m_KeyCount)
        if (pos < parent->m_KeyCount) {
            current_node = parent;
            current_pos = pos;
        } else {
            // Si estamos en la última subpágina, subir más
            current_node = parent;
            MoveToNextParent();
        }
    }
    
    friend class BTree<Trait>;
};


//backward iterator
template <typename Trait>
class BTreeBackwardIterator {
public:
    typedef std::forward_iterator_tag iterator_category;
    typedef std::ptrdiff_t difference_type;

    typedef typename CBTreePage<Trait>::ObjectInfo value_type;
    typedef CBTreePage<Trait> BTNode;

    typedef value_type* pointer;
    typedef value_type& reference;

    // Constructores
    BTreeBackwardIterator(){
        current_node = nullptr;
        current_pos = 0;
    }
    
    BTreeBackwardIterator(BTNode* node, size_t position) {
        current_node = node;
        current_pos = position;
    }
    
    // Operadores de comparación
    bool operator==(const BTreeBackwardIterator& other) const {
        return current_node == other.current_node && current_pos == other.current_pos;
    }
    
    bool operator!=(const BTreeBackwardIterator& other) const {
        return !(*this == other);
    }
    
    // Operadores de acceso
    reference operator*() const {
        return current_node->m_Keys[current_pos];
    }
    
    pointer operator->() const {
        return &(current_node->m_Keys[current_pos]);
    }
    
    // Operador de incremento (++it) - Backward (va hacia atrás)
    BTreeBackwardIterator& operator++() {
        if (!current_node) return *this;
        
        // Si hay subárbol izquierdo, ir al máximo de ese subárbol
        if (current_node->m_SubPages[current_pos]) {
            current_node = current_node->m_SubPages[current_pos];
            MoveToLast();
        }
        // Si estamos en el primer elemento del nodo, subir al padre
        else if (current_pos == 0) {
            MoveToPrevParent();
        }
        // Si no, retroceder en el nodo actual
        else {
            current_pos--;
        }
        
        return *this;
    }
    
    // Operador de incremento (it++)
    BTreeBackwardIterator operator++(int) {
        BTreeBackwardIterator temp = *this;
        ++(*this);
        return temp;
    }
    
private:
    BTNode* current_node;
    size_t current_pos;
    
    // Moverse al último elemento del subárbol actual (más a la derecha)
    void MoveToLast() {
        while (current_node && current_node->m_SubPages[current_node->m_KeyCount]) {
            current_node = current_node->m_SubPages[current_node->m_KeyCount];
        }
        if (current_node && current_node->m_KeyCount > 0) {
            current_pos = current_node->m_KeyCount - 1;
        }
    }
    
    // Subir al anterior nodo padre que tenga elementos no visitados
    void MoveToPrevParent() {
        if (!current_node || !current_node->m_Parent) {
            // Llegamos al inicio del árbol (fin del recorrido backward)
            current_node = nullptr;
            current_pos = 0;
            return;
        }
        
        BTNode* parent = current_node->m_Parent;
        
        // Encontrar la posición del nodo actual en el padre
        size_t pos = 0;
        for (; pos <= parent->m_KeyCount; pos++) {
            if (parent->m_SubPages[pos] == current_node) {
                break;
            }
        }
        
        // Si el nodo actual está en la posición pos > 0, el elemento anterior
        // está en parent->m_Keys[pos-1]
        if (pos > 0) {
            current_node = parent;
            current_pos = pos - 1;
        } else {
            // Si estamos en la primera subpágina, subir más
            current_node = parent;
            MoveToPrevParent();
        }
    }
    
    friend class BTree<Trait>;
};


/**
 * @class BTree
 * @brief Implementación de un árbol B genérico.
 * 
 * Esta clase implementa un árbol B (BTree) con soporte para inserción, eliminación,
 * búsqueda y serialización en formato binario. Es una implementación flexible que
 * permite especificar el orden del árbol y si los elementos deben ser únicos.
 * 
 * @tparam Trait Tipo de la estructura BTreeTrait que define los parámetros del árbol.
 */
template <typename Trait>
class BTree
{
    typedef typename Trait::keyType keyType; ///< Tipo de las claves del árbol.
    typedef typename Trait::ObjIDType ObjIDType; ///< Tipo de los identificadores de objeto.
    typedef typename Trait::CompareFunction CompareFunction; ///< Función de comparación.
    typedef CBTreePage<Trait> BTNode; ///< Nodo del árbol B (CBTreePage).
    
public:
    using forward_iterator = BTreeForwardIterator<Trait>;
    using backward_iterator = BTreeBackwardIterator<Trait>;
    // typedef CBTreePage<Trait> BTNode; ///< Nodo del árbol B (CBTreePage) solo para el testmove
    typedef typename BTNode::ObjectInfo ObjectInfo; ///< Información del objeto almacenado en el nodo.

    /**
     * @brief Constructor del árbol B.
     * 
     * Crea un árbol B con el orden especificado y la opción de elementos únicos.
     * 
     * @param order Orden del árbol B (número máximo de hijos por nodo).
     * @param unique Si es verdadero, los elementos deben ser únicos.
     */
    BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
        : m_Order(order), m_Root(2 * order + 1, unique), m_Unique(unique), m_NumKeys(0)
    {
        m_Root.SetMaxKeysForChilds(order);
        m_Height = 1;
    }

    ~BTree() {}

    //move constructor en btree
    BTree(BTree<Trait> &&other);

    // move asignment operator en btree
    BTree &operator=(BTree<Trait> &&other);


    forward_iterator begin() {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        
        if (m_NumKeys == 0) {
            return end();
        }
        
        // Buscar el nodo más a la izquierda 
        BTNode* current = &m_Root;
        while (current->m_SubPages[0]) {
            current = current->m_SubPages[0];
        }
        
        return forward_iterator(current, 0);
    }
    
    forward_iterator end() {
        return forward_iterator(nullptr, 0);
    }

//para la parte del backward rbegin y rend
    backward_iterator rbegin() {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        
        if (m_NumKeys == 0) {
            return rend();
        }
        
        // Buscar el nodo más a la derecha (lo opuesto al forward)
        BTNode* current = &m_Root;
        while (current->m_SubPages[current->m_KeyCount]) {
            current = current->m_SubPages[current->m_KeyCount];
        }
        
        if (current->m_KeyCount > 0) {
            return backward_iterator(current, current->m_KeyCount - 1);
        }
        
        return rend();
    }
    

    backward_iterator rend() {
        return backward_iterator(nullptr, 0);
    }

    /**
     * @brief Inserta una nueva clave en el árbol B.
     *
     * Si la clave ya existe y se permite duplicados, no se realiza ninguna acción.
     *
     * @param key La clave a insertar.
     * @param ObjID El identificador de objeto asociado con la clave.
     * @return `true` si la inserción fue exitosa, `false` si hubo un error (como duplicados).
     */
    bool Insert(const keyType key, const ObjIDType ObjID);

    /**
     * @brief Elimina una clave del árbol B.
     * 
     * Si la clave no existe o hay algún error, no se realiza ninguna acción.
     * 
     * @param key La clave a eliminar.
     * @param ObjID El identificador de objeto asociado con la clave.
     * @return `true` si la eliminación fue exitosa, `false` si hubo un error (como que la clave no fue encontrada).
     */
    bool Remove(const keyType key, const ObjIDType ObjID);

    /**
     * @brief Busca una clave en el árbol B.
     * 
     * Realiza una búsqueda para encontrar el identificador de objeto asociado a una clave.
     * 
     * @param key La clave a buscar.
     * @return El identificador de objeto asociado con la clave, o `-1` si no se encuentra.
     */
    ObjIDType Search(const keyType key)
    {
        // std::lock_guard<std::shared_mutex> lock(m_mutex);
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        ObjIDType ObjID = ObjIDType();
        
        if (m_Root.Search(key, ObjID))
            return ObjID;
        return -1;

    }
    /**
     * @brief Obtiene el número de claves almacenadas en el árbol B.
     * 
     * @return El número de claves en el árbol.
     */
    size_t size() { 
        // std::lock_guard<std::shared_mutex> lock(m_mutex);
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_NumKeys; 
    }

    /**
     * @brief Obtiene la altura del árbol B.
     * 
     * @return La altura del árbol.
     */
    size_t height() { 
        // std::lock_guard<std::shared_mutex> lock(m_mutex); 
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_Height; 
    }

    /**
     * @brief Obtiene el orden del árbol B.
     * 
     * @return El orden del árbol.
     */
    size_t GetOrder() { 
        // std::lock_guard<std::shared_mutex> lock(m_mutex); 
        std::shared_lock<std::shared_mutex> lock(m_mutex); 
        return m_Order; 
    }

    /**
     * @brief Imprime el árbol B en un flujo de salida.
     * 
     * @param os El flujo de salida.
     */
    void Print(std::ostream &os) { 
        // std::lock_guard<std::shared_mutex> lock(m_mutex); 
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        m_Root.Print(os); 
    }

    /**
     * @brief Escribe el árbol B en un flujo de salida en formato textual.
     * 
     * @param os El flujo de salida.
     * @return El flujo de salida.
     */
    std::ostream& Write(std::ostream &os);

    /**
     * @brief Lee un árbol B desde un flujo de entrada en formato textual.
     * 
     * @param is El flujo de entrada.
     * @return El flujo de entrada.
     */
    std::istream& Read(std::istream &is);

    /**
     * @brief Escribe el árbol B en un flujo de salida en formato simple.
     * 
     * @param os El flujo de salida.
     * @return El flujo de salida.
     */
    std::ostream& WriteBinaryTreeFormat(std::ostream& os);

    /**
     * @brief Lee un árbol B desde un flujo de entrada en formato simple.
     * 
     * @param is El flujo de entrada.
     * @return El flujo de entrada.
     */
    std::istream& ReadBinaryTreeFormat(std::istream& is);

    /**
     * @brief Realiza una acción sobre cada elemento del árbol B.
     * 
     * @tparam Function Tipo de la función que se aplicará a cada elemento.
     * @param fn La función a aplicar.
     */
    template <typename Function>
    void ForEach(Function fn);

    /**
     * @brief Busca el primer objeto que cumpla con una condición.
     * 
     * @tparam Function Tipo de la función de condición.
     * @param fn La función de condición.
     * @return El objeto que cumple la condición, o `nullptr` si no se encuentra.
     */
    template <typename Function>
    ObjectInfo* FirstThat(Function fn);

protected:
    BTNode m_Root; ///< Nodo raíz del árbol B.
    size_t m_Height; ///< Altura del árbol.
    size_t m_Order; ///< Orden del árbol (máximo número de hijos por nodo).
    size_t m_NumKeys; ///< Número de claves en el árbol.
    bool m_Unique; ///< Si es `true`, los elementos deben ser únicos.

    std::shared_mutex m_mutex;
};


//move constructor en btree
template <typename Trait>
BTree<Trait>::BTree(BTree&& other){
    // std::lock_guard<std::shared_mutex> lock(other.m_mutex);
    std::unique_lock<std::shared_mutex> lock(other.m_mutex);
    m_Root      = std::move(other.m_Root);    
    m_Height    = std::exchange(other.m_Height, 1);
    m_Order     = std::exchange(other.m_Order, DEFAULT_BTREE_ORDER);
    m_NumKeys   = std::exchange(other.m_NumKeys, 0);
    m_Unique    = std::exchange(other.m_Unique, true);
}

template <typename Trait>
BTree<Trait>& BTree<Trait>::operator=(BTree &&other){
    if(this != &other){
        std::scoped_lock locks(m_mutex, other.m_mutex);  //es para evitar el deadlock
        m_Root      = std::move(other.m_Root);        
        m_Height    = std::exchange(other.m_Height, 1);
        m_Order     = std::exchange(other.m_Order, DEFAULT_BTREE_ORDER);
        m_NumKeys   = std::exchange(other.m_NumKeys, 0);
        m_Unique    = std::exchange(other.m_Unique, true);
    }
    
    return *this;
}

/**
 * @brief Inserta una nueva clave en el árbol B.
 * 
 * Esta es la implementación del método Insert.
 * 
 * @param key La clave a insertar.
 * @param ObjID El identificador de objeto asociado con la clave.
 * @return `true` si la inserción fue exitosa, `false` si hubo un error.
 */
template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const ObjIDType ObjID)
{
    // std::lock_guard<std::shared_mutex> lock(m_mutex);
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    bt_ErrorCode error = m_Root.Insert(key, ObjID);
    if (error == bt_duplicate)
        return false;
    m_NumKeys++;
    if (error == bt_overflow) {
        m_Root.SplitRoot();
        m_Height++;
    }
    return true;
}

/**
 * @brief Elimina una clave del árbol B.
 * 
 * Esta es la implementación del método Remove.
 * 
 * @param key La clave a eliminar.
 * @param ObjID El identificador de objeto asociado con la clave.
 * @return `true` si la eliminación fue exitosa, `false` si hubo un error.
 */
template <typename Trait>
bool BTree<Trait>::Remove(const keyType key, const ObjIDType ObjID)
{
    // std::lock_guard<std::shared_mutex> lock(m_mutex);
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    bt_ErrorCode error = m_Root.Remove(key, ObjID);
    if (error == bt_duplicate || error == bt_nofound)
        return false;
    m_NumKeys--;
    if (error == bt_rootmerged)
        m_Height--;
    return true;
}

//implementamos foreach
template <typename Trait>
template <typename Function>
void BTree<Trait>::ForEach(Function fn)
{
    // std::lock_guard<std::shared_mutex> lock(m_mutex);
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    m_Root.ForEach(fn, 0);
}

//implementamos FirstThat
template <typename Trait>
template <typename Function>
typename BTree<Trait>::ObjectInfo* BTree<Trait>::FirstThat(Function fn)
{
    // std::lock_guard<std::shared_mutex> lock(m_mutex);
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_Root.FirstThat(fn, 0);
}

/**
 * @brief Escribe el árbol B en un formato binario.
 * 
 * Esta es la implementación del método WriteBinaryTreeFormat.
 * 
 * @param os El flujo de salida.
 * @return El flujo de salida.
 */


 template <typename Trait>
std::ostream& BTree<Trait>::Write(std::ostream &os) {
    // std::lock_guard<std::shared_mutex> lock(m_mutex);
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    // Cabecera 
    os << "BTree " << m_Order << " " << m_Height << " " << m_NumKeys << " " << m_Unique << "\n";
    
    // Escribimos la raíz
    m_Root.Write(os);
    
    return os;
}

template <typename Trait>
std::istream& BTree<Trait>::Read(std::istream &is) {
    // std::lock_guard<std::shared_mutex> lock(m_mutex);
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    std::string tag;
    is >> tag; // leemos la cabecera 
    
    if(tag == "BTree") {
        is >> m_Order >> m_Height >> m_NumKeys >> m_Unique;
        
        // Leemos la raíz
        m_Root.Read(is);
    }
    
    return is;
}

template <typename Trait>
std::ostream& BTree<Trait>::WriteBinaryTreeFormat(std::ostream& os)
{
    // std::lock_guard<std::shared_mutex> lock(m_mutex);
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    os << "BTreeSimple " << m_NumKeys << " elements: ";
    ForEach([&os](auto& info, size_t level) {
        os << info.key << " ";
    });
    return os;
}

/**
 * @brief Lee el árbol B desde un flujo de entrada en formato simple.
 * 
 * Esta es la implementación del método `ReadBinaryTreeFormat`. Lee una secuencia de caracteres desde
 * el flujo de entrada y usa cada uno para insertar claves en el árbol B. Asume que las claves están
 * representadas como dígitos en una línea de texto.
 * 
 * @param is El flujo de entrada desde donde se lee la representación del árbol B.
 * @return El flujo de entrada.
 */
template <typename Trait>
std::istream& BTree<Trait>::ReadBinaryTreeFormat(std::istream& is)
{
    // std::lock_guard<std::recursive_mutex> lock(m_mutex);
    std::string line;
    std::getline(is, line); // Leemos la primera línea

    m_Root.Reset(); // Reseteamos el árbol.
    m_NumKeys = 0;
    m_Height = 1;

    // Convertimos cada carácter de la línea a una clave numérica y la insertamos en el árbol
    for (char c : line) {
        if (std::isdigit(c)) {
            int key = c - '0';  // Convertir char a int
            Insert(key, key);
        }
    }

    return is;
}

/**
 * @brief Sobrecarga del operador de inserción para la impresión del árbol.
 * 
 * Permite imprimir el árbol B utilizando `std::ostream`. Esta sobrecarga usa el método `Print` del
 * árbol para generar la salida en el flujo.
 * 
 * @tparam Trait El tipo de los parámetros del árbol.
 * @param os El flujo de salida en el que se imprimirá el árbol.
 * @param tree El árbol B a imprimir.
 * @return El flujo de salida.
 */
template <typename Trait>
std::ostream& operator<<(std::ostream& os, BTree<Trait>& tree)
{
    // std::lock_guard<std::mutex> lock(m_mutex);
    // std::unique_lock<std::shared_mutex> lock(m_mutex);
    tree.Print(os);  // Llama al método Print de la clase BTree para generar la salida
    return os;
}

#endif // __BTREE_H__
