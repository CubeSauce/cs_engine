#pragma once

#include "cs/cs.hpp"
#include "cs/math/vec2.hpp"
#include "cs/math/vec3.hpp"
#include "cs/engine/resource.hpp"
#include "cs/memory/shared_ptr.hpp"
#include "cs/containers/dynamic_array.hpp"

struct Vertex_Data
{
	vec3 vertex_location;
	vec3 vertex_normal;
	vec2 vertex_texture_coordinate;
};

class Material_Resource;
struct Submesh_Data
{
	Dynamic_Array<Vertex_Data> vertices;
    Shared_Ptr<Material_Resource> material_resource;
};

class Mesh_Resource : public Resource
{
public:
	Dynamic_Array<Submesh_Data> submeshes;
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
