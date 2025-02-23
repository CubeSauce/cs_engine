// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/math/vec3.hpp"

#include <cmath>

vec3 vec3::right_vector     = vec3(1.0f, 0.0f, 0.0f);
vec3 vec3::forward_vector   = vec3(0.0f, 1.0f, 0.0f);
vec3 vec3::up_vector        = vec3(0.0f, 0.0f, 1.0f);
vec3 vec3::zero_vector      = vec3(0.0f);
vec3 vec3::one_vector       = vec3(1.0f);

vec3::vec3(float v)
    :x(v), y(v), z(v)
{

}

vec3::vec3(float x, float y, float z)
    :x(x), y(y), z(z)
{

}

float vec3::operator[](int32 index) const
{
    if(index < 0 || index > 2)
    {
        return data[0];
    }

    return data[index];
}

float& vec3::operator[](int32 index)
{
    if(index < 0 || index > 2)
    {
        return data[0];
    }

    return data[index];
}

vec3& vec3::operator+=(float v)
{
    x += v;
    y += v;
    z += v;

    return *this;
}

vec3 vec3::operator+(float v) const
{
    return vec3(
        x + v,
        y + v,
        z + v
    );
}

vec3& vec3::operator-=(float v)
{
    x -= v;
    y -= v;
    z -= v;

    return *this;
}

vec3 vec3::operator-(float v) const
{
    return vec3(
        x - v,
        y - v,
        z - v
    );
}

vec3& vec3::operator*=(float v)
{
    x *= v;
    y *= v;
    z *= v;

    return *this;
}

vec3 vec3::operator*(float v) const
{
    return vec3(
        x * v,
        y * v,
        z * v
    );
}

vec3& vec3::operator/=(float v)
{
    x /= v;
    y /= v;
    z /= v;

    return *this;
}

vec3 vec3::operator/(float v) const
{
    return vec3(
        x / v,
        y / v,
        z / v
    );
}

vec3& vec3::operator+=(const vec3& other)
{
    x += other.x;
    y += other.y;
    z += other.z;

    return *this;
}

vec3 vec3::operator+(const vec3& other) const
{
    return vec3(
        x + other.x,
        y + other.y,
        z + other.z
    );
}

vec3& vec3::operator-=(const vec3& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;

    return *this;
}

vec3 vec3::operator-(const vec3& other) const
{
    return vec3(
        x - other.x,
        y - other.y,
        z - other.z
    );
}

vec3& vec3::operator*=(const vec3& other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;

    return *this;
}

vec3 vec3::operator*(const vec3& other) const
{
    return vec3(
        x * other.x,
        y * other.y,
        z * other.z
    );
}


vec3& vec3::operator/=(const vec3& other)
{
    x /= NEAR_ZERO_CHECK(other.x);
    y /= NEAR_ZERO_CHECK(other.y);
    z /= NEAR_ZERO_CHECK(other.z);

    return *this;
}

vec3 vec3::operator/(const vec3& other) const
{
    return vec3(
        x / NEAR_ZERO_CHECK(other.x),
        y / NEAR_ZERO_CHECK(other.y),
        z / NEAR_ZERO_CHECK(other.z)
    );
}

float vec3::length() const
{
    return sqrtf(x * x + y * y + z * z);
}

vec3& vec3::normalize()
{
    float len = length();
    *this /= NEAR_ZERO_CHECK(len);
    return *this;
}

vec3 vec3::normalized() const
{
    float len = NEAR_ZERO_CHECK(length());
    return *this / len;
}

vec3 vec3::cross(const vec3& other) const
{
    return vec3(
        data[1] * other[2] - data[2] * other[1],
        data[2] * other[0] - data[0] * other[2],
        data[0] * other[1] - data[1] * other[0]
    );
}

float vec3::dot(const vec3& other) const
{
    return data[0] * other[0] +
        data[1] * other[1] +
        data[2] * other[2];
}

vec3 sin(const vec3& other)
{
    return vec3(
        sin(other.x),
        sin(other.y),
        sin(other.z)
    );
}

vec3 cos(const vec3& other)
{
    return vec3(
        cos(other.x),
        cos(other.y),
        cos(other.z)
    );
}

ivec3::ivec3(int32 v)
    :   x(v), y(v), z(v)
{

}

ivec3::ivec3(int32 x, int32 y, int32 z)
    :   x(x), y(y), z(z)
{

}