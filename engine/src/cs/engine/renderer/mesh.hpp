// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/engine/resource/renderer_resources.hpp"

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
