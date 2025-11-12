#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <fstream>
#include <memory>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <iterator>
#include <string>
#include <utility>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

template <typename Trait>
class BTree;
template <typename Trait>
class BTreeIterator;
template <typename Trait>
class BTreeReverseIterator;

const size_t MaxHeight = 5;

/**
 * @brief Estructura de características para el árbol B.
 *
 * @tparam _keyType Tipo de clave del árbol
 * @tparam _ObjIDType Tipo de identificador del objeto asociado a la clave
 * @tparam _Compare Función de comparación (por defecto std::less)
 */
template <typename _keyType, typename _ObjIDType, typename _Compare = std::less<_keyType>>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       using Compare = _Compare;
};

/** @brief Alias para árbol B con orden ascendente */
template <typename Key, typename Value>
using BTreeAscTrait = BTreeTrait<Key, Value, std::less<Key>>;

/** @brief Alias para árbol B con orden descendente */
template <typename Key, typename Value>
using BTreeDescTrait = BTreeTrait<Key, Value, std::greater<Key>>;

/**
 * @brief Iterador directo para árbol B.
 *
 * @tparam Trait Características del árbol B
 */
template <typename Trait>
class BTreeIterator
{
       friend class BTree<Trait>;
       typedef CBTreePage<Trait> BTPage;
       typedef typename BTPage::ObjectInfo ObjectInfo;

private:
       BTPage *m_CurrentPage;
       size_t m_CurrentIndex;

       BTreeIterator(BTPage *page, size_t index) : m_CurrentPage(page), m_CurrentIndex(index) {}

public:
       using iterator_category = std::forward_iterator_tag;
       using value_type = ObjectInfo;
       using difference_type = std::ptrdiff_t;
       using pointer = ObjectInfo *;
       using reference = ObjectInfo &;

       /** @brief Constructor por defecto */
       BTreeIterator() : m_CurrentPage(nullptr), m_CurrentIndex(0) {}

       /** @brief Operador de desreferencia */
       ObjectInfo &operator*() { return m_CurrentPage->m_Keys[m_CurrentIndex]; }
       /** @brief Operador de acceso a miembro */
       ObjectInfo *operator->() { return &(m_CurrentPage->m_Keys[m_CurrentIndex]); }
       /** @brief Incremento prefijo */
       BTreeIterator &operator++();
       /** @brief Incremento postfijo */
       BTreeIterator operator++(int)
       {
              BTreeIterator temp = *this;
              ++(*this);
              return temp;
       }

       /** @brief Comparación de igualdad */
       bool operator==(const BTreeIterator &other) const
       {
              if (m_CurrentPage == nullptr && other.m_CurrentPage == nullptr)
                     return true;
              return m_CurrentPage == other.m_CurrentPage && m_CurrentIndex == other.m_CurrentIndex;
       }

       /** @brief Comparación de desigualdad */
       bool operator!=(const BTreeIterator &other) const
       {
              return !(*this == other);
       }
};

/**
 * @brief Iterador reverso para árbol B.
 *
 * @tparam Trait Características del árbol B
 */
template <typename Trait>
class BTreeReverseIterator
{
       friend class BTree<Trait>;
       typedef CBTreePage<Trait> BTPage;
       typedef typename BTPage::ObjectInfo ObjectInfo;

private:
       BTPage *m_CurrentPage;
       size_t m_CurrentIndex;

       BTreeReverseIterator(BTPage *page, size_t index) : m_CurrentPage(page), m_CurrentIndex(index) {}

public:
       using iterator_category = std::forward_iterator_tag;
       using value_type = ObjectInfo;
       using difference_type = std::ptrdiff_t;
       using pointer = ObjectInfo *;
       using reference = ObjectInfo &;

       /** @brief Constructor por defecto */
       BTreeReverseIterator() : m_CurrentPage(nullptr), m_CurrentIndex(0) {}

       /** @brief Operador de desreferencia */
       ObjectInfo &operator*() { return m_CurrentPage->m_Keys[m_CurrentIndex]; }
       /** @brief Operador de acceso a miembro */
       ObjectInfo *operator->() { return &(m_CurrentPage->m_Keys[m_CurrentIndex]); }
       /** @brief Incremento prefijo */
       BTreeReverseIterator &operator++();
       /** @brief Incremento postfijo */
       BTreeReverseIterator operator++(int)
       {
              BTreeReverseIterator temp = *this;
              ++(*this);
              return temp;
       }

       /** @brief Comparación de igualdad */
       bool operator==(const BTreeReverseIterator &other) const
       {
              if (m_CurrentPage == nullptr && other.m_CurrentPage == nullptr)
                     return true;
              return m_CurrentPage == other.m_CurrentPage && m_CurrentIndex == other.m_CurrentIndex;
       }

       /** @brief Comparación de desigualdad */
       bool operator!=(const BTreeReverseIterator &other) const
       {
              return !(*this == other);
       }
};

/**
 * @brief Árbol B balanceado para búsqueda y ordenamiento.
 *
 * Implementa un árbol B con características personalizables mediante Trait.
 * Soporta operaciones concurrentes mediante std::shared_mutex.
 *
 * @tparam Trait Estructura con tipos keyType, ObjIDType y función Compare
 */
template <typename Trait>
class BTree
{
       typedef typename Trait::keyType keyType;
       typedef typename Trait::ObjIDType ObjIDType;
       typedef CBTreePage<Trait> BTNode;

public:
       typedef typename BTNode::ObjectInfo ObjectInfo;
       friend class BTreeIterator<Trait>;
       friend class BTreeReverseIterator<Trait>;
       typedef BTreeIterator<Trait> iterator;
       typedef BTreeReverseIterator<Trait> reverse_iterator;

public:
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
           : m_Order(order),
             m_Root(2 * order + 1, unique),
             m_Unique(unique),
             m_NumKeys(0)
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Height = 1;
       }

       /** @brief Constructor de movimiento */
       BTree(BTree &&other) noexcept
       {
              std::unique_lock<std::shared_mutex> lock(other.m_Mutex);
              m_Root = std::move(other.m_Root);
              m_Height = std::exchange(other.m_Height, 1);
              m_Unique = other.m_Unique;
              m_NumKeys = std::exchange(other.m_NumKeys, 0);
       }

       /** @brief Operador de asignación por movimiento */
       BTree &operator=(BTree &&other) noexcept
       {
              if (this != &other)
              {
                     std::scoped_lock lock(m_Mutex, other.m_Mutex);
                     m_Order = other.m_Order;
                     m_Root = std::move(other.m_Root);
                     m_Height = std::exchange(other.m_Height, 1);
                     m_Unique = other.m_Unique;
                     m_NumKeys = std::exchange(other.m_NumKeys, 0);
              }
              return *this;
       }

       ~BTree() {}

       /**
        * @brief Inserta una clave con su identificador en el árbol.
        *
        * @param key Clave a insertar
        * @param ObjID Identificador del objeto asociado
        * @return true si la inserción fue exitosa, false si la clave es duplicada
        */
       bool Insert(const keyType key, const long ObjID);

       /**
        * @brief Elimina una clave del árbol.
        *
        * @param key Clave a eliminar
        * @param ObjID Identificador del objeto
        * @return true si la eliminación fue exitosa, false si no se encontró
        */
       bool Remove(const keyType key, const long ObjID);

       /**
        * @brief Busca una clave en el árbol.
        *
        * @param key Clave a buscar
        * @return Identificador del objeto si existe, -1 en caso contrario
        */
       ObjIDType Search(const keyType key)
       {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              ObjIDType ObjID = -1;
              m_Root.Search(key, ObjID);
              return ObjID;
       }

       /**
        * @brief Obtiene el número total de claves en el árbol.
        * @return Cantidad de elementos
        */
       size_t size() const
       {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              return m_NumKeys;
       }

       /**
        * @brief Obtiene la altura del árbol.
        * @return Altura (número de niveles)
        */
       size_t height() const
       {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              return m_Height;
       }

       /**
        * @brief Obtiene el orden del árbol.
        * @return Orden del árbol B
        */
       size_t GetOrder() const { return m_Order; }

       /**
        * @brief Imprime la estructura del árbol.
        *
        * @param os Stream de salida
        */
       void Print(std::ostream &os) const
       {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              m_Root.Print(os);
       }

       /**
        * @brief Escribe la estructura del árbol en un stream.
        *
        * @param os Stream de salida
        * @return Referencia al stream
        */
       std::ostream &Write(std::ostream &os) const;
       /**
        * @brief Lee la estructura del árbol desde un stream.
        *
        * @param is Stream de entrada
        * @return Referencia al stream
        */
       std::istream &Read(std::istream &is);

       /**
        * @brief Aplica una función a cada elemento del árbol (in-order).
        *
        * @tparam Func Tipo de función
        * @tparam Args Argumentos adicionales
        * @param func Función a aplicar
        * @param args Argumentos adicionales
        */
       template <typename Func, typename... Args>
       void ForEach(Func &&func, Args &&...args)
       {
              m_Root.ForEach(std::forward<Func>(func), 0, std::forward<Args>(args)...);
       }

       /**
        * @brief Busca el primer elemento que cumple un predicado.
        *
        * @tparam Pred Tipo de predicado
        * @tparam Args Argumentos adicionales
        * @param predicatem Predicado a aplicar
        * @param args Argumentos adicionales
        * @return Puntero al elemento encontrado o nullptr
        */
       template <typename Pred, typename... Args>
       ObjectInfo *FirstThat(Pred &&predicate, Args &&...args)
       {
              return m_Root.FirstThat(std::forward<Pred>(predicate), 0, std::forward<Args>(args)...);
       }

       /**
        * @brief Operador de salida para imprimir el árbol.
        *
        * @param os Stream de salida
        * @param tree Árbol a imprimir
        * @return Referencia al stream
        */
       friend std::ostream &operator<<(std::ostream &os, const BTree<Trait> &tree)
       {
              tree.Print(os);
              return os;
       }

       /**
        * @brief Obtiene un iterador al primer elemento.
        *
        * @return Iterador al inicio
        */
       iterator begin()
       {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);

              if (m_NumKeys == 0)
                     return end();

              BTNode *page = &m_Root;
              while (page->m_SubPages[0])
                     page = page->m_SubPages[0];

              return iterator(page, 0);
       }

       /**
        * @brief Obtiene un iterador al final del árbol.
        * @return Iterador al final
        */
       iterator end()
       {
              return iterator(nullptr, 0);
       }

       /**
        * @brief Obtiene un iterador reverso al último elemento.
        *
        * @return Iterador reverso al inicio
        */
       reverse_iterator rbegin()
       {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);

              if (m_NumKeys == 0)
                     return rend();

              BTNode *page = &m_Root;
              while (page->m_SubPages[page->m_KeyCount])
                     page = page->m_SubPages[page->m_KeyCount];

              return reverse_iterator(page, page->m_KeyCount - 1);
       }

       /**
        * @brief Obtiene un iterador reverso al inicio (antes del primer elemento).
        * @return Iterador reverso al final
        */
       reverse_iterator rend()
       {
              return reverse_iterator(nullptr, 0);
       }

protected:
       /** @brief Nodo raíz del árbol */
       BTNode m_Root;
       /** @brief Altura del árbol */
       size_t m_Height;
       /** @brief Orden del árbol B */
       size_t m_Order;
       /** @brief Número total de claves */
       size_t m_NumKeys;
       /** @brief Indica si las claves deben ser únicas */
       bool m_Unique;
       /** @brief Mutex para control de concurrencia */
       mutable std::shared_mutex m_Mutex;
};

/**
 * @brief Inserción de un elemento en el árbol B.
 *
 * @tparam Trait Características del árbol
 * @param key Clave a insertar
 * @param ObjID Identificador del objeto
 * @return true si la inserción fue exitosa
 */
template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const long ObjID)
{
       std::unique_lock<std::shared_mutex> lock(m_Mutex);

       bt_ErrorCode error = m_Root.Insert(key, ObjID);
       if (error == bt_duplicate)
              return false;
       m_NumKeys++;
       if (error == bt_overflow)
       {
              m_Root.SplitRoot();
              m_Height++;
       }
       return true;
}

/**
 * @brief Eliminación de un elemento del árbol B.
 *
 * @tparam Trait Características del árbol
 * @param key Clave a eliminar
 * @param ObjID Identificador del objeto
 * @return true si la eliminación fue exitosa
 */
template <typename Trait>
bool BTree<Trait>::Remove(const keyType key, const long ObjID)
{
       std::unique_lock<std::shared_mutex> lock(m_Mutex);

       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if (error == bt_duplicate || error == bt_nofound)
              return false;
       m_NumKeys--;

       if (error == bt_rootmerged)
              m_Height--;
       return true;
}

/**
 * @brief Escribe la estructura del árbol en un stream.
 *
 * @tparam Trait Características del árbol
 * @param os Stream de salida
 * @return Referencia al stream
 */
template <typename Trait>
std::ostream &BTree<Trait>::Write(std::ostream &os) const
{
       std::shared_lock<std::shared_mutex> lock(m_Mutex);

       os << m_Order << " " << m_Height << " " << m_NumKeys << " " << m_Unique << "\n";

       const_cast<BTNode &>(m_Root).WriteStructure(os);

       return os;
}

/**
 * @brief Lee la estructura del árbol desde un stream.
 *
 * @tparam Trait Características del árbol
 * @param is Stream de entrada
 * @return Referencia al stream
 */
template <typename Trait>
std::istream &BTree<Trait>::Read(std::istream &is)
{
       std::unique_lock<std::shared_mutex> lock(m_Mutex);

       is >> m_Order >> m_Height >> m_NumKeys >> m_Unique;

       m_Root.Reset();
       m_Root = BTNode(2 * m_Order + 1, m_Unique);
       m_Root.SetMaxKeysForChilds(m_Order);

       m_Root.ReadStructure(is);

       return is;
}

/**
 * @brief Incremento del iterador directo.
 *
 * @tparam Trait Características del árbol
 * @return Referencia al iterador
 */
template <typename Trait>
BTreeIterator<Trait> &BTreeIterator<Trait>::operator++()
{
       if (!m_CurrentPage)
              return *this;

       BTPage *rightChild = m_CurrentPage->m_SubPages[m_CurrentIndex + 1];
       if (rightChild != nullptr)
       {
              BTPage *leftmost = rightChild;
              while (leftmost->m_SubPages[0] != nullptr)
              {
                     leftmost = leftmost->m_SubPages[0];
              }
              m_CurrentPage = leftmost;
              m_CurrentIndex = 0;
              return *this;
       }

       if (m_CurrentIndex + 1 < m_CurrentPage->m_KeyCount)
       {
              m_CurrentIndex++;
              return *this;
       }

       BTPage *child = m_CurrentPage;
       BTPage *parent = m_CurrentPage->m_Parent;

       while (parent != nullptr)
       {
              size_t childPos = 0;
              while (childPos <= parent->m_KeyCount && parent->m_SubPages[childPos] != child)
              {
                     childPos++;
              }

              if (childPos < parent->m_KeyCount)
              {
                     m_CurrentPage = parent;
                     m_CurrentIndex = childPos;
                     return *this;
              }

              child = parent;
              parent = parent->m_Parent;
       }

       m_CurrentPage = nullptr;
       m_CurrentIndex = 0;
       return *this;
}

/**
 * @brief Incremento del iterador reverso.

 * @tparam Trait Características del árbol
 * @return Referencia al iterador
 */
template <typename Trait>
BTreeReverseIterator<Trait> &BTreeReverseIterator<Trait>::operator++()
{
       if (!m_CurrentPage)
              return *this;

       BTPage *leftChild = m_CurrentPage->m_SubPages[m_CurrentIndex];
       if (leftChild != nullptr)
       {
              BTPage *rightmost = leftChild;
              while (rightmost->m_SubPages[rightmost->m_KeyCount] != nullptr)
              {
                     rightmost = rightmost->m_SubPages[rightmost->m_KeyCount];
              }
              m_CurrentPage = rightmost;
              m_CurrentIndex = rightmost->m_KeyCount - 1;
              return *this;
       }

       if (m_CurrentIndex > 0)
       {
              m_CurrentIndex--;
              return *this;
       }

       BTPage *child = m_CurrentPage;
       BTPage *parent = m_CurrentPage->m_Parent;

       while (parent != nullptr)
       {
              size_t childPos = 0;
              while (childPos <= parent->m_KeyCount && parent->m_SubPages[childPos] != child)
              {
                     childPos++;
              }

              if (childPos > 0)
              {
                     m_CurrentPage = parent;
                     m_CurrentIndex = childPos - 1;
                     return *this;
              }

              child = parent;
              parent = parent->m_Parent;
       }

       m_CurrentPage = nullptr;
       m_CurrentIndex = 0;
       return *this;
}

#endif