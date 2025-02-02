// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"

class  vec2
{
public:
    float x, y;

public:
    vec2(float v = 0.0f);
    vec2(float x, float y);
};

#define DECOMPOSE_VEC2(v2) v2.x, v2.y

class  ivec2
{
public:
    int32 x, y;

public:
    ivec2(int32 v = 0);
    ivec2(int32 x, int32 y);
};

#define DECOMPOSE_IVEC2(iv2) iv2.x, iv2.y
