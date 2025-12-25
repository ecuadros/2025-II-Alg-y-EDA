#ifndef __HEAPPAGE_H__
#define __HEAPPAGE_H__

template <typename Traits>
CHeap<Traits>::CHeap(const CHeap &other) {
    auto lock = std::shared_lock(other.m_Mutex);
    m_vect = other.m_vect;
    m_fCompare = other.m_fCompare;
}

template <typename Traits>
CHeap<Traits>::CHeap(CHeap &&other) {
    auto lock = std::lock_guard(other.m_Mutex);
    m_vect = std::move(other.m_vect);
    m_fCompare = other.m_fCompare;
}

template <typename Traits>
CHeap<Traits>& CHeap<Traits>::operator=(const CHeap &other) {
    if (this != &other) {
        std::lock(m_Mutex, other.m_Mutex);
        auto lk1 = std::lock_guard(m_Mutex, std::adopt_lock);
        auto lk2 = std::lock_guard(other.m_Mutex, std::adopt_lock);
        m_vect = other.m_vect;
        m_fCompare = other.m_fCompare;
    }
    return *this;
}

template <typename Traits>
CHeap<Traits>& CHeap<Traits>::operator=(CHeap &&other) {
    if (this != &other) {
        std::lock(m_Mutex, other.m_Mutex);
        auto lk1 = std::lock_guard(m_Mutex, std::adopt_lock);
        auto lk2 = std::lock_guard(other.m_Mutex, std::adopt_lock);
        m_vect = std::move(other.m_vect);
        m_fCompare = other.m_fCompare;
    }
    return *this;
}

template <typename Traits>
void CHeap<Traits>::AdjustUp(size_t idx) {
    while (idx > 0) {
        size_t parent = (idx - 1) / 2;
        if (!m_fCompare(m_vect[idx], m_vect[parent]))
            break;
        intercambio(m_vect[idx], m_vect[parent]);
        idx = parent;
    }
}

template <typename Traits>
void CHeap<Traits>::Push(const value_type &element) {
    auto lock = std::lock_guard(m_Mutex);
    m_vect.push_back(element);
    AdjustUp(m_vect.size() - 1);
}

template <typename Traits>
void CHeap<Traits>::AdjustDown(size_t idx) {
    size_t count = m_vect.size();
    while (true) {
        size_t left  = 2 * idx + 1;
        size_t right = 2 * idx + 2;
        size_t best  = idx;

        if (left < count && m_fCompare(m_vect[left], m_vect[best]))
            best = left;
        if (right < count && m_fCompare(m_vect[right], m_vect[best]))
            best = right;
        if (best == idx)
            break;

        intercambio(m_vect[idx], m_vect[best]);
        idx = best;
    }
}

template <typename Traits>
bool CHeap<Traits>::Pop(value_type &element) {
    auto lock = std::lock_guard(m_Mutex);
    if (m_vect.empty())
        return false;
    element = m_vect[0];
    m_vect[0] = m_vect.back();
    m_vect.pop_back();
    if (!m_vect.empty())
        AdjustDown(0);
    return true;
}

template <typename Traits>
size_t CHeap<Traits>::size() const {
    auto lock = std::shared_lock(m_Mutex);
    return m_vect.size();
}

template <typename Traits>
void CHeap<Traits>::Write(std::ostream &os) const {
    auto lock = std::shared_lock(m_Mutex);
    os << "HEAP " << m_vect.size() << "\n";
    for (size_t i = 0; i < m_vect.size(); ++i)
        os << m_vect[i] << " ";
}

template <typename Traits>
void CHeap<Traits>::Read(std::istream &is) {
    auto lock = std::lock_guard(m_Mutex);
    m_vect.clear();

    std::string header;
    is >> header;
    if (header != "HEAP")
        return;

    size_t count;
    is >> count;
    if (!is)
        return;

    m_vect.resize(count);
    for (size_t i = 0; i < count; ++i)
        is >> m_vect[i];

    if (!is) {
        m_vect.clear();
        return;
    }

    if (count > 1) {
        for (size_t i = count / 2; i-- > 0; )
            AdjustDown(i);
    }
}

#endif 