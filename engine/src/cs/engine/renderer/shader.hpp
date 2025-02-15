// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/math.hpp"
#include "cs/engine/name_id.hpp"
#include "cs/memory/shared_ptr.hpp"
#include "cs/engine/resource.hpp"

class Shader_Resource : public Resource
{
public:
    struct
    {
        const char* vertex_filepath = "";
        const char* geometry_filepath = "";
        const char* pixel_filepath = "";
    } source_paths [5]; // TODO: This would be nicer if we could compile from one source
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
