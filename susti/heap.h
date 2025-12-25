#ifndef HEAP_H
#define HEAP_H

#include <vector>
#include <mutex>
#include <shared_mutex>

template <typename T, bool IsMinHeap_P = true>
struct HeapTrait {
    using Type = T;
    inline static bool IsMinHeap = IsMinHeap_P;
};

template <typename Trait>
class Heap {
    typedef typename Trait::Type T;
    inline static bool IsMinHeap = Trait::IsMinHeap;

    static bool Compare(const T& a, const T& b) {
        if (IsMinHeap) return a < b;
        else return a > b;
    }

public:
    Heap();
    ~Heap();

    Heap(const Heap& heap);
    Heap(Heap&& heap) noexcept;

    Heap& operator=(const Heap& heap);
    Heap& operator=(Heap&& heap) noexcept;

    bool IsEmpty() const {
        std::shared_lock<std::shared_mutex> lock(m_Mutex);
        return heapVector.empty();
    }

    size_t GetSize() const {
        std::shared_lock<std::shared_mutex> lock(m_Mutex);
        return heapVector.size();
    }

protected:
    std::vector<T> heapVector;
    mutable std::shared_mutex m_Mutex;
};

template <typename Trait>
Heap<Trait>::Heap() {
}

template <typename Trait>
Heap<Trait>::~Heap() {
}

template <typename Trait>
Heap<Trait>::Heap(const Heap& heap) {
    std::shared_lock<std::shared_mutex> lock(heap.m_Mutex);
    heapVector = heap.heapVector;
}

template <typename Trait>
Heap<Trait>::Heap(Heap&& heap) noexcept {
    std::unique_lock<std::shared_mutex> lock(heap.m_Mutex);
    heapVector.swap(heap.heapVector);
}

template <typename Trait>
Heap<Trait>& Heap<Trait>::operator=(const Heap& heap) {
    std::scoped_lock lock(m_Mutex, heap.m_Mutex);
    heapVector = heap.heapVector;
    return *this;
}

template <typename Trait>
Heap<Trait>& Heap<Trait>::operator=(Heap&& heap) noexcept {
    std::scoped_lock lock(m_Mutex, heap.m_Mutex);
    heapVector.swap(heap.heapVector);
    return *this;
}

#endif
