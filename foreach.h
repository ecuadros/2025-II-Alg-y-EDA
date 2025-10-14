#ifndef __FOREACH_H__
#define __FOREACH_H__
#include <iostream>
#include <functional>

template <typename Iterator, typename Function >
void foreach(Iterator begin, Iterator end, Function func){
    auto iter = begin;
    for(; iter != end ; ++iter )
        func( *iter );
}

// V1.0
// template <typename Container, typename Func>
// void foreach(Container &container, Func fn){
//     // typename Container::iterator iter = container.begin();
//     auto iter = container.begin();
//     for(; iter != container.end() ; iter++ )
//         fn( *iter );
// }

// V1.1
template <typename Container, typename Function>
void foreach(Container &container, Function func){
    foreach(container.begin(), container.end(), func);
}

// Variadic templates
template<typename Iterator, typename Function, typename... Args>
void foreach (Iterator begin, Iterator end,
              Function func, Args const&... args)
{   auto iter = begin;
    for (; begin != end; ++iter)
        std::invoke(func, args..., *iter);
}

// template<typename Callable, typename... Args>
// decltype(auto) call(Callable op, Args&&... args)
// {
//     if constexpr(is_void_v<invoke_result_t<Callable, Args...>>)
//     { std::cout << "Function is returning: void!" << std::endl;
//       invoke(forward<Callable>(op), forward<Args>(args)...);
//       //...  // do something before we return
//       return;
//     }
//     else // return type is not void:
//     { auto ret = invoke(forward<Callable>(op), forward<Args>(args)...);
//       std::cout << "Function is returning: " << type_name<decltype(ret)>() << std::endl;
//       //...  // do something (with ret) before we return
//       return ret;
//     }
// }

#endif // __FOREACH_H__