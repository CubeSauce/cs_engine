// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/engine/name_id.hpp"
#include "cs/containers/hash_table.hpp"

#include <cstring>
#include <cassert>

constexpr uint32 prime_32_const = 0x1000193;
constexpr uint64 prime_64_const = 0x100000001b3;

constexpr uint32 hash_32_fnv1a_const(const char* const str, const uint32 value) noexcept
{
    return (str[0] == '\0') ? value : hash_32_fnv1a_const(&str[1], (value ^ uint32_t((uint8_t)str[0])) * prime_32_const);
}

constexpr uint64 hash_64_fnv1a_const(const char* const str, const uint64 value) noexcept
{
    return (str[0] == '\0') ? value : hash_64_fnv1a_const(&str[1], (value ^ uint64_t((uint8_t)str[0])) * prime_64_const);
}

Name_Id Name_Id::Empty = Name_Id(0, "");

Name_Id::Name_Id(const std::string& string)
    :id(hash_32_fnv1a_const(string.c_str())), str(string)
{

}
