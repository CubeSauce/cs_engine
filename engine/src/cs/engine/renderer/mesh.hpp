#pragma once

#include "cs/cs.hpp"
#include "cs/math/math.hpp"
#include "cs/engine/resource.hpp"
#include "cs/memory/shared_ptr.hpp"
#include "cs/containers/dynamic_array.hpp"

#include <string>

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
	Box bounds;
	Dynamic_Array<Vertex_Data> vertices;
    Shared_Ptr<Material_Resource> material_resource;
};

class Mesh_Resource : public Resource
{
public:
	Box bounds;
	Dynamic_Array<Submesh_Data> submeshes;

public:
	Mesh_Resource() = default;
	Mesh_Resource(const std::string& filepath);

	bool initialize_from_file(const std::string& filepath);
};

class Mesh
{
public:
    Shared_Ptr<Mesh_Resource> mesh_resource;

public:
	Mesh() = default;
	Mesh(const Shared_Ptr<Mesh_Resource> &in_mesh_resource);
	virtual ~Mesh() = default;
	virtual void upload_data() = 0;
};
