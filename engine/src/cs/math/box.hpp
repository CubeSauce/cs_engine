// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/vec3.hpp"

struct Box
{
public:
    vec3 min { vec3::max_float_vector };
    vec3 max { vec3::min_float_vector };

public:
    Box();
    Box(const vec3& in_min, const vec3& in_max);

    Box(const Box& other);
    Box(Box&& other) noexcept;
    
    Box& operator=(const Box& other);
    Box& operator=(Box&& other) noexcept;

    void expand(const vec3& p);

    bool intersects(const Box& other) const;

    vec3 get_min() const;
    vec3 get_max() const;
    vec3 get_center() const;
    vec3 get_extents() const;
    vec3 get_half_extents() const;

private:
    void _check_and_fix();
};
