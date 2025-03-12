// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"

struct  vec3
{
public:
    union
    {
        float data[3];
        struct {
            float x, y, z;
        };
    };

    static vec3 right_vector;
    static vec3 forward_vector;
    static vec3 up_vector;

    static vec3 zero_vector;
    static vec3 one_vector;
    
    static vec3 min_float_vector;
    static vec3 max_float_vector;

public:

    vec3(float v = 0.0f);
    vec3(float x, float y, float z);

    bool nearly_equal(const vec3& other, float delta = NEARLY_ZERO) const;

    float operator[](int32 index) const;
    float& operator[](int32 index);

    vec3& operator+=(float v);
    vec3 operator+(float v) const;
    vec3& operator-=(float v);
    vec3 operator-(float v) const;
    vec3& operator*=(float v);
    vec3 operator*(float v) const;
    vec3& operator/=(float v);
    vec3 operator/(float v) const;
    
    vec3& operator+=(const vec3& other);
    vec3 operator+(const vec3& other) const;
    vec3& operator-=(const vec3& other);
    vec3 operator-(const vec3& other) const;
    vec3& operator*=(const vec3& other);
    vec3 operator*(const vec3& other) const;
    vec3& operator/=(const vec3& other);
    vec3 operator/(const vec3& other) const;

    bool operator>(const vec3& other) const;
    bool operator>=(const vec3& other) const;
    bool operator<(const vec3& other) const;
    bool operator<=(const vec3& other) const;

    float length() const;
    vec3& normalize();
    vec3 normalized() const;
    vec3 cross(const vec3& other) const;
    float dot(const vec3& other) const;
};

vec3 sin(const vec3& other);
vec3 cos(const vec3& other);

#define DECOMPOSE_VEC3(v3) v3.x, v3.y, v3.z

struct  ivec3
{
public:
    union
    {
        int32 data[3];
        struct {
            int32 x, y, z;
        };
    };

public:
    ivec3(int32 v = 0);
    ivec3(int32 x, int32 y, int32 z);
};
