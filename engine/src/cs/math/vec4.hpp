// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/vec3.hpp"

struct fvec4
{
public:
    static fvec4 zero_vector;
    static fvec4 one_vector;

    union
    {
        float data[4];
        struct 
        {
            union 
            {
                struct 
                {
                    float x, y, z;
                };
                fvec3 xyz;
                struct 
                {
                    float r, g, b;
                };
                fvec3 rgb;
            };
            union
            {
                float w;
                float a;
            };            
        };
        struct
        {
            fvec2 xy, zw;
        };
    };

public:
    fvec4() = default;
    fvec4(float v);
    fvec4(float x, float y, float z, float w);
	fvec4(const fvec4& other);
	fvec4(const vec3& other, float w);
	fvec4(float x, const vec3& other);

    const float& operator[](int32 index) const;
	float& operator[](int32 index);

	fvec4 operator+(float other) const;
	fvec4 operator-(float other) const;
	fvec4 operator*(float other) const;
	fvec4 operator+(const fvec4& other) const;
	fvec4 operator-(const fvec4& other) const;
	fvec4 operator*(const fvec4& other) const;

};

using vec4 = fvec4;

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
