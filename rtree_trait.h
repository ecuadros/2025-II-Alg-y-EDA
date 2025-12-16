#ifndef __RTREE_TRAITS_H__
#define __RTREE_TRAITS_H__

#include <cstddef>

template<typename _ElemType, size_t _NumDims, typename _ObjIDType>
struct RTreeTrait
{
    using ElemType = _ElemType;
    using ObjIDType = _ObjIDType;
    static const size_t NumDims = _NumDims;
};

#endif // __RTREE_TRAITS_H__
