// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"

uint32 get_hash(const char* str, size_t _size = -1);
uint32 operator ""_hashed(const char* str, size_t _size);

const char* get_hashed_string(uint32 hash);

class Name_Id
{
public:
    uint32 id;
    const char* str;

public:
    Name_Id();
    Name_Id(const char* string);

    inline operator uint32() const { return id; }
    inline operator const char*() const { return str; }
    inline bool operator==(const Name_Id& other) const { return id == other.id; }

    static Name_Id Empty;
};

#ifndef SID
#define SID(str) str##_hashed
#endif
