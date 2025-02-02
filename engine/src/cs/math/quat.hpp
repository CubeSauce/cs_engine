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

    float x, y, z, w;

public:
    quat();
    quat(float x, float y, float z, float w);
    quat(const quat& other);

    mat4 to_mat4() const;
    static quat from_euler_angles(const vec3& euler);

public:
    static quat from_mat4(const mat4& m);
};

mat4 rotation(const quat& rotation);

quat slerp(quat a, quat b, float time);

