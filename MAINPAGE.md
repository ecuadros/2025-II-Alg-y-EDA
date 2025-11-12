# 2025-II-Alg-y-EDA Documentation {#mainpage}

Advanced Algorithms and Data Structures - UNI - 2025-II

## DS

- @ref group_btree "B-Tree"
- @ref group_vector "Vector"
- @ref group_binarytree "Binary Tree"
- @ref group_avl "AVL Tree"
- @ref group_linkedlist "Linked List"
- @ref group_doublelinkedlist "Double Linked List"
- @ref group_utilities "Utilities"

---

## B-Tree {#group_btree}

- `btree.h` - Main B-Tree template class
- `btreepage.h` - Page or node of the B-Tree
- `btree_iterator.h` - Iterator for B-Tree traversal
- `ArbolB.cpp` - Example

---

## Vector {#group_vector}
- `vector.h` - Main vector template class
- `DemoVector.h`
- `DemoVector.cpp` - Example

---

## Binary Tree {#group_binarytree}

- `binarytree.h` - Binary tree template class

---

## AVL Tree {#group_avl}

- `avl.h` - AVL tree template class (extends binary tree)

---

## Linked List {#group_linkedlist}

- `linkedlist.h` - Single linked list template class

---

## Double Linked List {#group_doublelinkedlist}

- `doublelinkedlist.h` - Double linked list template class

---

## Utilities {#group_utilities}

- `traits.h` - Trait definitions (AscendingTrait, DescendingTrait, ListTrait)
- `types.h`
- `util.h`
- `foreach.h` - Generic foreach implementation for containers

---

## Build

```bash
make all
```

## Generate docs

```bash
doxygen
```