// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

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
constexpr float operator""deg(long double value) noexcept
{
  return static_cast<float>(MATH_DEG_TO_RAD(value));
}

constexpr float operator""deg(unsigned long long value) noexcept
{
  return static_cast<float>(MATH_DEG_TO_RAD(static_cast<float>(value)));
}

constexpr double operator""ddeg(long double value) noexcept
{
  return MATH_DEG_TO_RAD(value);
}

constexpr double operator""ddeg(unsigned long long value) noexcept
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

template<typename Type_A, typename Type_B>
struct Pair
{
  Type_A a;
  Type_B b;
};
