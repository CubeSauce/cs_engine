// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"

#include <string>
#include <string_view>

constexpr uint32 fnv1a_32_const = 0x811c9dc5ul;
constexpr uint64 fnv1a_64_const = 0xcbf29ce484222325ull;
constexpr uint32 prime_32_const = 0x1000193;
constexpr uint64 prime_64_const = 0x100000001b3;

constexpr uint32 hash_32_fnv1a_const(const char* const str, const uint32 value = fnv1a_32_const) noexcept
{
    return (str[0] == '\0') ? value : hash_32_fnv1a_const(&str[1], (value ^ uint32_t((uint8_t)str[0])) * prime_32_const);
}

constexpr uint64 hash_64_fnv1a_const(const char* const str, const uint64 value = fnv1a_64_const) noexcept
{
    return (str[0] == '\0') ? value : hash_64_fnv1a_const(&str[1], (value ^ uint64_t((uint8_t)str[0])) * prime_64_const);
}

const char* get_hashed_string(uint32 hash);

class Name_Id
{
public:
    uint32 id { 0 };
    std::string_view str { "" };

public:
    Name_Id() = default;

    constexpr Name_Id(uint32 id) noexcept
    :id(id)
    {
    }

    constexpr Name_Id(const char* string) noexcept
    :id(hash_32_fnv1a_const(string)), str(string)
    {

    }

    inline constexpr operator uint32() const { return id; }
    inline constexpr const char* c_str() const { return str.data(); }
    inline constexpr operator const char*() const { return c_str(); }
    inline constexpr bool operator==(const Name_Id& other) const { return id == other.id; }

private:
    constexpr Name_Id(uint32 id, const char* string) noexcept
        :id(id), str(string)
    {
    }
};

static constexpr Name_Id empty_name_id;

namespace std 
{
    template<>
    struct hash<Name_Id> 
    {
        std::size_t operator()(const Name_Id& id) const noexcept 
        {
            return id.id;
        }
    };
}
