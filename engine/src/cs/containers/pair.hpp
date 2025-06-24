#pragma once

#include <cs/cs.hpp>

template<typename A, typename B>
struct Pair
{
    A a;
    B b;

    inline bool operator==(const Pair& other) const { return a == other.a && b == other.b; }
};
