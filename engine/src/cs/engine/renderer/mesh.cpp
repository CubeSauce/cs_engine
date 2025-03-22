// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/engine/renderer/mesh.hpp"
#include "cs/engine/renderer/material.hpp"
#include "cs/engine/engine.hpp"

Mesh::Mesh(const Shared_Ptr<Mesh_Resource> &in_mesh_resource)
    : mesh_resource(in_mesh_resource)
{
}