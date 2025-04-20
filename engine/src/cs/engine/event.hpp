// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/containers/dynamic_array.hpp"

#include <functional>

// Template magic for placeholders from: 
//  https://stackoverflow.com/questions/21192659/variadic-templates-and-stdbind
template<int...> struct int_sequence {};
template<int N, int... Is> struct make_int_sequence
    : make_int_sequence<N-1, N-1, Is...> {};
template<int... Is> struct make_int_sequence<0, Is...>
    : int_sequence<Is...> {};
template<int> // begin with 0 here!
struct placeholder_template
{};

#include <functional>
#include <type_traits>

namespace std
{
    template<int N>
    struct is_placeholder<placeholder_template<N> >
        : integral_constant<int, N+1> // the one is important
    {};
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
        _events.push_back(std::bind(func, obj, make_int_sequence<sizeof...(Args)>{}));
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
