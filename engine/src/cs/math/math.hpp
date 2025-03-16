// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"

#include "cs/math/vec2.hpp"
#include "cs/math/vec3.hpp"
#include "cs/math/vec4.hpp"
#include "cs/math/mat4.hpp"
#include "cs/math/quat.hpp"
#include "cs/math/box.hpp"

#include <cmath>

template<typename Type>
inline Type clamp(Type x, Type min, Type max) { return x <= min ? min : x >= max ? max : x; }

template<typename Type>
inline Type lerp(Type a, Type b, float time) { return (1 - time) * a + time * b; }

inline float min_abs(float a, float b) { return fabs(a) < fabs(b) ? a : b; }
inline float max_abs(float a, float b) { return fabs(a) > fabs(b) ? a : b; }

// Maps x from range [in_min, in_max] to range [out_min, out_max] 
inline float map(float x, float in_min, float in_max, float out_min, float out_max) 
{ 
    return (((x - in_min) * (out_max - out_min)) / (in_max - in_min)) + out_min;
}

inline bool is_nearly_equal(float a, float b, float delta = NEARLY_ZERO)
{
    return a < (b - delta) ? false : (a > (b + delta) ? false : true);
}
