/**
 * @file btree.h
 * @brief Árbol B con concurrencia thread-safe e iteradores bidireccionales
 */

#ifndef __BTREE_H__
#define __BTREE_H__

#include <iostream>
#include <iterator>
#include <utility>  // Para std::exchange
#include <shared_mutex>  // Permite múltiples lectores o un solo escritor
#include <mutex>         // Para usar shared_lock y unique_lock
#include "btreepage.h"
#define DEFAULT_BTREE_ORDER 3

const size_t MaxHeight = 5; 

/**
 * @brief Trait para configurar tipos del BTree
 * @tparam _keyType Tipo de clave
 * @tparam _ObjIDType Tipo de identificador
 * @tparam _Compare Función de comparación
 */
template <typename _keyType, typename _ObjIDType, typename _Compare = std::less<_keyType>>
struct BTreeTrait
{
       using keyType = _keyType;
       using ObjIDType = _ObjIDType;
       using Compare = _Compare;
};

/**
 * @brief Árbol B con concurrencia thread-safe
 * @tparam Trait Configuración de tipos (BTreeTrait)
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
    /**
     * @brief Forward iterator (recorre de menor a mayor)
     * 
     * En este iterador se usa operator++ para avanzar
     */
    class iterator {
        friend class BTree;
    private:
        BTNode* m_Node;
        size_t m_Index;
        BTree* m_Tree;
        
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = ObjectInfo;
        using difference_type = std::ptrdiff_t;
        using pointer = ObjectInfo*;
        using reference = ObjectInfo&;
        
        iterator(BTNode* node = nullptr, size_t index = 0, BTree* tree = nullptr) 
            : m_Node(node), m_Index(index), m_Tree(tree) {}
        
        reference operator*() { return m_Node->m_Keys[m_Index]; }
        pointer operator->() { return &m_Node->m_Keys[m_Index]; }
        
        // Operator++ para forward
        iterator& operator++() {
            if(!m_Node) return *this;
            
            // Si hay hijo derecho, ir al más izquierdo del hijo derecho
            if(m_Node->m_SubPages[m_Index + 1]) {
                m_Node = m_Node->m_SubPages[m_Index + 1];
                while(m_Node->m_SubPages[0]) {
                    m_Node = m_Node->m_SubPages[0];
                }
                m_Index = 0;
            }
            // Si no hay hijo derecho, siguiente key en el mismo nodo
            else if(m_Index + 1 < m_Node->m_KeyCount) {
                ++m_Index;
            }
            // Subir al padre
            else {
                BTNode* child = m_Node;
                m_Node = m_Node->m_Parent;
                
                while(m_Node) {
                    for(size_t i = 0; i <= m_Node->m_KeyCount; ++i) {
                        if(m_Node->m_SubPages[i] == child) {
                            if(i < m_Node->m_KeyCount) {
                                m_Index = i;
                                return *this;
                            }
                            break;
                        }
                    }
                    child = m_Node;
                    m_Node = m_Node->m_Parent;
                }
                m_Node = nullptr;
            }
            return *this;
        }
        
        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }
        
        bool operator==(const iterator& other) const {
            return m_Node == other.m_Node && 
                (m_Node == nullptr || m_Index == other.m_Index);
        }
        
        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }
    };

    /**
     * @brief Backward iterator/ reverse iterator (recorre de mayor a menor)
     * 
     * Se usa operator++ pero va hacia ATRÁS
     */
    class reverse_iterator {
        friend class BTree;
    private:
        BTNode* m_Node;
        size_t m_Index;
        BTree* m_Tree;
        
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = ObjectInfo;
        using difference_type = std::ptrdiff_t;
        using pointer = ObjectInfo*;
        using reference = ObjectInfo&;
        
        reverse_iterator(BTNode* node = nullptr, size_t index = 0, BTree* tree = nullptr) 
            : m_Node(node), m_Index(index), m_Tree(tree) {}
        
        reference operator*() { return m_Node->m_Keys[m_Index]; }
        pointer operator->() { return &m_Node->m_Keys[m_Index]; }
        
        // operator++ pero va HACIA ATRÁS (de mayor a menor)
        reverse_iterator& operator++() {
            if(!m_Node) return *this;
            
            // Si hay hijo izquierdo, ir al más derecho del hijo izquierdo
            if(m_Node->m_SubPages[m_Index]) {
                m_Node = m_Node->m_SubPages[m_Index];
                while(m_Node->m_SubPages[m_Node->m_KeyCount]) {
                    m_Node = m_Node->m_SubPages[m_Node->m_KeyCount];
                }
                m_Index = m_Node->m_KeyCount - 1;
            }
            // Si no hay hijo izquierdo, key anterior en el mismo nodo
            else if(m_Index > 0) {
                --m_Index;
            }
            // Subir al padre
            else {
                BTNode* child = m_Node;
                m_Node = m_Node->m_Parent;
                
                while(m_Node) {
                    for(size_t i = 0; i <= m_Node->m_KeyCount; ++i) {
                        if(m_Node->m_SubPages[i] == child) {
                            if(i > 0) {
                                m_Index = i - 1;
                                return *this;
                            }
                            break;
                        }
                    }
                    child = m_Node;
                    m_Node = m_Node->m_Parent;
                }
                m_Node = nullptr;
            }
            return *this;
        }
        
        reverse_iterator operator++(int) {
            reverse_iterator temp = *this;
            ++(*this);
            return temp;
        }
        
        bool operator==(const reverse_iterator& other) const {
            return m_Node == other.m_Node && 
                (m_Node == nullptr || m_Index == other.m_Index);
        }
        
        bool operator!=(const reverse_iterator& other) const {
            return !(*this == other);
        }
    };

    // Métodos begin/end
    iterator begin() {
        std::shared_lock<std::shared_mutex> lock(m_Mutex);
        BTNode* node = &m_Root;
        while(node->m_SubPages[0]) {
            node = node->m_SubPages[0];
        }
        return iterator(node, 0, this);
    }

    iterator end() {
        return iterator(nullptr, 0, this);
    }

    // Métodos rbegin/rend
    reverse_iterator rbegin() {
        std::shared_lock<std::shared_mutex> lock(m_Mutex);
        BTNode* node = &m_Root;
        while(node->m_SubPages[node->m_KeyCount]) {
            node = node->m_SubPages[node->m_KeyCount];
        }
        return reverse_iterator(node, node->m_KeyCount > 0 ? node->m_KeyCount - 1 : 0, this);
    }

    reverse_iterator rend() {
        return reverse_iterator(nullptr, 0, this);
    }

public:
       /** @brief Constructor del BTree
        *  @param order Orden del árbol (número de claves por nodo)
        *  @param unique Si es true, no permite claves duplicadas */
       BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
              : m_Order(order),
                m_Root(2 * order  + 1, unique),
                m_Unique(unique),
                m_NumKeys(0)
       {
              m_Root.SetMaxKeysForChilds(order);
              m_Height = 1;
       }
       
       /** @brief Move constructor (transfiere recursos sin copiar) */
        BTree(BTree&& other) noexcept
            : m_Mutex() {  // Inicializamos nuestro mutex
            std::unique_lock lock(other.m_Mutex);  // Se bloquea el mutex de other
            
            m_Order = std::exchange(other.m_Order, DEFAULT_BTREE_ORDER);
            m_Root = std::move(other.m_Root);
            m_Height = std::exchange(other.m_Height, 1);
            m_NumKeys = std::exchange(other.m_NumKeys, 0);
            m_Unique = std::exchange(other.m_Unique, true);
        }

        /** @brief Move assignment operator */
        BTree& operator=(BTree&& other) noexcept {
            if (this != &other) {
                std::unique_lock lock1(m_Mutex, std::defer_lock);
                std::unique_lock lock2(other.m_Mutex, std::defer_lock);
                std::lock(lock1, lock2);  // Evita deadlock
                
                m_Root = std::move(other.m_Root);
                m_Order = std::exchange(other.m_Order, DEFAULT_BTREE_ORDER);
                m_Height = std::exchange(other.m_Height, 1);
                m_NumKeys = std::exchange(other.m_NumKeys, 0);
                m_Unique = std::exchange(other.m_Unique, true);
            }
            return *this;
        }
       
       /** @brief Destructor del BTree */
       ~BTree() {}
       
       /** @brief Inserta una clave y su ObjID en el árbol
        *  @param key Clave a insertar
        *  @param ObjID ID del objeto asociado
        *  @return true si se insertó correctamente, false si ya existía (en modo unique) */
       bool            Insert (const keyType key, const long ObjID);
       
       /** @brief Remueve una clave del árbol
        *  @param key Clave a remover
        *  @param ObjID ID del objeto asociado
        *  @return true si se removió correctamente, false si no se encontró */
       bool            Remove (const keyType key, const long ObjID);
       
       /** @brief Busca una clave en el árbol
        *  @param key Clave a buscar
        *  @return ObjID asociado a la clave, o -1 si no se encuentra */
       ObjIDType       Search (const keyType key)
       {      
              std::shared_lock<std::shared_mutex> lock(m_Mutex);  // Lock compartido para lectura
              ObjID = -1;
              m_Root.Search(key, ObjID);
              return ObjID;
       }
       
       /** @brief Retorna el número total de claves en el árbol */
       size_t            size()  { 
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              return m_NumKeys; 
       }
       
       /** @brief Retorna la altura del árbol */
       size_t            height() { 
              std::shared_lock<std::shared_mutex> lock(m_Mutex);
              return m_Height;      
       }
       
       /** @brief Retorna el orden del árbol (número de claves por nodo) */
       size_t            GetOrder() { return m_Order;     }

       /** @brief Imprime el árbol en formato visual
        *  @param os Stream de salida */
       void            Print (ostream &os)
       {               
              std::shared_lock<std::shared_mutex> lock(m_Mutex);  // Lock compartido para lectura
              m_Root.Print(os);                              
       }
       
       /** @brief Guarda el árbol en formato texto
        *  @param os Stream de salida */
       void Write(std::ostream& os) {
              std::shared_lock<std::shared_mutex> lock(m_Mutex);  // Lock compartido para lectura
              os << m_Order << " " << m_Height << " " 
                 << m_NumKeys << " " << m_Unique << "\n";
              m_Root.Write(os);
       }
       
       /** @brief Carga el árbol desde formato texto
        *  @param is Stream de entrada */
       void Read(std::istream& is) {
              std::unique_lock<std::shared_mutex> lock(m_Mutex);  // Lock exclusivo para escritura
              is >> m_Order >> m_Height >> m_NumKeys >> m_Unique;
              m_Root.Read(is);
       }
       
       /** @brief Aplica una función a cada elemento del árbol (con variadic templates)
        *  @param func Función a aplicar
        *  @param args Argumentos adicionales para la función */
       template<typename Func, typename... Args>
       void ForEach(Func func, Args&&... args) { 
              std::shared_lock<std::shared_mutex> lock(m_Mutex);  // Lock compartido para lectura
              m_Root.ForEach(func, std::forward<Args>(args)...); 
       }

       /** @brief Encuentra el primer elemento que cumple un predicado (con variadic templates)
        *  @param predicate Predicado a evaluar
        *  @param args Argumentos adicionales para el predicado
        *  @return Puntero al ObjectInfo encontrado, o nullptr si no se encuentra */
       template<typename Pred, typename... Args>
       ObjectInfo* FirstThat(Pred predicate, Args&&... args) { 
              std::shared_lock<std::shared_mutex> lock(m_Mutex);  // Lock compartido para lectura
              return m_Root.FirstThat(predicate, std::forward<Args>(args)...); 
       }

protected:
       BTNode          m_Root;     ///< Nodo raíz del árbol
       size_t          m_Height;   ///< Altura del árbol
       size_t          m_Order;    ///< Orden del árbol (número de claves por nodo)
       size_t          m_NumKeys;  ///< Número total de claves en el árbol
       bool            m_Unique;   ///< Si es true, no permite claves duplicadas
       mutable std::shared_mutex m_Mutex;  ///< Mutex para concurrencia (múltiples lectores o un escritor)
};     

template <typename Trait>
bool BTree<Trait>::Insert(const keyType key, const long ObjID){
       std::unique_lock<std::shared_mutex> lock(m_Mutex);  // Lock exclusivo para escritura
       
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
       std::unique_lock<std::shared_mutex> lock(m_Mutex);  // Lock exclusivo para escritura
       
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if( error == bt_duplicate || error == bt_nofound )
               return false;
       m_NumKeys--;

       if( error == bt_rootmerged )
               m_Height--;
       return true;
}

/** @brief Operador de salida para imprimir el árbol
 *  @param os Stream de salida
 *  @param tree Árbol a imprimir
 *  @return Referencia al stream de salida */
template <typename Trait>
std::ostream& operator<<(std::ostream& os, BTree<Trait>& tree) {
       tree.Print(os);
       return os;
}

#endif