// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/math/vec2.hpp"

vec2 vec2::zero_vector = vec2(0.0f);
vec2 vec2::one_vector = vec2(1.0f);

vec2::vec2(float v)
    : x(v), y(v)
{

}

vec2::vec2(float x, float y)
    : x(x), y(y)
{

}

ivec2::ivec2(int32 v)
    : x(v), y(v)
{

}

ivec2::ivec2(int32 x, int32 y)
    : x(x), y(y)
{

}