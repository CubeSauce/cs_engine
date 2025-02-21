// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"

#include "cs/math/vec2.hpp"
#include "cs/math/vec3.hpp"
#include "cs/math/vec4.hpp"
#include "cs/math/mat4.hpp"
#include "cs/math/quat.hpp"

template<typename Type>
inline Type clamp(Type x, Type min, Type max) { return x <= min ? min : x >= max ? max : x; }

template<typename Type>
inline Type lerp(Type a, Type b, float time) { return (1 - time) * a + time * b; }

inline float min_abs(float a, float b) { return fabs(a) < fabs(b) ? a : b; }
inline float max_abs(float a, float b) { return fabs(a) > fabs(b) ? a : b; }
