// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/containers/dynamic_array.hpp"

#include <functional>

// Currently supports up to 6 arguments
template<int32 n, class...Args>
class Event
{
public:
    void bind(std::function<void(Args...)> func)
    {
        _events.add(func);
    }

    template<typename T, typename F>
    void bind(T *obj, F func)
    {
        if (N == 0)
        {
            _events.add(std::bind(func, obj));
        }
        else
        {
            _events.add(std::bind(func, obj, _generate_placeholders<N>()));
        }
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

private:
    template <std::size_t... S>
    auto _generate_placeholders(std::index_sequence<S...>) {
        return std::make_tuple(_Ph<S>...);
    }
};
