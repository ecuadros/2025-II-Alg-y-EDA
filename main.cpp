#include <iostream>
#include <vector>
#include "btree.h"

using Trait = BTreeTrait<int, long>;
static void Line(){ std::cout << "--------------------------------------\n"; }

static void print_vec(const std::vector<int>& v){
    for (auto x : v) std::cout << x << ' ';
    std::cout << '\n';
}

int main() {
    BTree<Trait> t(3, true);

    // casos borde: árbol vacío
    assert(t.begin()  == t.end());
    assert(t.rbegin() == t.rend());

    // inserta elementos
    int keys[] = {40,70,10,20,30,50,60,80,90};
    long id = 1;
    for (int k : keys) t.Insert(k, id++);

    // forward iteration (in-order)
    std::vector<int> fwd;
    for (auto it = t.begin(); it != t.end(); ++it)
        fwd.push_back(it->key);

    std::cout << "Forward:  "; print_vec(fwd);
    assert(std::is_sorted(fwd.begin(), fwd.end()));

    //reverse iteration (reverse in-order)
    std::vector<int> rev;
    for (auto it = t.rbegin(); it != t.rend(); ++it)
        rev.push_back(it->key);

    std::cout << "Reverse:  "; print_vec(rev);
    std::vector<int> fwd_reversed = fwd;
    std::reverse(fwd_reversed.begin(), fwd_reversed.end());
    assert(rev == fwd_reversed);

    BTree<Trait> one(3, true);
    one.Insert(42, 1);
    auto it1 = one.begin();
    assert(it1 != one.end() && it1->key == 42);
    ++it1;
    assert(it1 == one.end());

    auto rit1 = one.rbegin();
    assert(rit1 != one.rend() && rit1->key == 42);
    ++rit1;
    assert(rit1 == one.rend());

    Line();
    std::cout << "OK: iteradores forward/reverse verificados.\n";
    return 0;
}
