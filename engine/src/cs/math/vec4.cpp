// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/math/vec4.hpp"

fvec4 fvec4::zero_vector = fvec4(0.0f);
fvec4 fvec4::one_vector = fvec4(1.0f);

fvec4::fvec4(float v)
    :x(v), y(v), z(v), w(v)
{

}
fvec4::fvec4(float x, float y, float z, float w)
    :x(x), y(y), z(z), w(w)
{

}

fvec4::fvec4(const fvec4& other)
    : x(other.x), y(other.y), z(other.z), w(other.w)
{
}

fvec4::fvec4(const vec3& other, float w)
	: x(other.x), y(other.y), z(other.z), w(w)
{
}

fvec4::fvec4(float x, const vec3& other)
    : x(x), y(other.x), z(other.y), w(other.z)
{
}

fvec4 fvec4::operator+(float other) const
{
	return fvec4(x + other, y + other, z + other, w + other);
}


fvec4 fvec4::operator-(float other) const
{
	return fvec4(x - other, y - other, z - other, w - other);
}

fvec4 fvec4::operator*(float other) const
{
	return fvec4(x * other, y * other, z * other, w * other);
}

fvec4 fvec4::operator-(const fvec4& other) const
{
	return fvec4(x - other.x, y - other.y, z - other.z, w - other.w);
}

fvec4 fvec4::operator+(const fvec4& other) const
{
	return fvec4(x + other.x, y + other.y, z + other.z, w + other.w);
}

fvec4 fvec4::operator*(const fvec4& other) const
{
	return fvec4(x * other.x, y * other.y, z * other.z, w * other.w);
}

const float& fvec4::operator[](int32 index) const
{
    if (index < 0 || index > 3)
    {
        return data[0];
    }

    return data[index];
}

float& fvec4::operator[](int32 index)
{
    if (index < 0 || index > 3)
    {
        return data[0];
    }

    return data[index];
}

ivec4::ivec4(int32 v)
    :x(v), y(v), z(v), w(v)
{

}
ivec4::ivec4(int32 x, int32 y, int32 z, int32 w)
    :x(x), y(y), z(z), w(w)
{

}

ivec4::ivec4(const ivec4& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
}

const int32& ivec4::operator[](int32 index) const
{
    if (index < 0 || index > 3)
    {
        return data[0];
    }

    return data[index];
}

int32& ivec4::operator[](int32 index)
{
    if (index < 0 || index > 3)
    {
        return data[0];
    }

    return data[index];
}
