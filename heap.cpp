#include <cassert>
#include <fstream>
#include <thread>
#include "heap.h"
#include "traits.h"

int main() {
    CHeap<MinHeapTrait<int>> hmin;
    int a = 5, b = 3, c = 7;
    hmin.Push(a);
    hmin.Push(b);
    hmin.Push(c);

    CHeap<MinHeapTrait<int>> hcopy(hmin);
    CHeap<MinHeapTrait<int>> hmove(std::move(hcopy));
    assert(hcopy.size() == 0);

    std::ofstream out("heap.txt");
    hmove.Write(out);
    out.close();

    std::ifstream in("heap.txt");
    CHeap<MinHeapTrait<int>> hread;
    hread.Read(in);
    in.close();
    int x;
    assert(hread.Pop(x) && x == 3);


    CHeap<MaxHeapTrait<int>> hmax;
    hmax.Push(a);
    hmax.Push(b);
    hmax.Push(c);
    assert(hmax.Pop(x) && x == 7);

    CHeap<MinHeapTrait<int>> hcon;
    int v1 = 4, v2 = 1;
    std::thread t1([&]() { hcon.Push(v1); });
    std::thread t2([&]() { hcon.Push(v2); });
    t1.join();
    t2.join();
    assert(hcon.size() == 2);

    return 0;
}
