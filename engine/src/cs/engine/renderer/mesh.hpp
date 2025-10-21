// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/engine/resource/renderer_resources.hpp"

struct Instance_Data
{
	mat4 instance_matrix;
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
	virtual void upload_data(const Dynamic_Array<Instance_Data>& instances) = 0;
};
