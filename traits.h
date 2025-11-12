#ifndef __TRAITS_H__
#define __TRAITS_H__

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

#endif // __TRAITS_H__