#include "RTree.h"

int main()
{
    CRTree<RTreeTraits2D> t;

    using rect_2 = Rect<float, 2>;

    t.insert(rect_2({0, 0}, {2, 2}), 10);
    t.insert(rect_2({5, 5}, {6, 6}), 20);
    t.insert(rect_2({1, 1}, {3, 3}), 30);

    auto res = t.search(rect_2({1, 1}, {2.5f, 2.5f}));
    std::cout << "query results:\n";
    for (auto id : res)
        std::cout << "  " << id << "\n";

    t.remove(rect_2({1, 1}, {3, 3}), 30);

    if (!t.write_to_file("rtree_saved.txt"))
    {
        return 1;
    }

    CRTree<RTreeTraits2D> t2;
    if (!t2.read_from_file("rtree_saved.txt"))
    {
        return 1;
    }

    auto res2 = t2.search(rect_2({-1, -1}, {10, 10}));
    std::cout << "after reload:\n";
    for (auto id : res2)
        std::cout << "  " << id << "\n";

    return 0;
}
