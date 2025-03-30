// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/math/box.hpp"

AABB AABB::empty_box = AABB(vec3::max_float_vector, vec3::min_float_vector);

AABB::AABB(const vec3& in_min, const vec3& in_max)
    : min(in_min), max(in_max)
{
}

void AABB::expand(const vec3& p)
{
    for (int i = 0; i < 3; ++i)
    {
        if (p[i] < min[i])
        {
            min[i] = p[i];
        }
        if (p[i] > max[i])
        {
            max[i] = p[i];
        }
    }
}

void AABB::expand(const AABB& other)
{
    expand(other.min);
    expand(other.max);
}

bool AABB::intersects(const AABB& other) const
{
    return 
        min.x < other.max.x && max.x >= other.min.x &&
        min.y < other.max.y && max.y >= other.min.y &&
        min.z < other.max.z && max.z >= other.min.z;
}

vec3 AABB::get_center() const
{
    return (min + max) * 0.5f;
}

vec3 AABB::get_extents() const
{
    return max - min;
}

vec3 AABB::get_half_extents() const
{
    return get_extents() * 0.5f;
}

void AABB::_check_and_fix()
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
