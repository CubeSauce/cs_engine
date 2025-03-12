// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/math/box.hpp"


Box::Box()
{

}

Box::Box(const vec3& in_min, const vec3& in_max)
    : min(in_min), max(in_max)
{
    _check_and_fix();
}

Box::Box(const Box& other)
    : min(other.min), max(other.max)
{
}

Box::Box(Box&& other) noexcept
    : min(other.min), max(other.max)
{
    other.min = vec3::max_float_vector;
    other.max = vec3::min_float_vector;
}

Box& Box::operator=(const Box& other)
{
    min = other.min;
    max = other.max;
    return *this;
}

Box& Box::operator=(Box&& other) noexcept
{
    min = other.min;
    other.min = vec3::max_float_vector;
    max = other.max;
    other.max = vec3::min_float_vector;
    return *this;
}

void Box::expand(const vec3& p)
{
    if (min > p)
    {
        min = p;
    }
    else if (max < p)
    {
        max = p;
    }
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
