#ifndef __AVL_H__
#define __AVL_H__

#include "binarytree.h"

//
// CAVLNode: ahora hereda correctamente de CBinaryTreeNode<Traits>
// y se le agregó un atributo extra (m_balanceFactor).
//
template <typename Traits>
class CAVLNode : public CBinaryTreeNode<Traits> {
public:
    using value_type = typename Traits::T;
    using Node       = CAVLNode<Traits>;

protected:
    int m_balanceFactor = 0; // <-- agregado (no existía en el original)

public:
    // <-- agregado: constructor que llama al de la clase base
    CAVLNode(Node* pParent, value_type data, Ref ref,
             Node* p0 = nullptr, Node* p1 = nullptr)
        : CBinaryTreeNode<Traits>(pParent, data, ref, p0, p1) {}
};

//
// Traits corregidos: ahora usan T y CAVLNode con los traits correctos.
// En el original estaban mal definidos (usaban value_type o T sin traits).
//
template <typename _T>
struct AVLAscTraits {
    using T         = _T;
    using Node      = CAVLNode<AVLAscTraits<T>>;
    using CompareFn = less<T>;
};

template <typename _T>
struct AVLDescTraits {
    using T         = _T;
    using Node      = CAVLNode<AVLDescTraits<T>>;
    using CompareFn = greater<T>;
};

//
// CAVLTree: ahora hereda correctamente de CBinaryTree<Traits>
// y redefine internal_insert() para luego implementar balanceo.
// En el original no heredaba ni tenía override.
//
template <typename Traits>
class CAVLTree : public CBinaryTree<Traits> {
public:
    using Base       = CBinaryTree<Traits>;
    using Node       = typename Traits::Node;
    using value_type = typename Traits::T;
    using CompareFn  = typename Traits::CompareFn;
    using Container  = CAVLTree<Traits>;
    using iterator   = binary_tree_iterator<Container>;

protected:
    //
    // internal_insert sobrescrito:
    // ahora llama al insert base y deja espacio para balancear.
    //
    Node* internal_insert(value_type &elem, Ref ref,
                          Node* pParent, Node*& rpOrigin) override
    {
        Node* newNode = Base::internal_insert(elem, ref, pParent, rpOrigin);
        // TODO: agregar rotaciones y cálculo de balance
        return newNode;
    }

public:
    CAVLTree() : Base() {} // Constructor sin cambios
};

#endif // __AVL_H__
