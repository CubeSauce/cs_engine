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

template<typename Type>
struct Pair
{
  Type a, b;
};