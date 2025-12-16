#ifndef __RTREE_RECTANGLE_H__
#define __RTREE_RECTANGLE_H__

#include <iostream>
#include <limits> // infinity

using namespace std;

template <typename Trait>
struct Rectangle
{
    using ElemType = typename Trait::ElemType;

    // arrays to store
    // min and max coords for each dimension
    ElemType m_min[Trait::NumDims];
    ElemType m_max[Trait::NumDims];

    Rectangle() {
        for (size_t i = 0; i < Trait::NumDims; ++i) {
            m_min[i] = numeric_limits<ElemType>::max();     // +infinity
            m_max[i] = numeric_limits<ElemType>::lowest();  // -infinity
        }
    }

    // to set manually
    void Set(const ElemType* min, const ElemType* max)
    {
        for(size_t i=0; i<Trait::NumDims; ++i)
        {
            m_min[i] = min[i];
            m_max[i] = max[i];
        }
    }

    ElemType Area() const
    {
        ElemType area = 1;
        for(size_t i=0; i<Trait::NumDims; ++i)
        {
            area *= (m_max[i] - m_min[i]);
        }
        return area;
    }

    bool Intersects(const Rectangle& other) const
    {
        for(size_t i=0; i<Trait::NumDims; ++i)
        {
            if(m_max[i] < other.m_min[i] || m_min[i] > other.m_max[i])
            {
                return false;
            }
        }
        return true;
    }

    void Union(const Rectangle& other)
    {
        for(size_t i=0; i<Trait::NumDims; ++i)
        {
            if(other.m_min[i] < m_min[i])
                m_min[i] = other.m_min[i];
            if(other.m_max[i] > m_max[i])
                m_max[i] = other.m_max[i];
        }
    }

    Rectangle& Union_(const Rectangle& other)
    {
        Union(other);
        return *this;
    }

    // print
    void Print(ostream& os)
    {
        os << "[";
        for(size_t i=0; i<Trait::NumDims; ++i)
        {
            os << "(" << (size_t)m_min[i] << ", " << (size_t)m_max[i] << ")";
            if(i < Trait::NumDims - 1)
                os << ", ";
        }
        os << "]";
    }

    // Format: [(min, max), (min, max), ...]
    bool Read(istream& is)
    {
        char bracket, paren, comma;
        ElemType minVal, maxVal;

        is >> bracket;  // '['

        for (size_t i = 0; i < Trait::NumDims; ++i)
        {
            is >> paren;    // '('
            is >> minVal;
            is >> comma;    // ','
            is >> maxVal;
            is >> paren;    // ')'

            m_min[i] = minVal;
            m_max[i] = maxVal;

            if (i < Trait::NumDims - 1)
            {
                is >> comma;  // ','
            }
        }

        is >> bracket;  // ']'

        return is.good();
    }

    // ascii print with | - separators
    void AsciiPrint() const
    {
        for(size_t i=0; i<Trait::NumDims; ++i)
        {
            cout << "| " << m_min[i] << " - " << m_max[i] << " ";
        }
        cout << "|" << endl;
    }
};

#endif // __RTREE_RECTANGLE_H__
