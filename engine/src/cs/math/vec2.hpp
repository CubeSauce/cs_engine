// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"

class fvec2
{
public:
    static fvec2 zero_vector;
    static fvec2 one_vector;

public:
    float x, y;

public:
    fvec2() = default;
    fvec2(float v);
    fvec2(float x, float y);
    
    float length_squared() const;
    float length() const;
    fvec2& normalize();
    fvec2 normalized() const;
};

using vec2 = fvec2;

class  ivec2
{
public:
    int32 x, y;

public:
    ivec2() = default;
    ivec2(int32 v);
    ivec2(int32 x, int32 y);
};
