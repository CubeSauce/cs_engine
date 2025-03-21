// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/math/box.hpp"

Box Box::empty_box = Box(vec3::max_float_vector, vec3::min_float_vector);

Box::Box(const vec3& in_min, const vec3& in_max)
    : min(in_min), max(in_max)
{
}

void Box::expand(const vec3& p)
{
    if (p < min)
    {
        min = p;
    }
    else if (p > max)
    {
        max = p;
    }
}

void Box::expand(const Box& other)
{
    expand(other.min);
    expand(other.max);
}

bool Box::intersects(const Box& other) const
{
    return 
        min.x < other.max.x && max.x >= other.min.x &&
        min.y < other.max.y && max.y >= other.min.y &&
        min.z < other.max.z && max.z >= other.min.z;
}

vec3 Box::get_center() const
{
    return (max - min) * 0.5f;
}

vec3 Box::get_extents() const
{
    return max - min;
}

vec3 Box::get_half_extents() const
{
    return get_extents() * 0.5f;
}

void Box::_check_and_fix()
{
    if (min.x > max.x)
    {
        swap(min.x, max.x);
    }

    if (min.y > max.y)
    {
        swap(min.x, max.y);
    }

    if (min.z > max.z)
    {
        swap(min.z, max.z);
    }
}
