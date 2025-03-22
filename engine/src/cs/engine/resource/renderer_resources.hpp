// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/math.hpp"
#include "cs/engine/resource.hpp"
#include "cs/memory/shared_ptr.hpp"
#include "cs/containers/dynamic_array.hpp"

#include <string>

class Shader_Resource : public Resource
{
public:
    struct
    {
        const char* vertex_filepath = "";
        const char* geometry_filepath = "";
        union {
            const char* fragment_filepath = "";
            const char* pixel_filepath;
        };
    } source_paths [5]; // TODO: This would be nicer if we could compile from one source
};

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

class Material_Resource
{
public:
    Shared_Ptr<Shader_Resource> shader_resource;
    Shared_Ptr<Texture_Resource> texture_resource;
    
public:
    virtual ~Material_Resource() = default;
};

struct Vertex_Data
{
	vec3 vertex_location { vec3::zero_vector };
	vec3 vertex_normal { vec3::zero_vector };
	vec2 vertex_texture_coordinate { vec2::zero_vector };
	vec4 vertex_color { vec4::one_vector };
};

class Material_Resource;
struct Submesh_Data
{
	AABB bounds { AABB::empty_box };
	Dynamic_Array<Vertex_Data> vertices;
	Dynamic_Array<uint32> indices;
    Shared_Ptr<Material_Resource> material_resource;
};

struct Mesh_Import_Settings
{
	static Mesh_Import_Settings default_import_settings;

	vec3 import_offset;
	quat import_rotation;
	vec3 import_scale;
};

class Mesh_Resource : public Resource
{
public:
	AABB bounds { AABB::empty_box };
	Dynamic_Array<Submesh_Data> submeshes;

public:
	Mesh_Resource() = default;
	Mesh_Resource(const std::string& filepath, const Mesh_Import_Settings& import_settings = Mesh_Import_Settings::default_import_settings);

	bool initialize_from_file(const std::string& filepath, const Mesh_Import_Settings& import_settings = Mesh_Import_Settings::default_import_settings);
};
