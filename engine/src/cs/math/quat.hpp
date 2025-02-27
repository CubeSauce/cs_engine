// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/vec3.hpp"
#include "cs/math/mat4.hpp"

struct  quat
{
public:
    static quat zero_quat;

    vec3 v;
    float w;

public:
    quat();
    quat(const vec3& v, float w);
    quat(const quat& other);

    mat4 to_mat4() const;
    static quat from_direction(const vec3& direction);
    static quat from_euler_angles(const vec3& euler);
    static quat from_rotation_axis(const vec3& axis, float angle);
    
    quat mul(const quat& other) const;
    vec3 mul(const vec3& other) const;

    void normalize();
    quat normalized() const;

    quat conjugate() const;

    // Get a vector pointing in the direction of the quat
    vec3 get_direction(const vec3& forward_axis = vec3(0.0f, 1.0f, 0.0f));

public:
    static quat from_mat4(const mat4& m);
};

mat4 rotation(const quat& rotation);

quat slerp(quat a, quat b, float time);

