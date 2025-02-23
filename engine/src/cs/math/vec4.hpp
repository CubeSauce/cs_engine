// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/vec3.hpp"

struct  vec4
{
public:
    static vec4 zero_vector;
    static vec4 one_vector;

    union
    {
        float data[4];
        struct {
            union {
                struct {
                    float x, y, z;
                };
                vec3 xyz;
            };
            float w;
        };
    };

public:
    vec4(float v = float(0.0));
    vec4(float x, float y, float z, float w);
	vec4(const vec4& other);
	vec4(const vec3& other, float w);
	vec4(float x, const vec3& other);

    const float& operator[](int32 index) const;
	float& operator[](int32 index);

	vec4 operator+(float other) const;
	vec4 operator-(float other) const;
	vec4 operator*(float other) const;
	vec4 operator+(const vec4& other) const;
	vec4 operator-(const vec4& other) const;
	vec4 operator*(const vec4& other) const;

};

struct  ivec4
{
public:
    union
    {
        int32 data[4];
        struct {
            int32 x, y, z, w;
        };
    };

public:
    ivec4(int32 v = 0);
    ivec4(int32 x, int32 y, int32 z, int32 w);
    ivec4(const ivec4& other);

    const int32& operator[](int32 index) const;
    int32& operator[](int32 index);
};
