#ifndef __HEAP_H__
#define __HEAP_H__

#include <shared_mutex>
#include <mutex>
#include <iostream>
#include <string>
#include <vector>
#include "traits.h"
#include "util.h"

template <typename Traits>
class CHeap {
public:
    using value_type = typename Traits::value_type;
    using Func       = typename Traits::Func;

private:
    std::vector<value_type> m_vect;
    Func        m_fCompare;
    mutable std::shared_mutex m_Mutex;

public:
    CHeap() = default;
    CHeap(const CHeap &other);
    CHeap(CHeap &&other);
    ~CHeap() {
        std::lock_guard<std::shared_mutex> lock(m_Mutex);
    }

    CHeap& operator=(const CHeap &other);
    CHeap& operator=(CHeap &&other);

    void Push(const value_type &element);
    bool Pop(value_type &element);
    size_t size() const;

    void Write(std::ostream &os) const;
    void Read(std::istream &is);

private:
    void AdjustUp(size_t idx);
    void AdjustDown(size_t idx);
};

#include "heappage.h"
#endif
