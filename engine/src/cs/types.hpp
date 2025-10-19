// CS Util
// Author: matija.martinec@protonmail.com

#pragma once

#include <cfloat>
#include <cstdint>
#include <concepts>

using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

#define NEARLY_ZERO 1e-8f
#define EPSILON 1e-6f
#define KINDA_ZERO 1e-4f
#define MATH_DEG_TO_RAD(x) x * 0.01745329251
#define MATH_RAD_TO_DEG(x) x * 57.2957795131

#pragma warning(disable : 4455)  // Disable warning C4455 - we don't care about future std stuff
constexpr float operator"" _deg(long double value) noexcept
{
  return static_cast<float>(MATH_DEG_TO_RAD(value));
}

constexpr float operator"" _deg(unsigned long long value) noexcept
{
  return static_cast<float>(MATH_DEG_TO_RAD(static_cast<float>(value)));
}

constexpr double operator"" _ddeg(long double value) noexcept
{
  return MATH_DEG_TO_RAD(value);
}

constexpr double operator"" _ddeg(unsigned long long value) noexcept
{
  return MATH_DEG_TO_RAD(static_cast<double>(value));
}
#pragma warning(default : 4455)

template<class T, class U>
concept Derived = std::is_base_of<U, T>::value;

template<typename Type> const char* print_type();

#define DEFINE_PRINT_TYPE(type) \
template<>\
const char* print_type<type>() {\
 	return #type;\
}

#define DECLARE_PRINT_TYPE(type) \
template<>\
const char* print_type<type>();

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

inline bool is_nearly_equal(float a, float b, float delta = EPSILON)
{
  return a < (b - delta) ? false : (a > (b + delta) ? false : true);
}