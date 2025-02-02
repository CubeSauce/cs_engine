// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/math.hpp"
#include "cs/engine/name_id.hpp"
#include "cs/memory/shared_ptr.hpp"
#include "cs/engine/resource.hpp"
#include "cs/containers/hash_table.hpp"

class Shader_Resource : public Resource
{
public:
    const char* vertex_filepath = "";
    const char* geometry_filepath = "";
    const char* pixel_filepath = "";
};

class Shader
{
public:
    Shared_Ptr<Shader_Resource> shader_resource;

public:
    virtual ~Shader() {}

    virtual void bind() const = 0;
    virtual void unbind() const = 0;
};
