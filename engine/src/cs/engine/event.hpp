// CS Engine
// Author: matija.martinec@protonmail.com
// <script src="https://gist.github.com/engelmarkus/fc1678adbed1b630584c90219f77eb48.js"></script>
#pragma once

#include "cs/cs.hpp"
#include "cs/containers/dynamic_array.hpp"

#include <functional>
#include <type_traits>
#include <utility>
#include <iostream>

template <int>
struct variadic_placeholder {};

namespace std {
    template <int N>
    struct is_placeholder<variadic_placeholder<N>>
        : integral_constant<int, N + 1>
    {
    };
}


template <typename Ret, typename Class, typename... Args, size_t... Is, typename... Args2>
auto bind(std::index_sequence<Is...>, Ret (Class::*fptr)(Args...), Args2&&... args) {
    return std::bind(fptr, std::forward<Args2>(args)..., variadic_placeholder<Is>{}...);
}

template <typename Ret, typename Class, typename... Args, typename... Args2>
auto bind(Ret (Class::*fptr)(Args...), Args2&&... args) {
    return bind(std::make_index_sequence<sizeof...(Args) - sizeof...(Args2) + 1>{}, fptr, std::forward<Args2>(args)...);
}


template<class...Args>
class Event
{
public:
    void bind(std::function<void(Args...)> func)
    {
        _events.push_back(func);
    }
    
    template<typename T, typename F>
    void bind(T *obj, F func)
    {
        _events.push_back(::bind(func, obj));
    }

    void broadcast(Args...args)
    {
        for (auto event : _events)
        {
            if (event)
            {
                event(args...);
            }
        }
    }

protected:
    Dynamic_Array<std::function<void(Args...)>> _events;
};
