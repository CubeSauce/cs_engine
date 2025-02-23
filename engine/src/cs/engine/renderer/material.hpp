// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/vec4.hpp"
#include "cs/memory/shared_ptr.hpp"
#include "cs/engine/renderer/texture.hpp"

class Shader_Resource;
class Material_Resource
{
public:
    Shared_Ptr<Shader_Resource> shader_resource;
    Shared_Ptr<Texture_Resource> texture_resource;
    
public:
    virtual ~Material_Resource() = default;
};

class Material
{
public:
    virtual ~Material() = default;
};
