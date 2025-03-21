// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/vec2.hpp"

struct fvec3
{
public:
    union
    {
        float data[3];
        struct {
            float x, y, z;
        };
        struct {
            float r, g, b;
        };
        struct {
            vec2 xy;
            float z;
        };
        struct {
            float x;
            vec2 yz;
        };
    };

    static fvec3 right_vector;
    static fvec3 forward_vector;
    static fvec3 up_vector;

    static fvec3 zero_vector;
    static fvec3 one_vector;
    
    static fvec3 min_float_vector;
    static fvec3 max_float_vector;

public:

    fvec3() = default;
    fvec3(float v);
    fvec3(float x, float y, float z);

    bool nearly_equal(const fvec3& other, float delta = NEARLY_ZERO) const;

    float operator[](int32 index) const;
    float& operator[](int32 index);

    fvec3& operator+=(float v);
    fvec3 operator+(float v) const;
    fvec3& operator-=(float v);
    fvec3 operator-(float v) const;
    fvec3& operator*=(float v);
    fvec3 operator*(float v) const;
    fvec3& operator/=(float v);
    fvec3 operator/(float v) const;
    
    fvec3& operator+=(const fvec3& other);
    fvec3 operator+(const fvec3& other) const;
    fvec3& operator-=(const fvec3& other);
    fvec3 operator-(const fvec3& other) const;
    fvec3& operator*=(const fvec3& other);
    fvec3 operator*(const fvec3& other) const;
    fvec3& operator/=(const fvec3& other);
    fvec3 operator/(const fvec3& other) const;

    fvec3 operator-() const;

    bool operator>(const fvec3& other) const;
    bool operator>=(const fvec3& other) const;
    bool operator<(const fvec3& other) const;
    bool operator<=(const fvec3& other) const;

    float length_squared() const;
    float length() const;
    fvec3& normalize();
    fvec3 normalized() const;
    fvec3 cross(const fvec3& other) const;
    float dot(const fvec3& other) const;

    fvec3 perpendicular() const;
};

fvec3 sin(const fvec3& other);
fvec3 cos(const fvec3& other);

using vec3 = fvec3;

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
