// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/vec3.hpp"
#include "cs/math/vec4.hpp"

struct  mat4
{
public:
    vec4 columns[4];

public:
	mat4();
	mat4(float v);
	mat4(const vec4& col0, const vec4& col1, const vec4& col2, const vec4& col3);
	mat4(const mat4& other);

    vec4& operator[](int32 index);
    const vec4& operator[](int32 index) const;

	mat4 operator*(const mat4& other) const;

    mat4 &transpose();
    mat4 transposed() const;

    mat4 inverse() const;
};

mat4 translate(mat4& other, const vec3& translation);
mat4 rotate(mat4& other, float angle, const vec3& rotation_axis);
mat4 scale(mat4& other, const vec3& scaling);
mat4 orthographic_matrix(vec4 frustum, float near, float far);
mat4 perspective_matrix(float angle, float aspect_ratio, float near, float far);
mat4 look_at(vec3 const& eye, vec3 const& center, vec3 const& up);
