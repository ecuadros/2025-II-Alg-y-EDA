#ifndef __CBTreePage_H__
#define __CBTreePage_H__

#include <vector>
#include <assert.h>
#include <functional>

/**
 * @file CBTreePage.h
 * @brief Declaración de la clase `CBTreePage` y funciones auxiliares relacionadas con la implementación de un árbol B.
 *
 * Este archivo contiene la declaración de una clase que representa una página dentro de un árbol B,
 * así como funciones adicionales para manipular objetos de la estructura de datos.
 * Las funciones auxiliares y los TODOs indican áreas de mejora o funcionalidades pendientes de implementación.
 *
 * @todo #1 Crear una función para agregarla al demo.cpp.
 * @todo #2 Agregar un trait a la clase.
 * @todo #3 Crear un iterador para recorrer las páginas del árbol.
 * @todo #4 Integrar la función de recorrido al árbol B.
 */


// TODO: #1 Crear una function para agregarla al demo.cpp ( no trivial )
// TODO: #2 Agregarle un Trait (prueba git) ( no trivial )
// TODO: #3 crear un iterator ( no trivial )
//       Sugerencia: Tarea1 cada pagina debe tener un puntero al padre primero ( no trivial )
// TODO: #4 integrarlo al recorrer ( no trivial )


template <typename Trait>
class BTree;

using namespace std;

/**
 * @enum bt_ErrorCode
 * @brief Códigos de error utilizados en las operaciones del árbol B.
 *
 * Esta enumeración define los posibles resultados de las operaciones del árbol B,
 * tales como inserciones, eliminaciones, y búsquedas. Cada código indica un tipo específico
 * de error o el estado de éxito de la operación.
 */

enum bt_ErrorCode {bt_ok, bt_overflow, bt_underflow, bt_duplicate, bt_nofound, bt_rootmerged};


/**
 * @brief Realiza una búsqueda binaria en un contenedor.
 * 
 * Esta función realiza una búsqueda binaria en un contenedor para encontrar la posición en la que 
 * un objeto debería insertarse o para comprobar su existencia. La búsqueda binaria es eficiente 
 * y se realiza sobre un contenedor ordenado.
 * 
 * @tparam Container El tipo de contenedor sobre el que se realiza la búsqueda.
 * @tparam ObjType El tipo de los objetos que se almacenan en el contenedor.
 * @tparam Functor El tipo de la función de comparación que se usa para comparar objetos.
 * 
 * @param container El contenedor sobre el cual se realiza la búsqueda.
 * @param first El índice inicial del contenedor donde comienza la búsqueda.
 * @param last El índice final del contenedor donde termina la búsqueda.
 * @param object El objeto que se busca o se desea insertar.
 * @param compare La función de comparación que se utiliza para ordenar los elementos del contenedor. 
 *                Por defecto, se utiliza `std::less<ObjType>`.
 * 
 * @return El índice en el que se encuentra el objeto o el índice donde debería insertarse.
 * 
 * @note Esta implementación de búsqueda binaria es genérica y se puede utilizar con cualquier tipo de
 *       contenedor que permita el acceso mediante índices, como un `std::vector`.
 */

template <typename Container, typename ObjType, typename Functor = std::less<ObjType> >
size_t binary_search(Container& container, size_t first, size_t last, ObjType &object, Functor compare)
{
       if( first >= last )
               return first;
       while( first < last )
       {
               size_t mid = (first+last)/2;
        //        if( object == (ObjType)container[mid ] )
        //                return mid;
        //        if( object > (ObjType)container[mid ] )
        //                first = mid+1;
        //        else
        //                last  = mid;
                ObjType midObj = (ObjType)container[mid];
                if( !std::invoke(compare, midObj, object) && !std::invoke(compare, object, midObj) )
                        return mid;
                if( std::invoke(compare, midObj, object) )
                        first = mid+1;
                else
                        last  = mid;
       }
//        if( object <= (ObjType)container[first] )
//                return first;
//        return last;
          if( !std::invoke(compare, (ObjType)container[first], object) )
                  return first;
          return last;
}



// Error al poner size_t
// Posible motivo: El i está disminuyendo
template <typename Container, typename ObjType>
void insert_at(Container& container, ObjType object, int pos)
{
        // TODO: #5 replace int, long by types such as size_t
       size_t size = container.size();
       for(int i = size-2 ; i >= pos ; i--)
               container[i+1] = container[i];
       container[pos] =  object;
}


/**
 * @brief Elimina un objeto en una posición específica dentro de un contenedor.
 * 
 * Este método mueve todos los elementos después de la posición especificada una posición a la izquierda
 * para cubrir el espacio dejado por el objeto eliminado. 
 * 
 * @tparam Container El tipo del contenedor en el que se realiza la eliminación (por ejemplo, `std::vector`).
 * 
 * @param container El contenedor del cual se eliminará el objeto.
 * @param pos La posición del objeto a eliminar dentro del contenedor.
 */

template <typename Container>
void remove(Container& container, size_t pos)
{
       size_t size = container.size();
       for(auto i = pos+1 ; i < size ; i++)
           container[i-1] = container[i];
}

/**
 * @struct tagObjectInfo
 * @brief Estructura que contiene la información de un objeto en el árbol B.
 * 
 * Esta estructura almacena información relacionada con las claves de los objetos y su identificación
 * dentro del árbol B. Además, incluye un contador de uso para realizar un seguimiento de la cantidad de
 * veces que un objeto ha sido accedido o utilizado.
 * 
 * @tparam keyType El tipo de clave asociado con el objeto.
 * @tparam ObjIDType El tipo de identificación asociado con el objeto.
 */
template <typename keyType, typename ObjIDType>
struct tagObjectInfo
{
    keyType key;             ///< La clave asociada con el objeto.
    ObjIDType ObjID;         ///< El identificador único del objeto.
    size_t UseCounter;       ///< El contador de uso del objeto.
    
    /**
     * @brief Constructor que inicializa la clave y el identificador del objeto.
     * 
     * @param _key La clave del objeto.
     * @param _ObjID El identificador único del objeto.
     */
    tagObjectInfo(const keyType &_key, ObjIDType _ObjID)
        : key(_key), ObjID(_ObjID), UseCounter(0) {}
    
    /**
     * @brief Constructor de copia para crear un nuevo objeto a partir de otro.
     * 
     * @param objInfo El objeto a copiar.
     */
    tagObjectInfo(const tagObjectInfo &objInfo)
        : key(objInfo.key), ObjID(objInfo.ObjID), UseCounter(0) {}

    /**
     * @brief Constructor por defecto.
     */
    tagObjectInfo() {}

    /**
     * @brief Conversión implícita a tipo `keyType`.
     * 
     * Permite usar un objeto `tagObjectInfo` como una clave en otras estructuras de datos.
     * 
     * @return La clave del objeto.
     */
    operator keyType() { return key; }

    /**
     * @brief Obtiene el contador de uso del objeto.
     * 
     * @return El contador de uso.
     */
    size_t GetUseCounter() { return UseCounter; }
};


/**
 * @class CBTreePage
 * @brief Representa una página dentro de un árbol B en memoria.
 * 
 * La clase `CBTreePage` es una implementación en memoria de una página del árbol B. Cada página contiene un conjunto
 * de claves y referencias a otras páginas (subpáginas). Esta clase es responsable de manejar la inserción, eliminación,
 * búsqueda, y otras operaciones dentro de una página del árbol B.
 * 
 * @tparam Trait El tipo de traits que define las propiedades del árbol B, como el tipo de clave y el tipo de comparación.
 */
template <typename Trait>
class CBTreePage //: public SimpleIndex <keyType>
// this is the in-memory version of the CBTreePage
{
       friend class BTree<Trait>;
       typedef typename Trait::keyType  keyType;
       typedef typename Trait::ObjIDType  ObjIDType;
       typedef typename Trait::CompareFunction CompareFunction;

       typedef CBTreePage<Trait>    BTPage;         // useful shorthand
       typedef tagObjectInfo<keyType, ObjIDType> ObjectInfo;

public:
       CBTreePage(size_t maxKeys, bool unique = true);
       virtual ~CBTreePage();
       //move constuctor
       CBTreePage(CBTreePage &&other);
       //operator= for move constructor
       CBTreePage& operator=(CBTreePage &&other);

       template <typename T>
       friend std::ostream& operator<<(std::ostream& os, CBTreePage<T>& page);

       bt_ErrorCode    Insert (const keyType &key, const ObjIDType ObjID);
       bt_ErrorCode    Remove (const keyType &key, const ObjIDType ObjID);
       bool            Search (const keyType &key, ObjIDType &ObjID);
       void            Print  (ostream &os);

       size_t GetKeyCount();

       // TODO: #6 change by Invoke
       // TODO: #7 ForEach must be a template inside this template
//        void            ForEach(lpfnForEach2 lpfn, size_t level, void *pExtra1);
//        void            ForEach(lpfnForEach3 lpfn, size_t level, void *pExtra1, void *pExtra2);
        template <typename Function>
        void            ForEach(Function fn, size_t level);


       // TODO: #8 You may reduce these two function by using Invoke
//        ObjectInfo*     FirstThat(lpfnFirstThat2 lpfn, size_t level, void *pExtra1);
//        ObjectInfo*     FirstThat(lpfnFirstThat3 lpfn, size_t level, void *pExtra1, void *pExtra2);
        template <typename Function>
        ObjectInfo*     FirstThat(Function fn, size_t level);

        //agregamos metodo Write para escribir el contenido de un arbol
        std::ostream&   Write(ostream &os);
        //agregamos metodo Read para leer el contenido de un arbol
        std::istream& Read(istream &is);


protected:
       // TODO: #9 change by size_t
       size_t  m_MinKeys; // minimum number of keys in a node
       size_t  m_MaxKeys, // maximum number of keys in a node

                m_MaxKeysForChilds; // just to distinguish the root
       bool m_Unique;
       bool m_isRoot;
       //size_t           NextNode; // address of next node at same level
       //size_t RecAddr; // address of this node in the BTree file
       vector<ObjectInfo> m_Keys;
       vector<BTPage *>m_SubPages;

       // TODO: #10 size_t
       size_t  m_KeyCount;

       CompareFunction m_Compare;

       void  Create();
       void  Reset ();
       void  Destroy () {   Reset(); delete this;}
       void  clear ();

       bool  RedistributeWith1Brother   (size_t &pos);
       bool  RedistributeWith2Brothers   (size_t pos);
       void  RedistributeR2L (size_t pos);
       void  RedistributeL2R (size_t pos);

       bool    TreatUnderflow  (size_t &pos)
       {       return RedistributeWith1Brother(pos) || RedistributeWith2Brothers(pos);}

       bt_ErrorCode    Merge  (size_t pos);
       bt_ErrorCode    MergeRoot ();
       void  SplitChild (size_t pos);

       ObjectInfo &GetFirstObjectInfo();

       bool Overflow()  { return m_KeyCount > m_MaxKeys; }
       bool Underflow() { return m_KeyCount < MinNumberOfKeys(); }
       bool IsFull()    { return m_KeyCount >= m_MaxKeys; }


       size_t  MinNumberOfKeys()  { return 2*m_MaxKeys/3.0; }
       size_t  GetFreeCells()  { return m_MaxKeys - m_KeyCount; }
       size_t& NumberOfKeys()  { return m_KeyCount; }
       size_t  GetNumberOfKeys()  { return m_KeyCount; }
       bool IsRoot()  { return m_MaxKeysForChilds != m_MaxKeys; }
       void SetMaxKeysForChilds(size_t orderforchilds)
       {        m_MaxKeysForChilds = orderforchilds;       }
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
       bool SplitRoot();
       void SplitPageInto3(vector<ObjectInfo>   & tmpKeys,
                                               vector<BTPage *>  & SubPages,
                                               BTPage           *& pChild1,
                                               BTPage           *& pChild2,
                                               BTPage           *& pChild3,
                                               ObjectInfo        & oi1,
                                               ObjectInfo        & oi2);
       void MovePage(BTPage *  pChildPage,vector<ObjectInfo> & tmpKeys,vector<BTPage *> & tmpSubPages);
};

/**
 * @brief Constructor para crear una página del árbol B.
 * 
 * Este constructor inicializa una nueva página en el árbol B con un número máximo de claves (`maxKeys`) y la opción
 * de hacer las claves únicas (`unique`). El número de claves es inicializado a cero. Luego, se llaman a los métodos `Create()`
 * y `SetMaxKeysForChilds()` para completar la configuración de la página.
 * 
 * @tparam Trait El tipo de los traits que define las propiedades del árbol B.
 * 
 * @param maxKeys El número máximo de claves que esta página puede contener.
 * @param unique Indica si las claves deben ser únicas en esta página (por defecto es `true`).
 */

template <typename Trait>
CBTreePage<Trait>:: CBTreePage(size_t maxKeys, bool unique)
                               : m_MaxKeys(maxKeys), m_Unique(unique), m_KeyCount(0)
{
       Create();
       SetMaxKeysForChilds(m_MaxKeys);
}

/**
 * @brief Destructor de la clase `CBTreePage`.
 * 
 * El destructor de la página del árbol B llama al método `Reset()` para liberar los recursos asociados a esta página.
 */

template <typename Trait>
CBTreePage<Trait>::~CBTreePage()
{
       Reset();
}


//move constructor
/**
 * @brief Constructor de movimiento para transferir los recursos de otra página al nuevo objeto.
 * 
 * Este constructor toma una página existente (pasada como `other`) y transfiere sus recursos (como claves, subpáginas y configuraciones)
 * al nuevo objeto. Después de la transferencia, los atributos de `other` se restablecen a sus valores predeterminados.
 * 
 * @tparam Trait El tipo de los traits que define las propiedades del árbol B.
 * 
 * @param other La página de la cual se moverán los recursos al nuevo objeto.
 * 
 * @note Este constructor permite la transferencia eficiente de recursos, evitando copias innecesarias.
 */
template <typename Trait>
CBTreePage<Trait>::CBTreePage(CBTreePage &&other){
        // std::cout << "CBTreePage MOVE CONSTRUCTOR debug\n";

        m_Keys          = std::move(other.m_Keys);
        m_SubPages      = std::move(other.m_SubPages);
        m_KeyCount      = other.m_KeyCount;
        m_MaxKeys       = other.m_MaxKeys;
        m_MinKeys       = other.m_MinKeys;
        m_Unique        = other.m_Unique;
        m_isRoot        = other.m_isRoot;
        m_Compare       = other.m_Compare;

        other.m_KeyCount = 0;
        other.m_MaxKeys  = 0;
        other.m_MinKeys  = 0;
}

//move assignment operator
/**
 * @brief Operador de asignación por movimiento para transferir los recursos de otra página al objeto actual.
 * 
 * Este operador permite asignar una página existente (pasada como `other`) al objeto actual, moviendo sus recursos
 * (como claves, subpáginas y configuraciones) en lugar de copiarlos. Después de la transferencia, los atributos de `other`
 * se restablecen a sus valores predeterminados. Este método es eficiente y evita la duplicación innecesaria de recursos.
 * 
 * @tparam Trait El tipo de los traits que define las propiedades del árbol B.
 * 
 * @param other La página de la cual se moverán los recursos al objeto actual.
 * 
 * @return Una referencia al objeto actual (`*this`) después de la asignación.
 * 
 * @note Si el objeto actual es el mismo que `other`, no se realiza ninguna operación.
 */

template <typename Trait>
CBTreePage<Trait>& CBTreePage<Trait>::operator=(CBTreePage &&other){
        // std::cout << "CBTreePage MOVE ASSIGNMENT OPERATOR debug\n";

        if(this != &other){
                m_Keys          = std::move(other.m_Keys);
                m_SubPages      = std::move(other.m_SubPages);
                m_KeyCount      = other.m_KeyCount;
                m_MaxKeys       = other.m_MaxKeys;
                m_MinKeys       = other.m_MinKeys;
                m_Unique        = other.m_Unique;
                m_isRoot        = other.m_isRoot;
                m_Compare       = other.m_Compare;

                other.m_KeyCount = 0;
                other.m_MaxKeys  = 0;
                other.m_MinKeys  = 0;
        }
        return *this;
}


/**
 * @brief Inserta una clave y su identificador de objeto en la página del árbol B.
 * 
 * Este método realiza la inserción de una clave en la página del árbol B. Si la clave ya existe y no se permiten claves duplicadas, el método retorna un error de tipo `bt_duplicate`. Si la página es una hoja, la clave se inserta directamente; si no, la inserción se realiza de manera recursiva en las subpáginas correspondientes.
 * 
 * Si se produce un desbordamiento en la página (es decir, el número de claves excede el máximo permitido), se intenta redistribuir los elementos con los hermanos adyacentes, o bien se divide la página en dos. Si el desbordamiento persiste, se propaga hacia arriba.
 * 
 * @tparam Trait El tipo de los traits que define las propiedades del árbol B.
 * 
 * @param key La clave que se desea insertar.
 * @param ObjID El identificador del objeto asociado a la clave.
 * 
 * @return Un código de error `bt_ErrorCode`, que puede ser:
 * - `bt_ok`: Inserción exitosa.
 * - `bt_duplicate`: La clave ya existe en la página y no se permiten duplicados.
 * - `bt_overflow`: La página ha desbordado.
 * 
 * @note Este método maneja la inserción tanto en páginas hoja como en subpáginas, y gestiona los desbordamientos mediante redistribución o división de páginas.
 */
template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::Insert(const keyType& key, const ObjIDType ObjID){
       size_t pos = binary_search(m_Keys, 0, m_KeyCount, key, m_Compare);
       bt_ErrorCode error = bt_ok;

//        if( pos < m_KeyCount && (keyType)m_Keys[pos] == key && m_Unique)
//                return bt_duplicate; // this key is duplicate
        if( pos < m_KeyCount && (!m_Compare(key, m_Keys[pos].key) && !m_Compare(m_Keys[pos].key, key)) && m_Unique)
               return bt_duplicate;

       if( !m_SubPages[pos] ){ // this is a leave
               ::insert_at(m_Keys, ObjectInfo(key, ObjID), pos);
               m_KeyCount++;
               if( Overflow() )
                       return bt_overflow;
               return bt_ok;
       }
       else{
               // recursive insertion
               error = m_SubPages[pos]->Insert(key, ObjID);
               if( error == bt_overflow ){
                       if( !RedistributeWith1Brother(pos) )
                               SplitChild(pos);
                       if( Overflow() )  // Propagate overflow
                               return bt_overflow;
                       return bt_ok;
               }
       }
       if( Overflow() ) // node overflow
               return bt_overflow;
       return bt_ok;
}


/**
 * @brief Redistribuye las claves de un nodo B-Tree con uno de sus hermanos.
 * 
 * Este método maneja tanto los casos de subfluyo (cuando un nodo tiene menos claves de las necesarias) como
 * los casos de desbordamiento (cuando un nodo tiene más claves de las que puede manejar). Dependiendo de la situación, 
 * se redistribuirán las claves con el hermano izquierdo o derecho del nodo, o se realizará una fusión si no es posible 
 * la redistribución.
 * 
 * @param pos La posición del nodo en el arreglo de subpáginas. Se pasa como referencia ya que puede modificarse 
 *            durante el proceso de redistribución.
 * 
 * @return `true` si la redistribución fue exitosa; de lo contrario, `false` si no es posible realizar la 
 *         redistribución debido a la falta de espacio o las condiciones del árbol.
 * 
 * @details Este método primero verifica si el nodo en la posición indicada está en un estado de "subfluyo" (es decir, 
 *          tiene menos claves de las necesarias). Si es así, el algoritmo intenta redistribuir las claves con el hermano 
 *          izquierdo o derecho. Si el nodo está en estado de "desbordamiento", se intentará hacer espacio en el nodo 
 *          redistribuyendo claves con sus hermanos.
 * 
 * @note Este método es crítico para mantener el equilibrio del árbol B al garantizar que los nodos no se desborden o 
 *       queden en un estado subfluyente sin ser manejados adecuadamente.
 */
template <typename Trait>
bool CBTreePage<Trait>::RedistributeWith1Brother(size_t &pos)
{
       if( m_SubPages[pos]->Underflow() )
       {
               size_t NumberOfKeyOnLeft = 0,
                       NumberOfKeyOnRight = 0;
               // is this the first element or there are more elements on right brother
               if( pos > 0 )
                       NumberOfKeyOnLeft = m_SubPages[pos-1]->NumberOfKeys();
               if( pos < NumberOfKeys() )
                       NumberOfKeyOnRight = m_SubPages[pos+1]->NumberOfKeys();

               if( NumberOfKeyOnLeft > NumberOfKeyOnRight )
                       if( m_SubPages[pos-1]->NumberOfKeys() > m_SubPages[pos-1]->MinNumberOfKeys() )
                               RedistributeL2R(pos-1); // bring elements from left brother
                       else
                               if( pos == NumberOfKeys() )
                                       return (--pos, false);
                               else
                                       return false;
               else //NumberOfKeyOnLeft < NumberOfKeyOnRight )
                       if( m_SubPages[pos+1]->NumberOfKeys() > m_SubPages[pos+1]->MinNumberOfKeys() )
                               RedistributeR2L(pos+1); // bring elements from right brother
                       else
                               if( pos == 0 )
                                       return (++pos, false);
                               else
                                       return false;
       }
       else // it is due to overflow
       {
               size_t FreeCellsOnLeft = GetFreeCellsOnLeft(pos),   // Free Cells On Left
               fcor = GetFreeCellsOnRight(pos);  // Free Cells On Right

               if( !FreeCellsOnLeft && !fcor && m_SubPages[pos]->IsFull() )
                       return false;
               if( FreeCellsOnLeft > fcor ) // There is more space on left
                       RedistributeR2L(pos);
               else
                       RedistributeL2R(pos);

       }
       return true;
}

/** RedistributeWith2Brothers function
   it considers two brothers m_SubPages[pos-1] && m_SubPages[pos+1]
   if it fails the only way is merge !
**/


/**
 * @brief Redistribuye las claves entre tres nodos hermanos en un árbol B cuando dos de ellos están en subfluyo.
 * 
 * Este método maneja una situación en la que el nodo actual y al menos uno de sus hermanos (izquierdo o derecho) 
 * están en estado de subfluyo, es decir, contienen menos claves de las necesarias para cumplir con las condiciones 
 * del árbol B. El método redistribuye las claves entre los tres nodos (el nodo actual y sus dos hermanos) para 
 * equilibrar la cantidad de claves.
 * 
 * @param pos La posición del nodo en el arreglo de subpáginas, que se pasa como referencia. El nodo actual está 
 *            en la posición `pos`, con un hermano a la izquierda (`pos-1`) y uno a la derecha (`pos+1`).
 * 
 * @return `true` si la redistribución se ha realizado correctamente. Si no es posible realizar la redistribución 
 *         debido a que el estado de subfluyo persiste en alguno de los nodos, se retorna `false`.
 * 
 * @details Este método intenta redistribuir las claves de los nodos vecinos de la siguiente manera:
 * 
 * - Si el hermano izquierdo está en subfluyo, las claves se redistribuyen hacia el nodo actual y el hermano izquierdo 
 *   mediante una rotación de derecha a izquierda (`R2L`).
 * 
 * - Si el hermano derecho está en subfluyo, las claves se redistribuyen hacia el nodo actual y el hermano derecho 
 *   mediante una rotación de izquierda a derecha (`L2R`).
 * 
 * - Si ambos hermanos están en subfluyo, se realiza primero una rotación de izquierda a derecha en el hermano izquierdo, 
 *   seguida de una rotación de derecha a izquierda en el hermano derecho.
 * 
 * @note Este método es fundamental para mantener el equilibrio del árbol B y garantizar que los nodos no se 
 *       queden en un estado de subfluyo durante las operaciones de inserción y eliminación de claves.
 */
template <typename Trait>
bool CBTreePage<Trait>::RedistributeWith2Brothers(size_t pos)
{
       assert( pos > 0 && pos < NumberOfKeys()  );
       assert( m_SubPages[pos-1] != 0 && m_SubPages[pos] != 0 && m_SubPages[pos+1] != 0 );
       assert( m_SubPages[pos-1]->Underflow() ||
                       m_SubPages[ pos ]->Underflow() ||
                       m_SubPages[pos+1]->Underflow() );

       if( m_SubPages[pos-1]->Underflow() )
       {       // Rotate R2L
               RedistributeR2L(pos+1);
               RedistributeR2L(pos);
               if( m_SubPages[pos-1]->Underflow() )
                       return false;
       }
       else if( m_SubPages[pos+1]->Underflow() )
       {       // Rotate L2R
               RedistributeL2R(pos-1);
               RedistributeL2R(pos);
               if( m_SubPages[pos+1]->Underflow() )
                       return false;
       }
       else // The problem is exactly at pos !
       {
               // Rotate L2R
               RedistributeL2R(pos-1);
               RedistributeR2L(pos+1);
               if( m_SubPages[pos]->Underflow() )
                       return false;
       }
       return true;
}

/**
 * @brief Redistribuye claves y subpáginas de derecha a izquierda entre dos nodos hijos.
 * 
 * Este método toma claves y punteros desde el nodo hijo en la posición `pos` (nodo derecho) y los 
 * inserta en su hermano izquierdo (`pos-1`). La clave correspondiente en el nodo padre también 
 * se ajusta para mantener las propiedades del árbol B.
 * 
 * @param pos Posición del nodo hijo derecho en el vector `m_SubPages`. El hermano izquierdo está en `pos-1`.
 * 
 * @details La redistribución ocurre mientras el nodo derecho (source) tenga más claves que su mínimo permitido 
 *          (`MinNumberOfKeys`) y el nodo izquierdo (target) tenga menos claves que el nodo derecho. En cada iteración:
 *          - Se mueve la clave del nodo padre `m_Keys[pos-1]` al final del nodo izquierdo.
 *          - Se mueve el primer puntero del nodo derecho al nodo izquierdo.
 *          - La primera clave del nodo derecho se mueve al nodo padre (`m_Keys[pos-1]`).
 *          - Se elimina la primera clave y puntero del nodo derecho y se decrementa su contador de claves.
 * 
 * @note Este método es útil para resolver underflow (subfluyo) en árboles B sin necesidad de hacer split o merge, 
 *       redistribuyendo claves entre hermanos adyacentes.
 */
template <typename Trait>
void CBTreePage<Trait>::RedistributeR2L(size_t pos)
{
       BTPage  *pSource = m_SubPages[ pos ],
               *pTarget = m_SubPages[pos-1];

       while(pSource->GetNumberOfKeys() > pSource->MinNumberOfKeys() &&
                 pTarget->GetNumberOfKeys() < pSource->GetNumberOfKeys() )
       {
               // Move from this page to the down-left page \/
               ::insert_at(pTarget->m_Keys, m_Keys[pos-1], pTarget->NumberOfKeys()++);
               // Move the pointer leftest pointer to the rightest position
               ::insert_at(pTarget->m_SubPages, pSource->m_SubPages[0], pTarget->NumberOfKeys());

               // Move the leftest element to the root
               m_Keys[pos-1] = pSource->m_Keys[0];

               // Remove the leftest element from rigth page
               ::remove(pSource->m_Keys    , 0);
               ::remove(pSource->m_SubPages, 0);
               pSource->NumberOfKeys()--;
       }
}


/**
 * @brief Redistribuye claves y subpáginas de izquierda a derecha entre dos nodos hijos.
 * 
 * Este método toma claves y punteros desde el nodo hijo en la posición `pos` (nodo izquierdo) 
 * y los inserta en su hermano derecho (`pos+1`). La clave correspondiente en el nodo padre también 
 * se ajusta para mantener las propiedades del árbol B.
 * 
 * @param pos Posición del nodo hijo izquierdo en el vector `m_SubPages`. El hermano derecho está en `pos+1`.
 * 
 * @details La redistribución ocurre mientras el nodo izquierdo (source) tenga más claves que su mínimo permitido 
 *          (`MinNumberOfKeys`) y el nodo derecho (target) tenga menos claves que el nodo izquierdo. En cada iteración:
 *          - Se mueve la clave del nodo padre `m_Keys[pos]` al principio del nodo derecho.
 *          - Se mueve el último puntero del nodo izquierdo al principio del nodo derecho.
 *          - La última clave del nodo izquierdo se mueve al nodo padre (`m_Keys[pos]`).
 *          - Se elimina la última clave del nodo izquierdo y se decrementa su contador de claves.
 * 
 * @note Este método es útil para resolver underflow (subfluyo) en árboles B sin necesidad de hacer split o merge, 
 *       redistribuyendo claves entre hermanos adyacentes.
 */
template <typename Trait>
void CBTreePage<Trait>::RedistributeL2R(size_t pos)
{
       BTPage  *pSource = m_SubPages[pos],
                       *pTarget = m_SubPages[pos+1];
       while(pSource->GetNumberOfKeys() > pSource->MinNumberOfKeys() &&
                 pTarget->GetNumberOfKeys() < pSource->GetNumberOfKeys() )
       {
               // Move from this page to the down-RIGHT page \/
               ::insert_at(pTarget->m_Keys, m_Keys[pos], 0);
               // Move the pointer rightest pointer to the leftest position
               ::insert_at(pTarget->m_SubPages, pSource->m_SubPages[pSource->NumberOfKeys()], 0);
               pTarget->NumberOfKeys()++;

               // Move the rightest element to the root
               m_Keys[pos] = pSource->m_Keys[pSource->NumberOfKeys()-1];

               // Remove the leftest element from rigth page
               // it is not necessary erase because m_KeyCount controls
               pSource->NumberOfKeys()--;
       }
}


/**
 * @brief Divide un nodo hijo lleno en tres nodos.
 * 
 * Este método maneja la división de un nodo hijo que se encuentra lleno. La división implica tomar dos nodos hijos,
 * combinarlos temporalmente, y luego dividirlos en tres nodos distintos. El nodo padre se actualiza con las claves 
 * correspondientes, y los nodos hijos se reorganizan.
 * 
 * @param pos Posición del nodo hijo en el vector `m_SubPages`. El nodo hijo en `pos` se divide, y el nodo 
 *            adyacente izquierdo o derecho también se involucra en la división si está lleno.
 * 
 * @details El proceso de división consta de los siguientes pasos:
 * 1. Se identifican los nodos hijos a dividir (izquierdo y derecho) según su posición en el vector `m_SubPages`.
 * 2. Se crea un vector temporal para almacenar las claves y los punteros de los nodos hijos.
 * 3. Las claves y los punteros de los nodos hijos izquierdo y derecho se copian al vector temporal.
 * 4. El nodo padre se actualiza con dos nuevas claves (una de cada nodo hijo).
 * 5. Los nodos hijos resultantes se reorganizan y se asignan a las posiciones correspondientes.
 * 
 * @note Este método se llama cuando un nodo hijo se llena y necesita ser dividido para mantener las propiedades del árbol B.
 */
template <typename Trait>
void CBTreePage<Trait>::SplitChild(size_t pos)
{
       // FIRST: deciding the second page to split
       BTPage  *pChild1 = 0, *pChild2 = 0;
       if( pos > 0 )                                   // is left page full ?
               if( m_SubPages[pos-1]->IsFull() )
               {
                       pChild1 = m_SubPages[pos-1];
                       pChild2 = m_SubPages[pos--];
               }
       if( pos < GetNumberOfKeys() )   // is right page full ?
               if( m_SubPages[pos+1]->IsFull() )
               {
                       pChild1 = m_SubPages[pos];
                       pChild2 = m_SubPages[pos+1];
               }
       size_t nKeys = pChild1->GetNumberOfKeys() + pChild2->GetNumberOfKeys() + 1;

       // SECOND: copy both pages to a temporal one
       // Create two tmp vector
       vector<ObjectInfo> tmpKeys;
       //tmpKeys.resize(nKeys);
       vector<BTPage *>   tmpSubPages;
       //tmpKeys.resize(nKeys+1);

       // copy from left child
       MovePage(pChild1, tmpKeys, tmpSubPages);
       // copy a key from parent
       tmpKeys    .push_back(m_Keys[pos]);

       // copy from right child
       MovePage(pChild2, tmpKeys, tmpSubPages);

       BTPage *pChild3 = 0;
       ObjectInfo oi1, oi2;
       SplitPageInto3(tmpKeys, tmpSubPages, pChild1, pChild2, pChild3, oi1, oi2);

       // copy the first element to the root
       m_Keys    [pos] = oi1;
       m_SubPages[pos] = pChild1;

       // copy the second element to the root
       ::insert_at(m_Keys, oi2, pos+1);
       ::insert_at(m_SubPages, pChild2, pos+1);
       NumberOfKeys()++;

       m_SubPages[pos+2] = pChild3;
}

// Ddivide a large page into 3 pages (2m/3 each one)

/**
 * @brief Divide un conjunto de claves y subpáginas en tres nodos hijos.
 * 
 * Este método toma las claves y subpáginas de un nodo que se está dividiendo y las distribuye en tres nuevos nodos hijos. 
 * Los dos primeros nodos obtienen aproximadamente un tercio de las claves y subpáginas cada uno, mientras que el tercer nodo 
 * recibe el resto. Los elementos intermedios se "suben" al nodo padre.
 * 
 * @param tmpKeys Referencia a un vector que contiene las claves que se van a dividir entre los tres nodos hijos.
 * @param tmpSubPages Referencia a un vector que contiene los punteros a las subpáginas (hijos) que se van a dividir.
 * @param pChild1 Puntero a un puntero de un `BTPage`, donde se almacenará el primer nodo hijo creado.
 * @param pChild2 Puntero a un puntero de un `BTPage`, donde se almacenará el segundo nodo hijo creado.
 * @param pChild3 Puntero a un puntero de un `BTPage`, donde se almacenará el tercer nodo hijo creado.
 * @param oi1 Clave que se "sube" al nodo padre desde el primer conjunto de claves.
 * @param oi2 Clave que se "sube" al nodo padre desde el segundo conjunto de claves.
 * 
 * @details Este método distribuye las claves y subpáginas de la siguiente manera:
 * 1. Se calcula el número de claves que deben ir al primer nodo hijo (`pChild1`), que contiene aproximadamente un tercio de las claves.
 * 2. Se mueve un conjunto de claves al segundo nodo hijo (`pChild2`), que recibe otro tercio de las claves.
 * 3. El resto de las claves se asigna al tercer nodo hijo (`pChild3`).
 * 4. Las dos claves intermedias se "suben" al nodo padre.
 * 
 * @note El proceso asegura que las claves y subpáginas se distribuyan equitativamente entre los tres nodos hijos, 
 * cumpliendo con las restricciones de un árbol B.
 */
template <typename Trait>
void CBTreePage<Trait>::SplitPageInto3(vector<ObjectInfo>& tmpKeys,
                                                vector<BTPage *>  & tmpSubPages,
                                                BTPage*                   &     pChild1,
                                                BTPage*                   &     pChild2,
                                                BTPage*                   &     pChild3,
                                                ObjectInfo                & oi1,
                                                ObjectInfo                & oi2)
{
       assert(tmpKeys.size() >= 8);
       assert(tmpSubPages.size() >= 9);
       if( !pChild1 )
               pChild1 = new BTPage(m_MaxKeysForChilds, m_Unique);

       // Split tmpKeys page into 3 pages
       // copy 1/3 elements to the first child
       pChild1->clear();
       size_t nKeys = (tmpKeys.size()-2)/3;
       size_t i = 0;
       for(; i < nKeys; i++ )
       {
               pChild1->m_Keys    [i] = tmpKeys    [i];
               pChild1->m_SubPages[i] = tmpSubPages[i];
               pChild1->NumberOfKeys()++;
       }
       pChild1->m_SubPages[i] = tmpSubPages[i];

       // first element to go up !
       oi1 = tmpKeys[i++];

       if( !pChild2 )
               pChild2 = new BTPage(m_MaxKeysForChilds, m_Unique);
       pChild2->clear();
       // copy 1/3 to the second child
       nKeys += (tmpKeys.size()-2)/3 + 1;
       size_t j = 0;
       for(; i < nKeys; i++, j++ )
       {
               pChild2->m_Keys    [j] = tmpKeys    [i];
               pChild2->m_SubPages[j] = tmpSubPages[i];
               pChild2->NumberOfKeys()++;
       }
       pChild2->m_SubPages[j] = tmpSubPages[i];

       // copy the second element to the root
       oi2 = tmpKeys[i++];

       // copy 1/3 to the third child
       if( !pChild3 )
               pChild3 = new BTPage(m_MaxKeysForChilds, m_Unique);
       pChild3->clear();
       nKeys = tmpKeys.size();
       for(j = 0; i < nKeys; i++, j++)
       {
               pChild3->m_Keys    [j] = tmpKeys    [i];
               pChild3->m_SubPages[j] = tmpSubPages[i];
               pChild3->NumberOfKeys()++;
       }
       pChild3->m_SubPages[j] = tmpSubPages[i];
}

/**
 * @brief Divide la raíz de un árbol B en tres nodos hijos.
 * 
 * Este método maneja la división de la raíz del árbol B cuando se encuentra llena. La división implica tomar las claves
 * y subpáginas de la raíz, y distribuirlas en tres nuevos nodos hijos. Las dos claves intermedias se "suben" a la nueva raíz.
 * 
 * @return `true` si la división de la raíz se ha realizado correctamente.
 * 
 * @details El proceso de división consta de los siguientes pasos:
 * 1. Se crean tres nuevos nodos hijos (`pChild1`, `pChild2`, `pChild3`).
 * 2. Se llama al método `SplitPageInto3()` para distribuir las claves y subpáginas de la raíz entre los tres nodos hijos.
 * 3. La raíz se actualiza con las dos claves intermedias que se "suben" desde los nodos hijos.
 * 4. Los nodos hijos se asignan a las posiciones correspondientes en el vector de subpáginas de la raíz.
 * 
 * @note Este método es crucial para mantener las propiedades del árbol B cuando la raíz se llena, asegurando que el árbol
 *       permanezca equilibrado y eficiente para las operaciones de búsqueda e inserción.
 */
template <typename Trait>
bool CBTreePage<Trait>::SplitRoot(){
       BTPage  *pChild1 = 0, *pChild2 = 0, *pChild3 = 0;
       ObjectInfo oi1, oi2;
       SplitPageInto3( m_Keys,m_SubPages,pChild1, pChild2, pChild3, oi1, oi2);
       clear();

       // copy the first element to the root
       m_Keys    [0] = oi1;
       m_SubPages[0] = pChild1;
       NumberOfKeys()++;

       // copy the second element to the root
       m_Keys    [1] = oi2;
       m_SubPages[1] = pChild2;
       NumberOfKeys()++;

       m_SubPages[2] = pChild3;
       return true;
}


/**
 * @brief Busca una clave en la página del árbol B y obtiene su identificador de objeto.
 * 
 * Este método realiza una búsqueda binaria para localizar una clave específica en la página del árbol B. 
 * Si la clave se encuentra, se devuelve el identificador de objeto asociado. Si la clave no está presente 
 * en la página actual, la búsqueda se realiza recursivamente en las subpáginas correspondientes.
 * 
 * @tparam Trait El tipo de los traits que define las propiedades del árbol B.
 * 
 * @param key La clave que se desea buscar.
 * @param ObjID Referencia donde se almacenará el identificador del objeto si la clave es encontrada.
 * 
 * @return `true` si la clave fue encontrada y `ObjID` fue actualizado; `false` si la clave no está presente.
 * 
 * @note Este método utiliza una comparación personalizada definida por `m_Compare` para determinar el orden de las claves.
 */
template <typename Trait>
bool CBTreePage<Trait>::Search(const keyType &key, ObjIDType &ObjID)
{
       size_t pos = binary_search(m_Keys, 0, m_KeyCount, key, m_Compare);
       if( pos >= m_KeyCount )
       {    if( m_SubPages[pos] )
                return m_SubPages[pos]->Search(key, ObjID);
            else
                return false;
       }
//        if( key == m_Keys[pos].key )
       if( !m_Compare(key, m_Keys[pos].key) && !m_Compare(m_Keys[pos].key, key) )
       {
               ObjID = m_Keys[pos].ObjID;
               m_Keys[pos].UseCounter++;
               return true;
       }
//        if( key < m_Keys[pos].key )
       if( m_Compare(key, m_Keys[pos].key) )
               if( m_SubPages[pos] )
                       return m_SubPages[pos]->Search(key, ObjID);
       return false;
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

// template <typename Trait>
// void CBTreePage<Trait>::ForEach(lpfnForEach2 lpfn, size_t level, void *pExtra1)
// {
//        for(size_t i = 0 ; i < m_KeyCount ; i++)
//        {
//                if( m_SubPages[i] )
//                        m_SubPages[i]->ForEach(lpfn, level+1, pExtra1);
//                lpfn(m_Keys[i], level, pExtra1);
//        }
//        if( m_SubPages[m_KeyCount] )
//                m_SubPages[m_KeyCount]->ForEach(lpfn, level+1, pExtra1);
// }


//generalizamos el metodo for each
/*brief Aplica una función a cada clave en la página del árbol B, recorriéndola en orden.
 * 
 * Este método recorre todas las claves almacenadas en la página del árbol B y aplica una función proporcionada a cada clave. 
 * La función también recibe el nivel actual en el árbol como argumento, lo que permite realizar operaciones específicas según la profundidad.
 * 
 * @tparam Trait El tipo de los traits que define las propiedades del árbol B.
 * @tparam Function El tipo de la función que se aplicará a cada clave. Debe ser invocable con dos argumentos: la clave y el nivel.
 * 
 * @param fn La función que se aplicará a cada clave. Debe aceptar dos parámetros: la clave y el nivel.
 * @param level El nivel actual en el árbol B, utilizado para proporcionar contexto adicional a la función aplicada.
 * 
 * @details El método recorre las claves en orden ascendente. Para cada clave, primero se llama recursivamente a `ForEach` en la subpágina correspondiente (si existe), 
 *          luego se aplica la función `fn` a la clave actual junto con el nivel, y finalmente se continúa con la siguiente clave. Al final, también se llama a `ForEach` 
 *          en la última subpágina (si existe).
 */
template <typename Trait>
template <typename Function>
void CBTreePage<Trait>::ForEach(Function fn, size_t level){
       for(size_t i = 0 ; i < m_KeyCount ; i++)
       {
               if( m_SubPages[i] )
                       m_SubPages[i]->ForEach(fn, level+1);

               std::invoke(fn, m_Keys[i], level);
       }
       if( m_SubPages[m_KeyCount] )
               m_SubPages[m_KeyCount]->ForEach(fn, level+1);
}

/*template <typename keyType, typename ObjIDType>
void CBTreePage<keyType, ObjIDType>::ForEachReverse(lpfnForEach3 lpfn,
                                                                                                       size_t level, void *pExtra1, void *pExtra2)
{
       if( m_SubPages[m_KeyCount] )
               m_SubPages[m_KeyCount]->ForEach(lpfn, level+1, pExtra1, pExtra2);
       for(size_t i = m_KeyCount-1 ; i >= 0  ; i--)
       {
               lpfn(m_Keys[i], level, pExtra1, pExtra2);
               if( m_SubPages[i] )
                       m_SubPages[i]->ForEach(lpfn, level+1, pExtra1, pExtra2);
       }
}*/

// template <typename Trait>
// void CBTreePage<Trait>::ForEach(lpfnForEach3 lpfn, size_t level, void *pExtra1, void *pExtra2)
// {
//        for(size_t i = 0 ; i < m_KeyCount ; i++)
//        {
//                if( m_SubPages[i] )
//                        m_SubPages[i]->ForEach(lpfn, level+1, pExtra1, pExtra2);
//                lpfn(m_Keys[i], level, pExtra1, pExtra2);
//        }
//        if( m_SubPages[m_KeyCount] )
//                m_SubPages[m_KeyCount]->ForEach(lpfn, level+1, pExtra1, pExtra2);
// }

// Apicar una funcion hasta encontrar el 1er elemento
// aque que retorne true ante esta funcion
// template <typename Trait>
// typename CBTreePage<Trait>::ObjectInfo *
// CBTreePage<Trait>::FirstThat(lpfnFirstThat2 lpfn, size_t level, void *pExtra1)
// {
//        ObjectInfo *pTmp;
//        for(size_t i = 0 ; i < m_KeyCount ; i++)
//        {
//                if( m_SubPages[i] )
//                        if( (pTmp = m_SubPages[i]->FirstThat(lpfn, level+1, pExtra1)) )
//                                return pTmp;
//                if( lpfn(m_Keys[i], level, pExtra1) )
//                        return &m_Keys[i];
//        }
//        if( m_SubPages[m_KeyCount] )
//                if( (pTmp = m_SubPages[m_KeyCount]->FirstThat(lpfn, level+1, pExtra1)) )
//                        return pTmp;
//        return 0;
// }

// template <typename Trait>
// typename CBTreePage<Trait>::ObjectInfo *
// CBTreePage<Trait>::FirstThat(lpfnFirstThat3 lpfn,size_t level, void *pExtra1, void *pExtra2)
// {
//        ObjectInfo *pTmp;
//        for(size_t i = 0 ; i < m_KeyCount ; i++)
//        {
//                if( m_SubPages[i] )
//                        if( (pTmp = m_SubPages[i]->FirstThat(lpfn, level+1, pExtra1, pExtra2) ) )
//                                return pTmp;
//                if( lpfn(m_Keys[i], level, pExtra1, pExtra2) )
//                        return &m_Keys[i];
//        }
//        if( m_SubPages[m_KeyCount] )
//                if( (pTmp = m_SubPages[m_KeyCount]->FirstThat(lpfn, level+1, pExtra1, pExtra2) ) )
//                        return pTmp;
//        return 0;
// }


//generalizamos tambien first that
/**
 * @brief Busca el primer objeto que cumple con una condición.
 * 
 * Este método recursivo busca en las claves de la página actual (y sus subpáginas) el primer objeto que cumpla con una 
 * condición definida por una función. La búsqueda se realiza de forma descendente en las subpáginas y se detiene tan pronto 
 * como se encuentra un objeto que cumple la condición.
 * 
 * @tparam Function Tipo de la función que define la condición de búsqueda. La función debe ser invocable con una clave y un 
 * nivel como parámetros y debe devolver un valor booleano.
 * 
 * @param fn Función que define la condición de búsqueda. Esta función debe aceptar una clave y el nivel de profundidad actual 
 * y debe devolver `true` si la clave cumple con la condición, o `false` en caso contrario.
 * @param level Nivel actual de profundidad en el árbol. Se utiliza para la recursión en las subpáginas.
 * 
 * @return Puntero al primer objeto que cumple con la condición, o `nullptr` si no se encuentra ninguno.
 * 
 * @details Este método recursivo realiza una búsqueda en las claves del nodo actual. Para cada clave, invoca la función 
 * `fn` con la clave y el nivel actual. Si la función devuelve `true`, devuelve un puntero al objeto correspondiente. Si la clave 
 * no cumple con la condición, el método continúa buscando en las subpáginas del nodo, descendiendo recursivamente.
 * Si no se encuentra ningún objeto que cumpla con la condición, el método devuelve `nullptr`.
 * 
 * @note Este método es útil para realizar búsquedas complejas en un árbol B donde la condición de búsqueda no puede ser 
 * fácilmente representada mediante comparaciones simples.
 */
template <typename Trait>
template <typename Function>
typename CBTreePage<Trait>::ObjectInfo *
CBTreePage<Trait>::FirstThat(Function fn, size_t level)
{
       ObjectInfo *pTmp;
       for(size_t i = 0 ; i < m_KeyCount ; i++)
       {
               if( m_SubPages[i] )
                       if( (pTmp = m_SubPages[i]->FirstThat(fn, level+1) ) )
                               return pTmp;
               if( std::invoke(fn, m_Keys[i], level) )
                       return &m_Keys[i];
       }
       if( m_SubPages[m_KeyCount] )
               if( (pTmp = m_SubPages[m_KeyCount]->FirstThat(fn, level+1) ) )
                       return pTmp;
       return 0;
}


//implementamos el metodo write
/**
 * @brief Escribe la página del árbol B en un flujo de salida.
 * 
 * Este método serializa la información contenida en la página del árbol B a un flujo de salida, como `std::ofstream` o `std::cout`. La serialización incluye:
 * - La cantidad de claves en la página.
 * - Las claves y sus respectivos `ObjID`.
 * - Un marcador `ENDKEYS` que indica el final de las claves.
 * - Las subpáginas de la página, indicadas por "1" (si existen) o "0" (si no existen).
 * 
 * Además, si existen subpáginas, el método las serializa recursivamente.
 * 
 * @param os Flujo de salida donde se serializará la página.
 * @return El flujo de salida `os` para permitir encadenamiento de operaciones de salida.
 * 
 * @details La función escribe la cabecera con el número de claves en la página, luego escribe cada clave y su respectivo `ObjID`. Después, indica la existencia de las subpáginas y recursivamente escribe cada subpágina si es necesario.
 * 
 * @note Este método es útil para guardar el estado de una página del árbol B en un archivo o para depuración.
 */
template <typename Trait>
std::ostream& CBTreePage<Trait>::Write(std::ostream& os) {
    // Cabecera
    os << "PAGE " << m_KeyCount << "\n";
    
    // escribimos las claves y ObjID
    for(size_t i = 0; i < m_KeyCount; i++) {
        os << m_Keys[i].key << " " << m_Keys[i].ObjID << "\n";
    }
    os << "ENDKEYS\n"; //es importante para saber cuando terminan las claves
    
    // Indicamos que subpáginas existen con 1 o 0
    for(size_t i = 0; i <= m_KeyCount; i++) {
        os << (m_SubPages[i] ? "1" : "0") << " ";
    }
    os << "\n";
    
    // ahora escribimos recursivamente las subpaginas si existen
    for(size_t i = 0; i <= m_KeyCount; i++) {
        if(m_SubPages[i]) {
            m_SubPages[i]->Write(os);
        }
    }
    
    return os;
}

//implementamos el metodo read
/**
 * @brief Lee los datos de una página del árbol B desde un flujo de entrada.
 * 
 * Este método deserializa una página del árbol B desde un flujo de entrada, como `std::ifstream`. Los datos leídos incluyen:
 * - La cantidad de claves en la página.
 * - Las claves y sus respectivos `ObjID`.
 * - El marcador `ENDKEYS` que indica el fin de las claves.
 * - La existencia de las subpáginas.
 * 
 * Si existen subpáginas, las deserializa recursivamente.
 * 
 * @param is Flujo de entrada desde el cual se deserializará la página.
 * @return El flujo de entrada `is` para permitir encadenamiento de operaciones de entrada.
 * 
 * @details La función comienza leyendo la cabecera y el número de claves. Luego, deserializa las claves y sus respectivos `ObjID`. Después, lee las subpáginas, creando nuevas páginas y deserializándolas recursivamente si es necesario.
 * 
 * @note Este método es útil para cargar una página previamente serializada desde un archivo o flujo de entrada.
 */
template <typename Trait>
std::istream& CBTreePage<Trait>::Read(std::istream& is) {
    std::string tag;
    is >> tag; //leemos la cabecera "PAGE"
    
    if(tag == "PAGE") {
        is >> m_KeyCount;
        
        // Redimensionar y leer claves
        m_Keys.resize(m_KeyCount);
        for(size_t i = 0; i < m_KeyCount; i++) {
            is >> m_Keys[i].key >> m_Keys[i].ObjID;
            m_Keys[i].UseCounter = 0;
        }
        
        is >> tag; //leemos "ENDKEYS"
        
        // Redimensiona y lee kas subpáginas
        m_SubPages.resize(m_MaxKeys + 2, nullptr);
        for(size_t i = 0; i <= m_KeyCount; i++) {
            std::string childFlag;
            is >> childFlag;
            
            if(childFlag == "1") {
                m_SubPages[i] = new CBTreePage<Trait>(m_MaxKeysForChilds, m_Unique);
                m_SubPages[i]->Read(is);
            }
        }
    }
    return is;
}


//operador <<
/**
 * @brief Sobrecarga del operador de salida `<<` para imprimir una página del árbol B.
 * 
 * Esta sobrecarga del operador `<<` permite imprimir una página del árbol B en un flujo de salida de manera conveniente. Internamente, se llama al método `Print` para realizar la impresión.
 * 
 * @param os Flujo de salida donde se imprimirá la página.
 * @param page La página del árbol B a imprimir.
 * @return El flujo de salida `os` para permitir encadenamiento de operaciones de salida.
 * 
 * @note Esta sobrecarga es útil para imprimir la página del árbol B en la consola o en otros flujos de salida para depuración o visualización.
 */
template <typename Trait>
std::ostream& operator<<(std::ostream& os, CBTreePage<Trait>& page) {
    page.Print(os);
    return os;
}

template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::Remove(const keyType &key, const ObjIDType ObjID)
{
       bt_ErrorCode error = bt_ok;
       size_t pos = binary_search(m_Keys, 0, m_KeyCount, key, m_Compare);
//        if( pos < NumberOfKeys() && key == m_Keys[pos].key /*&& m_Keys[pos].m_ObjID == ObjID*/) // We found it !
        if( pos < NumberOfKeys() && (!m_Compare(key, m_Keys[pos].key) && !m_Compare(m_Keys[pos].key, key)) /*&& m_Keys[pos].m_ObjID == ObjID*/) // We found it !
       {
               // This is a leave: First
               if( !m_SubPages[pos+1] )  // This is a leave ? FIRST CASE !
               {
                       ::remove(m_Keys, pos);
                       NumberOfKeys()--;
                       if( Underflow() )
                               return bt_underflow;
                       return bt_ok;
               }

               // We FOUND IT BUT it is NOT a leave ? SECOND CASE !
               {
                       // Get the first element from right branch
                       ObjectInfo &rFirstFromRight = m_SubPages[pos+1]->GetFirstObjectInfo();
                       // change with a leave
                       swap(m_Keys[pos], rFirstFromRight);
                       // Remove it from this leave

                       //Print(cout);
                       error = m_SubPages[++pos]->Remove(key, ObjID);
               }
       }
       else if( pos == NumberOfKeys() ) // it is not here, go by the last branch
               error = m_SubPages[pos]->Remove(key, ObjID);
//        else if( key <= m_Keys[pos].key ) // = is because identical keys are inserted on left (see Insert)
        else if( !m_Compare(m_Keys[pos].key, key) ) // = is because identical keys are inserted on left (see Insert)
       {        if( m_SubPages[pos] )
                       error = m_SubPages[pos]->Remove(key, ObjID);
               else
                       return bt_nofound;
       }
       if( error == bt_underflow )
       {
               // THIRD CASE: After removing the element we have an underflow
               // Print(cout);
               if( TreatUnderflow(pos) )
                       return bt_ok;
               // FOURTH CASE: it was not possible to redistribute -> Merge
               if( IsRoot() && NumberOfKeys() == 2 )
                       return MergeRoot();
               return Merge(pos);
       }
       if( error == bt_nofound )
               return bt_nofound;
       return bt_ok;
}


template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::Merge(size_t pos)
{
       assert( m_SubPages[pos-1]->NumberOfKeys() +
                       m_SubPages[ pos ]->NumberOfKeys() +
                       m_SubPages[pos+1]->NumberOfKeys() ==
                       3*m_SubPages[ pos ]->MinNumberOfKeys() - 1);

       // FIRST: Put all the elements into a vector
       vector<ObjectInfo> tmpKeys;
       //tmpKeys.resize(nKeys);
       vector<BTPage *>   tmpSubPages;

       BTPage  *pChild1 = m_SubPages[pos-1],
                       *pChild2 = m_SubPages[ pos ],
                       *pChild3 = m_SubPages[pos+1];
       MovePage(pChild1, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[pos-1]);
       MovePage(pChild2, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[ pos ]);
       MovePage(pChild3, tmpKeys, tmpSubPages);
       pChild3->Destroy();;

       // Move 1/2 elements to pChild1
       size_t nKeys = pChild1->GetFreeCells();
       size_t i = 0;
       for(; i < nKeys ; i++ )
       {
               pChild1->m_Keys    [i] = tmpKeys    [i];
               pChild1->m_SubPages[i] = tmpSubPages[i];
               pChild1->NumberOfKeys()++;
       }
       pChild1->m_SubPages[i] = tmpSubPages[i];

       m_Keys    [pos-1] = tmpKeys[i];
       m_SubPages[pos-1] = pChild1;

       ::remove(m_Keys    , pos);
       ::remove(m_SubPages, pos);
       NumberOfKeys()--;

       nKeys = pChild2->GetFreeCells();

       // TODO: #32 change int by size_t
       size_t j = ++i;
       for(i = 0 ; i < nKeys ; i++, j++ )
       {
               pChild2->m_Keys    [i] = tmpKeys    [j];
               pChild2->m_SubPages[i] = tmpSubPages[j];
               pChild2->NumberOfKeys()++;
       }
       pChild2->m_SubPages[i] = tmpSubPages[j];
       m_SubPages[ pos ]          = pChild2;

       if( Underflow() )
               return bt_underflow;
       return bt_ok;
}

template <typename Trait>
bt_ErrorCode CBTreePage<Trait>::MergeRoot()
{
        // TODO: #33 change int by size_t
       size_t pos = 1;
       assert( m_SubPages[pos-1]->NumberOfKeys() +
                       m_SubPages[ pos ]->NumberOfKeys() +
                       m_SubPages[pos+1]->NumberOfKeys() ==
                       3*m_SubPages[ pos ]->MinNumberOfKeys() - 1);

       BTPage  *pChild1 = m_SubPages[pos-1], *pChild2 = m_SubPages[ pos ], *pChild3 = m_SubPages[pos+1];
       // TODO: #34 change int by size_t
       size_t nKeys = pChild1->NumberOfKeys() + pChild2->NumberOfKeys() + pChild3->NumberOfKeys() + 2;

       // FIRST: Put all the elements into a vector
       vector<ObjectInfo> tmpKeys;
       //tmpKeys.resize(nKeys);
       vector<BTPage *>   tmpSubPages;

       MovePage(pChild1, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[pos-1]);
       MovePage(pChild2, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[ pos ]);
       MovePage(pChild3, tmpKeys, tmpSubPages);

       clear();
       size_t i = 0;
       for( ; i < nKeys ; i++ )
       {
               m_Keys    [i] = tmpKeys    [i];
               m_SubPages[i] = tmpSubPages[i];
               NumberOfKeys()++;
       }
       m_SubPages[i] = tmpSubPages[i];

       //Print(cout);
       pChild1->Destroy();
       pChild2->Destroy();
       pChild3->Destroy();

       return bt_rootmerged;
}

template <typename Trait>
typename CBTreePage<Trait>::ObjectInfo &
CBTreePage<Trait>::GetFirstObjectInfo()
{
        if( m_SubPages[0] )
                return m_SubPages[0]->GetFirstObjectInfo();
        return m_Keys[0];
}

template <typename keyType, typename ObjIDType>
void Print(tagObjectInfo<keyType, ObjIDType> &info, size_t level, void *pExtra)
{
       ostream &os = *(ostream *)pExtra;
       for(size_t i = 0; i < level ; i++)
               os << "\t";
       os << info.key << "->" << info.ObjID << "\n";
}

template <typename Trait>
void CBTreePage<Trait>::Print(ostream & os)
{
    ForEach([&os](auto &info, size_t level){
        for(size_t i = 0; i < level; i++)
            os << "\t";
        os << info.key << "-->" << info.ObjID << "\n";
    }, 0);
}

//descomentar para usar el test del move cosntructor
// template <typename Trait>
// size_t CBTreePage<Trait>::GetKeyCount()
// {
//        return m_KeyCount;
// }


template <typename Trait>
void CBTreePage<Trait>::Create()
{
       Reset();
       m_Keys.resize(m_MaxKeys+1);
       m_SubPages.resize(m_MaxKeys+2, NULL);
       m_KeyCount = 0;
       m_MinKeys  = 2 * m_MaxKeys/3;
}

template <typename Trait>
void CBTreePage<Trait>::Reset()
{
        // TODO: #35 change int by size_t
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

template <typename Trait>
CBTreePage<Trait> * CreateBTreeNode (size_t maxKeys, bool unique)
{
       return new CBTreePage<Trait> (maxKeys, unique);
}

template <typename Trait>
void CBTreePage<Trait>::MovePage(BTPage *pChildPage, vector<ObjectInfo> &tmpKeys,vector<BTPage *> &tmpSubPages)
{
        // TODO: #37 change int by size_t
       size_t nKeys = pChildPage->GetNumberOfKeys();
       size_t i = 0;
       for(i = 0; i < nKeys; i++ )
       {
                tmpKeys    .push_back(pChildPage->m_Keys[i]);
                tmpSubPages.push_back(pChildPage->m_SubPages[i]);
       }
       tmpSubPages.push_back(pChildPage->m_SubPages[i]);
       pChildPage->clear();
}

#endif