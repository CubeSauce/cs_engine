// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/math/vec2.hpp"

fvec2 fvec2::zero_vector = fvec2(0.0f);
fvec2 fvec2::one_vector = fvec2(1.0f);

fvec2::fvec2(float v)
    : x(v), y(v)
{

}

fvec2::fvec2(float x, float y)
    : x(x), y(y)
{

}

float vec2::length_squared() const
{
    return x*x + y*y;
}

float vec2::length() const
{
    return sqrtf(length_squared());
}

vec2& vec2::normalize()
{
    const float len = length();
    x /= len;
    y /= len;
    return *this;
}

vec2 vec2::normalized() const
{
    const float len = length();
    return { x / len, y / len };
}


ivec2::ivec2(int32 v)
    : x(v), y(v)
{

}

ivec2::ivec2(int32 x, int32 y)
    : x(x), y(y)
{

}