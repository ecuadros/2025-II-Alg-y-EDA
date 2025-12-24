#ifndef __HEAP_H__
#define __HEAP_H__

#include <vector>
#include <shared_mutex>
#include <mutex>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <functional>
#include <fstream>

using namespace std;


template <typename Tp>
struct MaxHeapTraits {
    using T = Tp;
    using CompareFn = less<T>;
};

template <typename Tp>
struct MinHeapTraits {
    using T = Tp;
    using CompareFn = greater<T>;
};

template <typename Traits>
class CHeap {
public:
    using value_type = typename Traits::T;
    using CompareFn  = typename Traits::CompareFn;

protected:
    vector<value_type> m_data;
    mutable shared_mutex m_mtx;
    CompareFn          m_comp;

    void heapify_down(size_t i) {
        size_t largest = i;
        size_t left = 2 * i + 1;
        size_t right = 2 * i + 2;
        size_t n = m_data.size();

        if (left < n && m_comp(m_data[largest], m_data[left]))
            largest = left;

        if (right < n && m_comp(m_data[largest], m_data[right]))
            largest = right;

        if (largest != i) {
            std::swap(m_data[i], m_data[largest]);
            heapify_down(largest);
        }
    }

    void heapify_up(size_t i) {
        while (i != 0) {
            size_t parent = (i - 1) / 2;
            if (m_comp(m_data[parent], m_data[i])) {
                std::swap(m_data[i], m_data[parent]);
                i = parent;
            } else {
                break;
            }
        }
    }

    void print_tree(ostream& os, size_t index, int level) const {
        if (index >= m_data.size()) return;

        print_tree(os, 2 * index + 2, level + 1);

        for (auto i = 0; i < level; ++i) os << "\t";
        os << m_data[index] << endl;

        print_tree(os, 2 * index + 1, level + 1);
    }

public:
    CHeap() = default;

    CHeap(const CHeap& other) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_data = other.m_data;
        m_comp = other.m_comp;
    }

    CHeap& operator=(const CHeap& other) {
        if (this != &other) {
            unique_lock<shared_mutex> lockThis(m_mtx, defer_lock);
            shared_lock<shared_mutex> lockOther(other.m_mtx, defer_lock);
            
            std::lock(lockThis, lockOther);
            
            m_data = other.m_data;
            m_comp = other.m_comp;
        }
        return *this;
    }

    CHeap(CHeap&& other) noexcept {
        lock_guard<shared_mutex> lock(other.m_mtx);
        m_data = std::exchange(other.m_data, vector<value_type>{});
        m_comp = std::move(other.m_comp);
    }

    CHeap& operator=(CHeap&& other) noexcept {
        if (this != &other) {
            unique_lock<shared_mutex> lockThis(m_mtx, defer_lock);
            unique_lock<shared_mutex> lockOther(other.m_mtx, defer_lock);
            std::lock(lockThis, lockOther);

            m_data = std::exchange(other.m_data, vector<value_type>{});
            m_comp = std::move(other.m_comp);
        }
        return *this;
    }

    virtual ~CHeap() = default;



    void Push(value_type elem) {
        lock_guard<shared_mutex> lock(m_mtx);
        m_data.push_back(std::move(elem));
        heapify_up(m_data.size() - 1);
    }

    value_type Pop() {
        lock_guard<shared_mutex> lock(m_mtx);
        if (m_data.empty()) {
            throw out_of_range("Heap vacio");
        }
        
        value_type result = std::move(m_data[0]);
        
        m_data[0] = std::move(m_data.back());
        m_data.pop_back();
        
        if (!m_data.empty()) {
            heapify_down(0);
        }
        
        return result;
    }

    value_type Top() const {
        shared_lock<shared_mutex> lock(m_mtx);
        if (m_data.empty()) {
            throw out_of_range("Heap vacio");
        }
        return m_data.front();
    }

    bool Empty() const {
        shared_lock<shared_mutex> lock(m_mtx); 
        return m_data.empty();
    }

    size_t Size() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_data.size();
    }

    void Write(ostream &os) const {
        shared_lock<shared_mutex> lock(m_mtx);
        os << m_data.size() << " ";
        for (const auto& item : m_data) {
            os << item << " ";
        }
    }

    void Print(ostream &os) const {
        shared_lock<shared_mutex> lock(m_mtx);
        print_tree(os, 0, 0);
    }

    void Read(istream &is) {
        lock_guard<shared_mutex> lock(m_mtx);
        size_t size;
        if (is >> size) {
            m_data.clear();
            m_data.reserve(size);
            for (size_t i = 0; i < size; ++i) {
                value_type temp;
                is >> temp;
                m_data.push_back(std::move(temp));
            }
        }
    }
};


template <typename Traits>
ostream & operator<<(ostream &os, const CHeap<Traits> &heap) {
    heap.Print(os);
    return os;
}

template <typename Traits>
istream & operator>>(istream &is, CHeap<Traits> &heap) {
    heap.Read(is);
    return is;
}

#endif // __HEAP_H__