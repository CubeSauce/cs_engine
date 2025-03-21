// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/vec3.hpp"
#include "cs/math/mat4.hpp"

struct fquat
{
public:
    static fquat zero_quat;

    vec3 v;
    float w;

public:
    fquat();
    fquat(const vec3& v, float w);
    fquat(const fquat& other);

    mat4 to_mat4() const;
    static fquat from_direction(const vec3& direction);
    static fquat from_euler_angles(const vec3& euler);
    static fquat from_rotation_axis(const vec3& axis, float angle);
    
    fquat mul(const fquat& other) const;
    vec3 mul(const vec3& other) const;

    void normalize();
    fquat normalized() const;

    fquat conjugate() const;

    // Get a vector pointing in the direction of the fquat
    vec3 get_direction(const vec3& forward_axis = vec3(0.0f, 1.0f, 0.0f));

public:
    static fquat from_mat4(const mat4& m);
};

using quat = fquat;

mat4 rotation(const fquat& rotation);

fquat slerp(fquat a, fquat b, float time);

