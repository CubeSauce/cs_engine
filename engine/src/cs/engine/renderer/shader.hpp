// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/engine/resource/renderer_resources.hpp"

class Shader
{
public:
    Shared_Ptr<Shader_Resource> shader_resource;

public:
    virtual ~Shader() {}

    virtual void bind() const = 0;
    virtual void unbind() const = 0;
};
