#ifndef _TRAITS_H_
#define _TRAITS_H_

template <typename _T, typename _Ref>
struct RTreeTrait{
    using  T          = _T;
    using  Ref        = _Ref;
    using  size_t M   =  10;
    using  size_t m   =  6;
    using  size_t DIM =  2;
};

template <typename _T>
struct RTrait{
    using  Ref       = _T;
    using  size_t M  =  20;
    using  size_t m  =  0.4*M;
};

#endif // _TRAITS_H_ //