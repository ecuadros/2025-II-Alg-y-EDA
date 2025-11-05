#ifndef __TRAITS_H__
#define __TRAITS_H__
#include <functional>

template <typename T, typename _Func>
struct ListTrait{
    using value_type = T;
    using Func       = _Func;
};

template <typename T>
struct AscendingTrait :
    public ListTrait<T, std::less<T> >{
};

template <typename T>
struct DescendingTrait :
    public ListTrait<T, std::greater<T> >{
};

template <typename _keyType, typename _ObjIDType, typename _CompareFn>
struct BTreeTraitBase
{
    using keyType    = _keyType;
    using ObjIDType  = _ObjIDType;
    using CompareFn  = _CompareFn;
};

template <typename _keyType, typename _ObjIDType>
struct BTreeAscendingTrait :
    public BTreeTraitBase<_keyType, _ObjIDType, std::less<_keyType> >
{
};

template <typename _keyType, typename _ObjIDType>
struct BTreeDescendingTrait :
    public BTreeTraitBase<_keyType, _ObjIDType, std::greater<_keyType> >
{
};

#endif // __TRAITS_H__