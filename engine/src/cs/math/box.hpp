// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/vec3.hpp"

struct AABB
{
public:
    static AABB empty_box;

public:
    vec3 min;
    vec3 max;

public:
    AABB() = default;
    AABB(const vec3& in_min, const vec3& in_max);

    void expand(const vec3& p);
    void expand(const AABB& other);

    bool intersects(const AABB& other) const;

    vec3 get_center() const;
    vec3 get_extents() const;
    vec3 get_half_extents() const;

    AABB get_with_offset(const vec3& offset) const;
private:
    void _check_and_fix();
};
