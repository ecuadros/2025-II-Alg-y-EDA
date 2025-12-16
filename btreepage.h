#ifndef __CBTreePage_H__
#define __CBTreePage_H__

/**
 * @file btreepage.h
 * @brief Define la clase CBTreePage, que representa un único nodo (página) en el B-Tree.
 */

#include <vector>
#include <assert.h>
#include <functional>

// TODO: #1 Crear una function para agregarla al demo.cpp ( no trivial ) (DONE)
// TODO: #2 Agregarle un Trait (prueba git) ( no trivial ) (DONE)
// TODO: #3 crear un iterator ( no trivial ) (DONE)
//       Sugerencia: Tarea1 cada pagina debe tener un puntero al padre primero ( no trivial ) (DONE)
// TODO: #4 integrarlo al recorrer ( no trivial ) (DONE)


template <typename Trait> // Declaración anticipada para la clase principal del árbol.
class RTree;

using namespace std;
/// Códigos de error para las operaciones del B-Tree.
enum bt_ErrorCode {bt_ok, bt_overflow, bt_underflow, bt_duplicate, bt_nofound, bt_rootmerged};


// Error al poner size_t
// Posible motivo: El i está disminuyendo
/**
 * @brief Inserta un objeto en un contenedor en una posición específica.
 * @tparam Container El tipo del contenedor.
 * @tparam ObjType El tipo del objeto a insertar.
 * @param container El contenedor.
 * @param object El objeto a insertar.
 * @param pos La posición en la que se insertará.
 */
template <typename Container, typename ObjType>
void insert_at(Container& container, ObjType object, size_t pos)
{
        // TODO: #5 replace int, long by types such as size_t (DONE)
        size_t i = container.size() - 1;
        for (i ; i > pos ; i--) 
               container[i] = container[i-1];
        
        container[pos] =  object;	
}

/**
 * @brief Elimina un elemento de un contenedor en una posición específica.
 * @tparam Container El tipo del contenedor.
 * @param container El contenedor.
 * @param pos La posición del elemento a eliminar.
 */
template <typename Container>
void remove(Container& container, size_t pos)
{
       size_t size = container.size();
       for(size_t i = pos + 1 ; i < size ; i++)
           container[i-1] = container[i];
}

/**
 * @struct tagObjectInfo
 * @brief Almacena un par clave-valor y un contador de uso.
 * @tparam keyType El tipo de la clave.
 * @tparam ObjIDType El tipo del valor (ID de objeto).
 */
template <typename keyType, typename ObjIDType>
struct tagObjectInfo
{
       keyType                 key;         ///< La clave Rect, en nodos internos es el MBR del hijo.
       ObjIDType               ObjID;       ///< En nodos hoja, es el ID del objeto. En nodos internos no se usa.
       size_t                    UseCounter;  ///< Contador para rastrear el uso.
       tagObjectInfo(const keyType     &_key, ObjIDType _ObjID)
               : key(_key), ObjID(_ObjID), UseCounter(0) {}
       tagObjectInfo(const tagObjectInfo &objInfo)
               : key(objInfo.key), ObjID(objInfo.ObjID), UseCounter(0) {}
       tagObjectInfo()                          {}
       operator keyType                         ()     { return key; }
       size_t                    GetUseCounter() { return UseCounter;    }
};

/**
 * @brief Función de ayuda para imprimir un ObjectInfo.
 * @param info El ObjectInfo a imprimir.
 * @param level El nivel de profundidad en el árbol.
 * @param pExtra Puntero genérico, usado aquí para pasar el ostream.
 */
template <typename keyType, typename ObjIDType>
void PrintObjectInfoHelper(const tagObjectInfo<keyType, ObjIDType> &info, size_t level, void *pExtra)
{
       ostream &os = *(ostream *)pExtra;
       for(size_t i = 0; i < level ; i++)
               os << "\t";
       os << info.key << "->" << info.ObjID << "\n";
}

/**
 * @class CBTreePage
 * @brief Representa un único nodo (página) dentro de un B-Tree.
 * @tparam Trait Un struct que define los tipos usados por el B-Tree.
 */
template <typename Trait>
class CBTreePage //: public SimpleIndex <keyType>
// this is the in-memory version of the CBTreePage
{
       friend class RTree<Trait>;
       typedef typename Trait::keyType  keyType;
       typedef typename Trait::ObjIDType  ObjIDType; 
       typedef typename Trait::Compare  Compare;

       typedef CBTreePage<Trait>    BTPage;         // useful shorthand
       typedef tagObjectInfo<keyType, ObjIDType> ObjectInfo;


 public:
       /**
        * @brief Constructor por movimiento.
        * @param other La página a mover.
        */
       CBTreePage& operator=(CBTreePage&& other) noexcept; // Move Assignment
       CBTreePage(CBTreePage&& other) noexcept; // Move Constructor
       /**
        * @brief Constructor de la página.
        * @param maxKeys Número máximo de claves que puede contener la página.
        * @param unique Verdadero si las claves deben ser únicas.
        */
       CBTreePage(size_t maxKeys, bool unique = true);
       /// @brief Destructor. Libera los recursos de la página.
       virtual ~CBTreePage();

       /**
        * @brief Inserta un par clave-valor en este nodo o en un nodo hijo.
        * @param key La clave a insertar.
        * @param ObjID El valor a asociar con la clave.
        * @return Un código de error que indica el resultado (ej. bt_ok, bt_overflow).
        */
       bt_ErrorCode    Insert (const keyType &key, const ObjIDType ObjID, BTPage** ppNewNode);

       bool Remove(const keyType& key, const ObjIDType ObjID, std::vector<ObjectInfo>& reinsert_list);

       /**
        * @brief Busca una clave en este nodo o en sus hijos.
        * @param key La clave a buscar.
        * @param ObjID Parámetro de salida para el ID de objeto encontrado.
        * @return Verdadero si se encontró la clave, falso en caso contrario.
        */
       void            Search (const keyType &area, vector<ObjIDType>& resultados);

       /// Imprime el contenido de este nodo y sus hijos.
       void            Print  (ostream &os) const;
       /// Escribe los datos del nodo en un flujo para serialización.
       void            Write(ostream& os) const;
       /// Lee los datos del nodo desde un flujo para deserialización.
       void            Read(istream& is);

       
       // TODO #6, #7, #8: Generalizado con plantillas variádicas y std::invoke (DONE)
       template<typename Func, typename... Args>
       void ForEach(size_t level, Func&& func, Args&&... args) const;

       template<typename Func, typename... Args>
       ObjectInfo* FirstThat(size_t level, Func&& func, Args&&... args) const;

protected:
       size_t  m_MinKeys;          ///< Número mínimo de claves en un nodo.
       size_t  m_MaxKeys;          ///< Número máximo de claves en un nodo.
       size_t  m_MaxKeysForChilds; ///< Máximo de claves para los nodos hijos.
       bool m_Unique;              ///< Verdadero si las claves deben ser únicas.
       bool m_isRoot;              ///< Verdadero si este nodo es la raíz.
       vector<ObjectInfo> m_Keys;  ///< Vector de entradas. En R-Tree, ObjectInfo.key es el MBR.
       vector<BTPage *>m_SubPages; ///< Vector de punteros a nodos hijos.
       BTPage* m_pParent = nullptr; ///< Puntero al nodo padre.
       Compare m_Compare;           ///< Objeto de función de comparación.
       
       size_t  m_KeyCount;          ///< Número actual de claves en el nodo.

       /// @brief Inicializa los vectores de claves y sub-páginas.
       void  Create();
       /// @brief Libera los recursos de las sub-páginas.
       void  Reset ();
       /// @brief Llama a Reset y elimina el objeto actual.
       void  Destroy () {   Reset(); delete this;}
       /// @brief Reinicia el contador de claves a cero.
       void  clear ();

       /// @brief Verifica si el nodo ha excedido su capacidad máxima de claves.
       bool Overflow()  { return m_KeyCount > m_MaxKeys; } // La condición sigue siendo válida
       /// @brief Verifica si el nodo tiene menos claves que el mínimo permitido.
       bool Underflow() { return m_KeyCount < MinNumberOfKeys(); }
       /// @brief Verifica si el nodo está lleno.
       bool IsFull()    { return m_KeyCount >= m_MaxKeys; }


       /// @brief Calcula el número mínimo de claves que un nodo debe tener.
       size_t  MinNumberOfKeys()  { return 2*m_MaxKeys/3.0; }
       /// @brief Devuelve el número de espacios libres para claves.
       size_t  GetFreeCells()  { return m_MaxKeys - m_KeyCount; }
       /// @brief Devuelve una referencia al contador de claves.
       size_t& NumberOfKeys()  { return m_KeyCount; }
       /// @brief Devuelve el número actual de claves.
       size_t  GetNumberOfKeys()  { return m_KeyCount; }
       /// @brief Verifica si este nodo es la raíz del árbol.
       bool IsRoot()  { return m_MaxKeysForChilds != m_MaxKeys; }
       /// @brief Establece el número máximo de claves para los nodos hijos.
       void SetMaxKeysForChilds(size_t orderforchilds)
       {        m_MaxKeysForChilds = orderforchilds;       }
       /// @brief Establece el puntero al nodo padre.
       void SetParent(BTPage* pParent) { m_pParent = pParent; }
       size_t GetFreeCellsOnLeft(size_t pos)
       {        if( pos > 0 )                                   // there is some page on left ?
                        return m_SubPages[pos-1]->GetFreeCells();
                return 0;
       }
       size_t GetFreeCellsOnRight(size_t pos)
       {    if( pos < GetNumberOfKeys() )   // there is some page on right ?
                return m_SubPages[pos+1]->GetFreeCells();
           return 0;
       }

private:
       void AdjustMBR(BTPage* child);
       void RemoveChild(BTPage* child, std::vector<ObjectInfo>& reinsert_list);
       void CollectAllLeafEntries(std::vector<ObjectInfo>& list);

       /// @brief Calcula el MBR que envuelve todas las entradas de este nodo.
       keyType CalculateMBR();
       /// @brief Añade un hijo a un nodo interno y actualiza su MBR.
       void AddChild(BTPage* pChild);
       /// @brief Algoritmo de división cuadrática para manejar el desbordamiento de un nodo.
       void QuadraticSplit(ObjectInfo& new_entry, BTPage* pNewNode);
       void QuadraticSplit(BTPage* new_child, BTPage* pNewNode);
       /// @brief Elige las dos "semillas" iniciales para el QuadraticSplit.
       void PickSeeds(std::vector<ObjectInfo>& entries, int& seed1, int& seed2);
       /// @brief Elige la siguiente entrada a asignar durante el QuadraticSplit.
       int PickNext(std::vector<ObjectInfo>& entries, keyType& mbr1, keyType& mbr2);
       /// @brief Proceso de condensación del árbol después de un borrado.
       void CondenseTree(std::vector<ObjectInfo>& reinsert_list);
};

template <typename Trait>
CBTreePage<Trait>:: CBTreePage(size_t maxKeys, bool unique)
                               : m_MaxKeys(maxKeys), m_Unique(unique), m_KeyCount(0)
{
       Create();
       SetMaxKeysForChilds(m_MaxKeys);
}

template <typename Trait>
CBTreePage<Trait>::~CBTreePage()
{
       Reset();
}

template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::Insert(const keyType& key, const ObjIDType ObjID, BTPage** ppNewNode) {
    bool is_leaf = (m_SubPages.empty() || m_SubPages[0] == nullptr);
    *ppNewNode = nullptr;

    if (is_leaf) {
        ObjectInfo new_entry(key, ObjID);
        if (!IsFull()) {
            m_Keys[m_KeyCount++] = new_entry;
            return bt_ok;
        } else {
            // el nodo hoja está lleno por eso se dividide.
            *ppNewNode = new BTPage(m_MaxKeys, m_Unique);
            QuadraticSplit(new_entry, *ppNewNode);
            return bt_overflow;
        }
    } else {
         // nodo interno
        int best_child_idx = -1;
        int min_expansion = -1;

        for (size_t i = 0; i < m_KeyCount; ++i) {
            int expansion = Rect::expansionNecesaria(m_Keys[i].key, key);
            if (best_child_idx == -1 || expansion < min_expansion) {
                min_expansion = expansion;
                best_child_idx = i;
            } else if (expansion == min_expansion) {
                // en empate se elege el mas pequeño
                if (m_Keys[i].key.area() < m_Keys[best_child_idx].key.area()) {
                    best_child_idx = i;
                }
            }
        }

        BTPage* pNewChildNode = nullptr;
        bt_ErrorCode error = m_SubPages[best_child_idx]->Insert(key, ObjID, &pNewChildNode);

        // ajuste del MBR del padre
        m_Keys[best_child_idx].key = Rect::unir(m_Keys[best_child_idx].key, key);

        if (error == bt_overflow) { 
            // se tiene que añadir pNewChildNode
            pNewChildNode->SetParent(this);
            if (!IsFull()) {
                AddChild(pNewChildNode);
                return bt_ok;
            } else {
                // el nodo está lleno por eso se divide
                *ppNewNode = new BTPage(m_MaxKeys, m_Unique);
                (*ppNewNode)->m_SubPages.resize(m_MaxKeys, nullptr);
                QuadraticSplit(pNewChildNode, *ppNewNode);
                return bt_overflow;
            }
        }
        return bt_ok;
    }
}

template <typename Trait>
void CBTreePage<Trait>::AddChild(BTPage* pChild) {
    m_Keys[m_KeyCount].key = pChild->CalculateMBR();
    m_Keys[m_KeyCount].ObjID = -1;
    m_SubPages[m_KeyCount] = pChild;
    m_KeyCount++;
}

template <typename Trait>
CBTreePage<Trait>& CBTreePage<Trait>::operator=(CBTreePage<Trait>&& other) noexcept {
    if (this != &other) {
        Reset(); 

        m_MinKeys = std::exchange(other.m_MinKeys, 0);
        m_MaxKeys = std::exchange(other.m_MaxKeys, 0);
        m_MaxKeysForChilds = std::exchange(other.m_MaxKeysForChilds, 0);
        m_Unique = std::exchange(other.m_Unique, false);
        m_isRoot = std::exchange(other.m_isRoot, false);
        m_Keys = std::move(other.m_Keys);
        m_SubPages = std::move(other.m_SubPages);
        m_pParent = std::exchange(other.m_pParent, nullptr);
        m_Compare = std::move(other.m_Compare);
        m_KeyCount = std::exchange(other.m_KeyCount, 0);

        // actualizacion de los punteros 
        for (size_t i = 0; i < m_KeyCount; ++i) {
            if (m_SubPages[i]) m_SubPages[i]->SetParent(this);
        }
    }
    return *this;
}
template <typename Trait>
CBTreePage<Trait>::CBTreePage(CBTreePage&& other) noexcept
    : m_MinKeys(std::exchange(other.m_MinKeys, 0)),
      m_MaxKeys(std::exchange(other.m_MaxKeys, 0)),
      m_MaxKeysForChilds(std::exchange(other.m_MaxKeysForChilds, 0)),
      m_Unique(std::exchange(other.m_Unique, false)),
      m_isRoot(std::exchange(other.m_isRoot, false)),
      m_Keys(std::move(other.m_Keys)),
      m_SubPages(std::move(other.m_SubPages)),
      m_Compare(std::move(other.m_Compare)),
      m_KeyCount(std::exchange(other.m_KeyCount, 0)),
      m_pParent(std::exchange(other.m_pParent, nullptr))
{
    // actualizacion de los punteros 
    for (size_t i = 0; i < m_KeyCount; ++i) {
        if (m_SubPages[i]) m_SubPages[i]->SetParent(this);
    }
}

template <typename Trait>
void CBTreePage<Trait>::Search(const keyType& area, vector<ObjIDType>& resultados)
{
    bool is_leaf = (m_SubPages.empty() || m_SubPages[0] == nullptr);

    if (is_leaf) {
        for (size_t i = 0; i < m_KeyCount; ++i) {
            if (m_Keys[i].key.intersecta(area)) {
                resultados.push_back(m_Keys[i].ObjID);
            }
        }
    } else {
        // nodo interno
        for (size_t i = 0; i < m_KeyCount; ++i) {
            // la key es el MBR del hijo
            if (m_Keys[i].key.intersecta(area)) {
                if(m_SubPages[i]) {
                    m_SubPages[i]->Search(area, resultados);
                }
            }
        }
    }
}

/*template <typename keyType, typename ObjIDType>
void CBTreePage<keyType, ObjIDType>::ForEachReverse(lpfnForEach2 lpfn, size_t level, void *pExtra1)
{
       if( m_SubPages[m_KeyCount] )
               m_SubPages[m_KeyCount]->ForEach(lpfn, level+1, pExtra1);
       for(size_t i = m_KeyCount-1 ; i >= 0  ; i--)
       {
               lpfn(m_Keys[i], level, pExtra1);
               if( m_SubPages[i] )
                       m_SubPages[i]->ForEach(lpfn, level+1, pExtra1);
       }
}*/

template <typename Trait>
template<typename Func, typename... Args>
void CBTreePage<Trait>::ForEach(size_t level, Func&& func, Args&&... args) const {
    for (size_t i = 0; i < m_KeyCount; i++) {
        if (m_SubPages[i]) {
            m_SubPages[i]->ForEach(level + 1, std::forward<Func>(func), std::forward<Args>(args)...);
        }
        std::invoke(func, m_Keys[i], level, std::forward<Args>(args)...);
    }
}

// Apicar una funcion hasta encontrar el 1er elemento
// aque que retorne true ante esta funcion
template <typename Trait>
template<typename Func, typename... Args>
typename CBTreePage<Trait>::ObjectInfo* CBTreePage<Trait>::FirstThat(size_t level, Func&& func, Args&&... args) const
{
    ObjectInfo* pTmp = nullptr;
    for (size_t i = 0; i < m_KeyCount; i++) {
        if (m_SubPages[i]) {
            if ((pTmp = m_SubPages[i]->FirstThat(level + 1, std::forward<Func>(func), std::forward<Args>(args)...))) {
                return pTmp;
            }
        }
        if (std::invoke(func, m_Keys[i], level, std::forward<Args>(args)...)) {
            return &m_Keys[i];
        }
    }
    return nullptr;
}

template <typename Trait>
bool CBTreePage<Trait>::Remove(const keyType& key, const ObjIDType ObjID, std::vector<ObjectInfo>& reinsert_list) {
    bool is_leaf = (m_SubPages.empty() || m_SubPages[0] == nullptr);

    if (is_leaf) {
        for (size_t i = 0; i < m_KeyCount; ++i) {
            // comparacion por ID y por rectángulos
            if (m_Keys[i].ObjID == ObjID && 
                m_Keys[i].key.x1 == key.x1 && m_Keys[i].key.y1 == key.y1 &&
                m_Keys[i].key.x2 == key.x2 && m_Keys[i].key.y2 == key.y2) 
            {
                ::remove(m_Keys, i);
                m_KeyCount--;
                CondenseTree(reinsert_list);
                return true;
            }
        }
        return false; 
    } else {
        for (size_t i = 0; i < m_KeyCount; ++i) {
            if (m_Keys[i].key.intersecta(key)) {
                if (m_SubPages[i]->Remove(key, ObjID, reinsert_list)) {
                    return true;
                }
            }
        }
        return false;
    }
}

template <typename Trait>
void CBTreePage<Trait>::CondenseTree(std::vector<ObjectInfo>& reinsert_list) {
    BTPage* current = this;
    while (current != nullptr && !current->IsRoot()) {
        BTPage* parent = current->m_pParent;
        if (current->Underflow()) {
            parent->RemoveChild(current, reinsert_list); 
        } else {
            // el nodo no tiene underflow
            parent->AdjustMBR(current);
        }
        current = parent;
    }
}

template <typename Trait>
void CBTreePage<Trait>::AdjustMBR(BTPage* child) {
    for (size_t i = 0; i < m_KeyCount; ++i) {
        if (m_SubPages[i] == child) {
            m_Keys[i].key = child->CalculateMBR();
            return;
        }
    }
}

template <typename Trait>
void CBTreePage<Trait>::CollectAllLeafEntries(std::vector<ObjectInfo>& list) {
    bool is_leaf = (m_SubPages.empty() || m_SubPages[0] == nullptr);
    if (is_leaf) {
        for (size_t i = 0; i < m_KeyCount; ++i) {
            list.push_back(m_Keys[i]);
        }
    } else {
        for (size_t i = 0; i < m_KeyCount; ++i) {
            if (m_SubPages[i]) {
                m_SubPages[i]->CollectAllLeafEntries(list);
            }
        }
    }
}

template <typename Trait>
void CBTreePage<Trait>::RemoveChild(BTPage* child, std::vector<ObjectInfo>& reinsert_list) {
    for (size_t i = 0; i < m_KeyCount; ++i) {
        if (m_SubPages[i] == child) {
            child->CollectAllLeafEntries(reinsert_list);
            
            // eliminar el puntero al hijo
            delete m_SubPages[i];
            ::remove(m_Keys, i);
            ::remove(m_SubPages, i);
            m_KeyCount--;
            return;
        }
    }
}

template <typename Trait>
void CBTreePage<Trait>::Create()
{
       Reset();
       m_Keys.resize(m_MaxKeys);
       m_SubPages.resize(m_MaxKeys, NULL);
       m_KeyCount = 0;
       m_MinKeys  = 2 * m_MaxKeys/3;
}

template <typename Trait>
void CBTreePage<Trait>::Reset()
{
        // TODO: #35 change int by size_t (DONE)
       for( size_t i = 0 ; i < m_KeyCount ; i++ )
               delete m_SubPages[i];
       clear();
}

template <typename Trait>
void CBTreePage<Trait>::clear()
{
       //m_Keys.clear();
       //m_SubPages.clear();
       m_KeyCount = 0;
}

/**
 * @brief Crea una nueva instancia de CBTreePage.
 * @return Un puntero a la nueva página.
 */
template <typename Trait>
CBTreePage<Trait> * CreateBTreeNode (size_t maxKeys, bool unique)
{
       return new CBTreePage<Trait> (maxKeys, unique);
}

/**
 * @brief Escribe el contenido de un arbol
 */
template <typename Trait>
void CBTreePage<Trait>::Write(ostream& os) const {
    bool is_leaf = (m_SubPages[0] == nullptr);
    os << m_KeyCount << " " << is_leaf << "\n";

    for (size_t i = 0; i < m_KeyCount; ++i) {
        os << m_Keys[i].key << " " << m_Keys[i].ObjID << " ";
    }
    os << "\n";

    if (!is_leaf) {
        for (size_t i = 0; i < m_KeyCount; ++i) {
            m_SubPages[i]->Write(os);
        }
    }
}
/**
 * @brief Lee el contenido de un arbol
 */
template <typename Trait>
void CBTreePage<Trait>::Read(istream& is) {
    bool is_leaf;
    is >> m_KeyCount >> is_leaf;

    for (size_t i = 0; i < m_KeyCount; ++i) {
        keyType k;
        ObjIDType o;
        is >> k >> o;
        m_Keys[i] = ObjectInfo(k, o);
    }

    if (!is_leaf) {
        m_SubPages.assign(m_MaxKeys, nullptr);
        for (size_t i = 0; i < m_KeyCount; ++i) {
            m_SubPages[i] = new BTPage(m_MaxKeysForChilds, m_Unique);
            m_SubPages[i]->SetParent(this);
            m_SubPages[i]->Read(is);
        }
    }
}

template <typename Trait>
typename Trait::keyType CBTreePage<Trait>::CalculateMBR() {
    if (m_KeyCount == 0) return {};
    keyType mbr = m_Keys[0].key;
    for (size_t i = 1; i < m_KeyCount; ++i) {
        mbr = Rect::unir(mbr, m_Keys[i].key);
    }
    return mbr;
}

template <typename Trait>
void CBTreePage<Trait>::QuadraticSplit(ObjectInfo& new_entry, BTPage* pNewNode) {
    std::vector<ObjectInfo> all_entries;
    all_entries.reserve(m_KeyCount + 1);
    for(size_t i = 0; i < m_KeyCount; ++i) all_entries.push_back(m_Keys[i]);
    all_entries.push_back(new_entry);

    int seed1, seed2;
    PickSeeds(all_entries, seed1, seed2);

    pNewNode->m_KeyCount = 0;
    this->m_KeyCount = 0;

    this->m_Keys[this->m_KeyCount++] = all_entries[seed1];
    pNewNode->m_Keys[pNewNode->m_KeyCount++] = all_entries[seed2];

    keyType mbr1 = all_entries[seed1].key;
    keyType mbr2 = all_entries[seed2].key;

    all_entries.erase(all_entries.begin() + std::max(seed1, seed2));
    all_entries.erase(all_entries.begin() + std::min(seed1, seed2));

    while (!all_entries.empty()) {
        if (this->m_KeyCount + all_entries.size() <= MinNumberOfKeys()) {
            for(const auto& entry : all_entries) this->m_Keys[this->m_KeyCount++] = entry;
            all_entries.clear();
            break;
        }
        if (pNewNode->m_KeyCount + all_entries.size() <= MinNumberOfKeys()) {
            for(const auto& entry : all_entries) pNewNode->m_Keys[pNewNode->m_KeyCount++] = entry;
            all_entries.clear();
            break;
        }

        int next_idx = PickNext(all_entries, mbr1, mbr2);
        ObjectInfo next_entry = all_entries[next_idx];

        int expansion1 = Rect::expansionNecesaria(mbr1, next_entry.key);
        int expansion2 = Rect::expansionNecesaria(mbr2, next_entry.key);

        if (expansion1 < expansion2) {
            this->m_Keys[this->m_KeyCount++] = next_entry;
            mbr1 = Rect::unir(mbr1, next_entry.key);
        } else if (expansion2 < expansion1) {
            pNewNode->m_Keys[pNewNode->m_KeyCount++] = next_entry;
            mbr2 = Rect::unir(mbr2, next_entry.key);
        } else { // Empate
            if (mbr1.area() < mbr2.area()) {
                this->m_Keys[this->m_KeyCount++] = next_entry;
                mbr1 = Rect::unir(mbr1, next_entry.key);
            } else {
                pNewNode->m_Keys[pNewNode->m_KeyCount++] = next_entry;
                mbr2 = Rect::unir(mbr2, next_entry.key);
            }
        }
        all_entries.erase(all_entries.begin() + next_idx);
    }
}

template <typename Trait>
void CBTreePage<Trait>::QuadraticSplit(BTPage* new_child, BTPage* pNewNode) {
    // división del nodo interno
    std::vector<BTPage*> all_children;
    all_children.reserve(m_KeyCount + 1);
    for(size_t i = 0; i < m_KeyCount; ++i) all_children.push_back(m_SubPages[i]);
    all_children.push_back(new_child);
    
    std::vector<ObjectInfo> all_entries;
    all_entries.reserve(m_KeyCount + 1);
    for(size_t i = 0; i < m_KeyCount; ++i) all_entries.push_back(m_Keys[i]);
    all_entries.push_back(ObjectInfo(new_child->CalculateMBR(), -1));

    int seed1, seed2;
    PickSeeds(all_entries, seed1, seed2);

    pNewNode->m_KeyCount = 0;
    this->m_KeyCount = 0;

    this->AddChild(all_children[seed1]);
    pNewNode->AddChild(all_children[seed2]);

    keyType mbr1 = all_entries[seed1].key;
    keyType mbr2 = all_entries[seed2].key;

    all_entries.erase(all_entries.begin() + std::max(seed1, seed2));
    all_children.erase(all_children.begin() + std::max(seed1, seed2));
    all_entries.erase(all_entries.begin() + std::min(seed1, seed2));
    all_children.erase(all_children.begin() + std::min(seed1, seed2));

    while (!all_entries.empty()) {
        if (this->m_KeyCount + all_entries.size() <= MinNumberOfKeys()) {
            for(auto child : all_children) this->AddChild(child);
            all_children.clear();
            break;
        }
        if (pNewNode->m_KeyCount + all_entries.size() <= MinNumberOfKeys()) {
            for(auto child : all_children) pNewNode->AddChild(child);
            all_children.clear();
            break;
        }

        int next_idx = PickNext(all_entries, mbr1, mbr2);
        BTPage* next_child = all_children[next_idx];

        int expansion1 = Rect::expansionNecesaria(mbr1, next_child->CalculateMBR());
        int expansion2 = Rect::expansionNecesaria(mbr2, next_child->CalculateMBR());

        if (expansion1 < expansion2) {
            this->AddChild(next_child);
            mbr1 = Rect::unir(mbr1, next_child->CalculateMBR());
        } else if (expansion2 < expansion1) {
            pNewNode->AddChild(next_child);
            mbr2 = Rect::unir(mbr2, next_child->CalculateMBR());
        } else { // Empate
            if (mbr1.area() < mbr2.area()) {
                this->AddChild(next_child);
                mbr1 = Rect::unir(mbr1, next_child->CalculateMBR());
            } else {
                pNewNode->AddChild(next_child);
                mbr2 = Rect::unir(mbr2, next_child->CalculateMBR());
            }
        }
        all_entries.erase(all_entries.begin() + next_idx);
        all_children.erase(all_children.begin() + next_idx);
    }
}

template <typename Trait>
void CBTreePage<Trait>::PickSeeds(std::vector<ObjectInfo>& entries, int& seed1, int& seed2) {
    int max_waste = -1;
    seed1 = 0; seed2 = 1;
    for (size_t i = 0; i < entries.size(); ++i) {
        for (size_t j = i + 1; j < entries.size(); ++j) {
            keyType combined = Rect::unir(entries[i].key, entries[j].key);
            int waste = combined.area() - entries[i].key.area() - entries[j].key.area();
            if (waste > max_waste) {
                max_waste = waste;
                seed1 = i;
                seed2 = j;
            }
        }
    }
}

template <typename Trait>
int CBTreePage<Trait>::PickNext(std::vector<ObjectInfo>& entries, keyType& mbr1, keyType& mbr2) {
    int max_diff = -1;
    int next = 0;
    for (size_t i = 0; i < entries.size(); ++i) {
        int expansion1 = Rect::expansionNecesaria(mbr1, entries[i].key);
        int expansion2 = Rect::expansionNecesaria(mbr2, entries[i].key);
        int diff = std::abs(expansion1 - expansion2);
        if (diff > max_diff) {
            max_diff = diff;
            next = i;
        }
    }
    return next;
}

template <typename Trait>
void CBTreePage<Trait>::Print(ostream & os) const
{
       ForEach(0, &::PrintObjectInfoHelper<keyType, ObjIDType>, &os);
}

#endif