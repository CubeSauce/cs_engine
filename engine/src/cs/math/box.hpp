// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/vec3.hpp"

struct Box
{
public:
    static Box empty_box;

public:
    vec3 min;
    vec3 max;

public:
    Box() = default;
    Box(const vec3& in_min, const vec3& in_max);

    void expand(const vec3& p);
    void expand(const Box& other);

    bool intersects(const Box& other) const;

    vec3 get_center() const;
    vec3 get_extents() const;
    vec3 get_half_extents() const;

private:
    void _check_and_fix();
};
