// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/math/vec3.hpp"

#include <cmath>
#include <limits>

fvec3 fvec3::right_vector     = fvec3(1.0f, 0.0f, 0.0f);
fvec3 fvec3::forward_vector   = fvec3(0.0f, 1.0f, 0.0f);
fvec3 fvec3::up_vector        = fvec3(0.0f, 0.0f, 1.0f);
fvec3 fvec3::zero_vector      = fvec3(0.0f);
fvec3 fvec3::one_vector       = fvec3(1.0f);
fvec3 fvec3::min_float_vector = fvec3(-FLT_MAX);
fvec3 fvec3::max_float_vector = fvec3(FLT_MAX);

fvec3::fvec3(float v)
    :x(v), y(v), z(v)
{

}

fvec3::fvec3(float x, float y, float z)
    :x(x), y(y), z(z)
{

}

bool fvec3::nearly_equal(const fvec3& other, float delta) const
{
    return 
        fabs(x - other.x) < delta &&
        fabs(y - other.y) < delta &&
        fabs(z - other.z) < delta;
}

float fvec3::operator[](int32 index) const
{
    if(index < 0 || index > 2)
    {
        return data[0];
    }

    return data[index];
}

float& fvec3::operator[](int32 index)
{
    if(index < 0 || index > 2)
    {
        return data[0];
    }

    return data[index];
}

fvec3& fvec3::operator+=(float v)
{
    x += v;
    y += v;
    z += v;

    return *this;
}

fvec3 fvec3::operator+(float v) const
{
    return fvec3(
        x + v,
        y + v,
        z + v
    );
}

fvec3& fvec3::operator-=(float v)
{
    x -= v;
    y -= v;
    z -= v;

    return *this;
}

fvec3 fvec3::operator-(float v) const
{
    return fvec3(
        x - v,
        y - v,
        z - v
    );
}

fvec3& fvec3::operator*=(float v)
{
    x *= v;
    y *= v;
    z *= v;

    return *this;
}

fvec3 fvec3::operator*(float v) const
{
    return fvec3(
        x * v,
        y * v,
        z * v
    );
}

fvec3& fvec3::operator/=(float v)
{
    x /= v;
    y /= v;
    z /= v;

    return *this;
}

fvec3 fvec3::operator/(float v) const
{
    return fvec3(
        x / v,
        y / v,
        z / v
    );
}

fvec3& fvec3::operator+=(const fvec3& other)
{
    x += other.x;
    y += other.y;
    z += other.z;

    return *this;
}

fvec3 fvec3::operator+(const fvec3& other) const
{
    return fvec3(
        x + other.x,
        y + other.y,
        z + other.z
    );
}

fvec3& fvec3::operator-=(const fvec3& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;

    return *this;
}

fvec3 fvec3::operator-(const fvec3& other) const
{
    return fvec3(
        x - other.x,
        y - other.y,
        z - other.z
    );
}

fvec3& fvec3::operator*=(const fvec3& other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;

    return *this;
}

fvec3 fvec3::operator*(const fvec3& other) const
{
    return fvec3(
        x * other.x,
        y * other.y,
        z * other.z
    );
}


fvec3& fvec3::operator/=(const fvec3& other)
{
    x /= other.x;
    y /= other.y;
    z /= other.z;

    return *this;
}

fvec3 fvec3::operator/(const fvec3& other) const
{
    return fvec3(
        x / other.x,
        y / other.y,
        z / other.z
    );
}

fvec3 fvec3::operator-() const
{
    return fvec3(-x, -y, -z);
}

bool fvec3::operator>(const fvec3& other) const
{
    return x > other.x && y > other.y && z > other.z;
}

bool fvec3::operator>=(const fvec3& other) const
{
    return x >= other.x && y >= other.y && z >= other.z;
}

bool fvec3::operator<(const fvec3& other) const
{
    return x < other.x && y < other.y && z < other.z;
}

bool fvec3::operator<=(const fvec3& other) const
{
    return x <= other.x && y <= other.y && z <= other.z;
}

float fvec3::length_squared() const
{
    return x * x + y * y + z * z;
}

float fvec3::length() const
{
    return sqrtf(length_squared());
}

fvec3& fvec3::normalize()
{
    float len = length();
    *this /= len;
    return *this;
}

fvec3 fvec3::normalized() const
{
    float len = length();
    return *this / len;
}

fvec3 fvec3::cross(const fvec3& other) const
{
    return fvec3(
        data[1] * other[2] - data[2] * other[1],
        data[2] * other[0] - data[0] * other[2],
        data[0] * other[1] - data[1] * other[0]
    );
}

float fvec3::dot(const fvec3& other) const
{
    return data[0] * other[0] +
        data[1] * other[1] +
        data[2] * other[2];
}


fvec3 sin(const fvec3& other)
{
    return fvec3(
        sin(other.x),
        sin(other.y),
        sin(other.z)
    );
}

fvec3 cos(const fvec3& other)
{
    return fvec3(
        cos(other.x),
        cos(other.y),
        cos(other.z)
    );
}

fvec3 fvec3::perpendicular() const
{
    // Choose the smallest component to avoid zero vectors
    if (fabs(x) > fabs(z)) 
    {
        return fvec3(-y, x, 0).normalized();
    } 
    else
    {
        return fvec3(0, -z, y).normalized();
    }
}

ivec3::ivec3(int32 v)
    :   x(v), y(v), z(v)
{

}

ivec3::ivec3(int32 x, int32 y, int32 z)
    :   x(x), y(y), z(z)
{

}