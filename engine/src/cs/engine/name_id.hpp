// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"

#include <string>
#include <string_view>

constexpr uint32 fnv1a_32_const = 0x811c9dc5ul;
constexpr uint64 fnv1a_64_const = 0xcbf29ce484222325ull;

constexpr uint32_t hash_32_fnv1a_const(const char* const str, const uint32_t value = fnv1a_32_const) noexcept;
constexpr uint64_t hash_64_fnv1a_const(const char* const str, const uint64_t value = fnv1a_64_const) noexcept;

const char* get_hashed_string(uint32 hash);

class Name_Id
{
public:
    uint32 id { 0 };
    std::string_view str { "" };

public:
    Name_Id() = default;
    
    constexpr Name_Id(const char* string) noexcept
    :id(hash_32_fnv1a_const(string)), str(string)
    {

    }

    Name_Id(const std::string& string);

    inline operator uint32() const { return id; }
    inline const char* c_str() const { return str.data(); }
    inline operator const char*() const { return c_str(); }
    inline bool operator==(const Name_Id& other) const { return id == other.id; }

    static Name_Id Empty;

private:
    constexpr Name_Id(uint32 id, const char* string) noexcept
        :id(id), str(string)
    {
    }
};
