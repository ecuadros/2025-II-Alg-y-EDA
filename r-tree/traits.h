#ifndef _TRAITS_H_
#define _TRAITS_H_

/**
 * @brief Traits structure template
 * Users must specialize this for their types
 */
template<typename T, size_t DIM, typename Ref>
struct RTreeTraits {
    using value_type = T;
    using RefType = Ref;
    static constexpr size_t DIM = DIM;
    static constexpr size_t M = 50;  
    static constexpr size_t m = 20;  
};

template <typename _T>
struct RTrait{
    using  Ref       = _T;
    using  size_t M  =  20;
    using  size_t m  =  0.4*M;
};

#endif // _TRAITS_H_ //