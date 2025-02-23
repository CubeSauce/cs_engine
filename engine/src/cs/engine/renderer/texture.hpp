// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include <string>

class Texture_Resource
{
public:
    uint32 width { 0 };
    uint32 height { 0 };
    uint32 num_channels { 0 };
    uint8* data { nullptr };
    //Type?

public:
    virtual ~Texture_Resource();

    Texture_Resource() = default;
    Texture_Resource(const std::string& filepath);

    bool initialize_from_file(const std::string& filepath);
};

class Texture
{
public:
    virtual ~Texture() = default;
};
