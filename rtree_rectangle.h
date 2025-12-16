#ifndef __RTREE_RECTANGLE_H__
#define __RTREE_RECTANGLE_H__

#include <iostream>

using namespace std;

template <typename Trait>
struct Rectangle
{
    using ElemType = typename Trait::ElemType;

    // arrays to store
    // min and max coords for each dimension
    ElemType m_min[Trait::NumDims];
    ElemType m_max[Trait::NumDims];

    Rectangle() {}

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
