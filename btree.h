#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <mutex>
#include <fstream>
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

/**
 * @brief Trait para configurar el BTree con tipos y comparador
 * @tparam _keyType Tipo de la clave
 * @tparam _ObjIDType Tipo del identificador de objeto
 * @tparam _Compare Función de comparación (por defecto std::less)
 */
template <typename _keyType, typename _ObjIDType, typename _Compare = std::less<_keyType>>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       using Compare = _Compare;
       
       // TODO: agregar funcion de comparacion - IMPLEMENTADO
       static Compare compare;
};

template <typename _keyType, typename _ObjIDType, typename _Compare>
_Compare BTreeTrait<_keyType, _ObjIDType, _Compare>::compare = _Compare();

/**
 * @brief Clase principal del árbol B
 * @tparam Trait Configuración del árbol (tipos y comparador)
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

       // Forward iterator
       class iterator;
       class const_iterator;
       class reverse_iterator;

public:
       /**
        * @brief Constructor del BTree
        * @param order Orden del árbol
        * @param unique Si permite claves duplicadas
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
        * @brief Destructor
        */
       ~BTree() {}
       
       /**
        * @brief Constructor de movimiento
        * @param other BTree a mover
        */
       BTree(BTree&& other) noexcept
              : m_Root(std::move(other.m_Root)),
                m_Height(other.m_Height),
                m_Order(other.m_Order),
                m_NumKeys(other.m_NumKeys),
                m_Unique(other.m_Unique)
       {
              other.m_Height = 1;
              other.m_NumKeys = 0;
       }
       
       /**
        * @brief Operador de asignación por movimiento
        */
       BTree& operator=(BTree&& other) noexcept
       {
              if (this != &other) {
                     m_Root = std::move(other.m_Root);
                     m_Height = other.m_Height;
                     m_Order = other.m_Order;
                     m_NumKeys = other.m_NumKeys;
                     m_Unique = other.m_Unique;
                     other.m_Height = 1;
                     other.m_NumKeys = 0;
              }
              return *this;
       }
       
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();
       
       /**
        * @brief Inserta un elemento en el árbol
        * @param key Clave a insertar
        * @param ObjID ID del objeto
        * @return true si se insertó correctamente
        */
       bool            Insert (const keyType key, const long ObjID);
       
       /**
        * @brief Elimina un elemento del árbol
        * @param key Clave a eliminar
        * @param ObjID ID del objeto
        * @return true si se eliminó correctamente
        */
       bool            Remove (const keyType key, const long ObjID);
       
       /**
        * @brief Busca una clave en el árbol
        * @param key Clave a buscar
        * @return ID del objeto encontrado o -1
        */
       ObjIDType       Search (const keyType key)
       {      
              std::lock_guard<std::mutex> lock(m_mutex);
              ObjIDType ObjID = -1;
              m_Root.Search(key, ObjID);
              return ObjID;
       }
       
       /**
        * @brief Obtiene el número de claves en el árbol
        */
       size_t            size()  { return m_NumKeys; }
       
       /**
        * @brief Obtiene la altura del árbol
        */
       size_t            height() { return m_Height;      }
       
       /**
        * @brief Obtiene el orden del árbol
        */
       size_t            GetOrder() { return m_Order;     }

       /**
        * @brief Imprime el árbol
        */
       void            Print (ostream &os)
       {               
              std::lock_guard<std::mutex> lock(m_mutex);
              m_Root.Print(os);                              
       }
       
       // TODO: #6 change by Invoke - Versión generalizada con templates
       /**
        * @brief Aplica una función a cada elemento (versión generalizada)
        * @tparam Func Tipo de función/lambda
        * @param func Función a aplicar
        */
       template<typename Func>
       void ForEach(Func func)
       {
              std::lock_guard<std::mutex> lock(m_mutex);
              m_Root.template ForEachGeneric<Func>(func, 0);
       }
       
       void            ForEach( lpfnForEach2 lpfn, void *pExtra1 )
       {               
              std::lock_guard<std::mutex> lock(m_mutex);
              m_Root.ForEach(lpfn, 0, pExtra1);              
       }
       
       void            ForEach( lpfnForEach3 lpfn, void *pExtra1, void *pExtra2)
       {               
              std::lock_guard<std::mutex> lock(m_mutex);
              m_Root.ForEach(lpfn, 0, pExtra1, pExtra2);     
       }
       
       // TODO: #8 You may reduce these two function by using Invoke - Versión generalizada
       /**
        * @brief Busca el primer elemento que cumple una condición (versión generalizada)
        * @tparam Func Tipo de función/lambda
        * @param func Función predicado
        * @return Puntero al elemento encontrado o nullptr
        */
       template<typename Func>
       ObjectInfo* FirstThat(Func func)
       {
              std::lock_guard<std::mutex> lock(m_mutex);
              return m_Root.template FirstThatGeneric<Func>(func, 0);
       }
       
       ObjectInfo*     FirstThat( lpfnFirstThat2 lpfn, void *pExtra1 )
       {               
              std::lock_guard<std::mutex> lock(m_mutex);
              return m_Root.FirstThat(lpfn, 0, pExtra1);     
       }
       
       ObjectInfo*     FirstThat( lpfnFirstThat3 lpfn, void *pExtra1, void *pExtra2)
       {               
              std::lock_guard<std::mutex> lock(m_mutex);
              return m_Root.FirstThat(lpfn, 0, pExtra1, pExtra2);   
       }
       
       /**
        * @brief Escribe el árbol en un archivo
        * @param filename Nombre del archivo
        * @return true si se escribió correctamente
        */
       bool Write(const std::string& filename)
       {
              std::lock_guard<std::mutex> lock(m_mutex);
              std::ofstream ofs(filename, std::ios::binary);
              if (!ofs) return false;
              
              ofs.write(reinterpret_cast<const char*>(&m_Height), sizeof(m_Height));
              ofs.write(reinterpret_cast<const char*>(&m_Order), sizeof(m_Order));
              ofs.write(reinterpret_cast<const char*>(&m_NumKeys), sizeof(m_NumKeys));
              ofs.write(reinterpret_cast<const char*>(&m_Unique), sizeof(m_Unique));
              
              m_Root.Write(ofs);
              return ofs.good();
       }
       
       /**
        * @brief Lee el árbol desde un archivo
        * @param filename Nombre del archivo
        * @return true si se leyó correctamente
        */
       bool Read(const std::string& filename)
       {
              std::lock_guard<std::mutex> lock(m_mutex);
              std::ifstream ifs(filename, std::ios::binary);
              if (!ifs) return false;
              
              ifs.read(reinterpret_cast<char*>(&m_Height), sizeof(m_Height));
              ifs.read(reinterpret_cast<char*>(&m_Order), sizeof(m_Order));
              ifs.read(reinterpret_cast<char*>(&m_NumKeys), sizeof(m_NumKeys));
              ifs.read(reinterpret_cast<char*>(&m_Unique), sizeof(m_Unique));
              
              m_Root.Read(ifs, m_Order);
              return ifs.good();
       }
       
       /**
        * @brief Operador de salida
        */
       friend std::ostream& operator<<(std::ostream& os, BTree& tree)
       {
              tree.Print(os);
              return os;
       }
       
       // Iteradores
       iterator begin();
       iterator end();
       const_iterator begin() const;
       const_iterator end() const;
       reverse_iterator rbegin();
       reverse_iterator rend();

       //typedef               ObjectInfo iterator;

protected:
       BTNode          m_Root;
       size_t          m_Height;  // height of tree
       size_t          m_Order;   // order of tree
       size_t          m_NumKeys; // number of keys
       bool            m_Unique;  // Accept the elements only once ?
       mutable std::mutex m_mutex; // Mutex para concurrencia
};     

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const long ObjID){
       std::lock_guard<std::mutex> lock(m_mutex);
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
       std::lock_guard<std::mutex> lock(m_mutex);
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if( error == bt_duplicate || error == bt_nofound )
               return false;
       m_NumKeys--;

       if( error == bt_rootmerged )
               m_Height--;
       return true;
}

// TODO: #3 crear un iterator ( no trivial )
//       Sugerencia: Tarea1 cada pagina debe tener un puntero al padre primero ( no trivial )
/**
 * @brief Forward iterator para recorrer el BTree
 */
template <typename Trait>
class BTree<Trait>::iterator
{
       typedef typename BTree<Trait>::ObjectInfo ObjectInfo;
       typedef typename BTree<Trait>::BTNode BTNode;
       
       struct NodePosition {
              BTNode* node;
              size_t index;
              NodePosition(BTNode* n = nullptr, size_t i = 0) : node(n), index(i) {}
       };
       
       std::vector<NodePosition> m_stack;
       ObjectInfo* m_current;
       
       void findNext()
       {
              if (m_stack.empty()) {
                     m_current = nullptr;
                     return;
              }
              
              NodePosition& pos = m_stack.back();
              
              if (pos.index < pos.node->m_KeyCount) {
                     m_current = &pos.node->m_Keys[pos.index];
                     
                     if (pos.node->m_SubPages[pos.index + 1]) {
                            m_stack.push_back(NodePosition(pos.node->m_SubPages[pos.index + 1], 0));
                            pos.index++;
                            findLeftmost();
                     } else {
                            pos.index++;
                     }
              } else {
                     m_stack.pop_back();
                     findNext();
              }
       }
       
       void findLeftmost()
       {
              while (!m_stack.empty()) {
                     NodePosition& pos = m_stack.back();
                     if (pos.node->m_SubPages[0]) {
                            m_stack.push_back(NodePosition(pos.node->m_SubPages[0], 0));
                     } else {
                            break;
                     }
              }
       }
       
public:
       iterator(BTNode* root = nullptr) : m_current(nullptr)
       {
              if (root && root->m_KeyCount > 0) {
                     m_stack.push_back(NodePosition(root, 0));
                     findLeftmost();
                     findNext();
              }
       }
       
       ObjectInfo& operator*() { return *m_current; }
       ObjectInfo* operator->() { return m_current; }
       
       iterator& operator++()
       {
              findNext();
              return *this;
       }
       
       iterator operator++(int)
       {
              iterator tmp = *this;
              ++(*this);
              return tmp;
       }
       
       bool operator==(const iterator& other) const
       {
              return m_current == other.m_current;
       }
       
       bool operator!=(const iterator& other) const
       {
              return !(*this == other);
       }
};

/**
 * @brief Const iterator para recorrer el BTree
 */
template <typename Trait>
class BTree<Trait>::const_iterator
{
       typename BTree<Trait>::iterator m_iter;
       
public:
       const_iterator(typename BTree<Trait>::BTNode* root = nullptr) : m_iter(root) {}
       
       const typename BTree<Trait>::ObjectInfo& operator*() const { return *m_iter; }
       const typename BTree<Trait>::ObjectInfo* operator->() const { return m_iter.operator->(); }
       
       const_iterator& operator++() { ++m_iter; return *this; }
       const_iterator operator++(int) { const_iterator tmp = *this; ++m_iter; return tmp; }
       
       bool operator==(const const_iterator& other) const { return m_iter == other.m_iter; }
       bool operator!=(const const_iterator& other) const { return m_iter != other.m_iter; }
};

/**
 * @brief Reverse iterator para recorrer el BTree en orden inverso
 */
template <typename Trait>
class BTree<Trait>::reverse_iterator
{
       typedef typename BTree<Trait>::ObjectInfo ObjectInfo;
       typedef typename BTree<Trait>::BTNode BTNode;
       
       struct NodePosition {
              BTNode* node;
              int index;
              NodePosition(BTNode* n = nullptr, int i = -1) : node(n), index(i) {}
       };
       
       std::vector<NodePosition> m_stack;
       ObjectInfo* m_current;
       
       void findPrev()
       {
              if (m_stack.empty()) {
                     m_current = nullptr;
                     return;
              }
              
              NodePosition& pos = m_stack.back();
              
              if (pos.index >= 0) {
                     m_current = &pos.node->m_Keys[pos.index];
                     
                     if (pos.node->m_SubPages[pos.index]) {
                            m_stack.push_back(NodePosition(pos.node->m_SubPages[pos.index], 
                                                          pos.node->m_SubPages[pos.index]->m_KeyCount - 1));
                            pos.index--;
                            findRightmost();
                     } else {
                            pos.index--;
                     }
              } else {
                     m_stack.pop_back();
                     findPrev();
              }
       }
       
       void findRightmost()
       {
              while (!m_stack.empty()) {
                     NodePosition& pos = m_stack.back();
                     if (pos.node->m_SubPages[pos.node->m_KeyCount]) {
                            BTNode* rightmost = pos.node->m_SubPages[pos.node->m_KeyCount];
                            m_stack.push_back(NodePosition(rightmost, rightmost->m_KeyCount - 1));
                     } else {
                            break;
                     }
              }
       }
       
public:
       reverse_iterator(BTNode* root = nullptr) : m_current(nullptr)
       {
              if (root && root->m_KeyCount > 0) {
                     m_stack.push_back(NodePosition(root, root->m_KeyCount - 1));
                     findRightmost();
                     findPrev();
              }
       }
       
       ObjectInfo& operator*() { return *m_current; }
       ObjectInfo* operator->() { return m_current; }
       
       reverse_iterator& operator++()
       {
              findPrev();
              return *this;
       }
       
       reverse_iterator operator++(int)
       {
              reverse_iterator tmp = *this;
              ++(*this);
              return tmp;
       }
       
       bool operator==(const reverse_iterator& other) const
       {
              return m_current == other.m_current;
       }
       
       bool operator!=(const reverse_iterator& other) const
       {
              return !(*this == other);
       }
};

// Implementación de begin/end
template <typename Trait>
typename BTree<Trait>::iterator BTree<Trait>::begin()
{
       return iterator(&m_Root);
}

template <typename Trait>
typename BTree<Trait>::iterator BTree<Trait>::end()
{
       return iterator();
}

template <typename Trait>
typename BTree<Trait>::const_iterator BTree<Trait>::begin() const
{
       return const_iterator(&m_Root);
}

template <typename Trait>
typename BTree<Trait>::const_iterator BTree<Trait>::end() const
{
       return const_iterator();
}

template <typename Trait>
typename BTree<Trait>::reverse_iterator BTree<Trait>::rbegin()
{
       return reverse_iterator(&m_Root);
}

template <typename Trait>
typename BTree<Trait>::reverse_iterator BTree<Trait>::rend()
{
       return reverse_iterator();
}

#endif