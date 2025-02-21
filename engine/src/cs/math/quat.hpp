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
    static quat from_euler_angles(const vec3& euler);
    static quat from_rotation_axis(const vec3& axis, float angle);
    
    quat mul(const quat& other) const;

    void normalize();
    quat normalized() const;

public:
    static quat from_mat4(const mat4& m);
};

mat4 rotation(const quat& rotation);

quat slerp(quat a, quat b, float time);

