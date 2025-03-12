// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/engine/renderer/mesh.hpp"
#include "cs/engine/renderer/material.hpp"
#include "cs/engine/engine.hpp"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <cstring>
#include <filesystem>

Mesh_Resource::Mesh_Resource(const std::string& filepath)
{
    initialize_from_file(filepath);
}

bool Mesh_Resource::initialize_from_file(const std::string& filepath)
{
    name = filepath.c_str();

    mat4 qmat = quat::from_euler_angles(vec3(MATH_DEG_TO_RAD(-90.0f), 0.0f, 0.0f)).to_mat4();

    const char* extension = strrchr(filepath.c_str(), '.');
    assert(extension);
    assert(aiIsExtensionSupported(extension) == AI_TRUE);

    const char* folder = strrchr(filepath.c_str(), '/');
    int32 folder_index = folder - filepath.c_str() + 1;

    std::string folder_path(filepath);
    folder_path = folder_path.substr(0, folder_index);

    std::string path = std::filesystem::current_path().string() + "/" + filepath;
    const aiScene* ai_scene = aiImportFile(path.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);
    assert(ai_scene);

    uint32 offset = 0;
    for (uint32 m = 0; m < ai_scene->mNumMeshes; ++m)
    {
        const aiMesh* ai_mesh = ai_scene->mMeshes[m];

        aiMaterial* ai_material = ai_scene->mMaterials[ai_mesh->mMaterialIndex];
        
        aiColor4D ai_material_color;
        assert(AI_SUCCESS == ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, ai_material_color));
        vec4 material_color(ai_material_color.r, ai_material_color.g, ai_material_color.b, ai_material_color.a);

        Shared_Ptr<Material_Resource> material_resource = Shared_Ptr<Material_Resource>::create();
        
        Submesh_Data submesh_data;

        bool has_uvs = ai_mesh->HasTextureCoords(0);
        if(has_uvs)
        {
            material_resource->shader_resource = Engine::get().default_texture_shader_resource;

            if (ai_material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
            {
                aiString path;
                assert(aiReturn_SUCCESS == ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &path));
            
                material_resource->texture_resource = Shared_Ptr<Texture_Resource>::create(folder_path + path.C_Str());
            }
            else
            {
                material_resource->texture_resource = Engine::get().default_texture_resource;
            }
        }
        else
        {
            material_resource->shader_resource = Engine::get().default_color_shader_resource;
        }

        submesh_data.material_resource = material_resource;

        const uint32 num_vertices = (int32)(ai_mesh->mNumFaces * 3);
        for (uint32 f = 0; f < ai_mesh->mNumFaces; ++f)
        {
            Vertex_Data vertex;

            const aiFace& ai_face = ai_mesh->mFaces[f];

            int32 indices[] = {2, 1, 0};  // TODO: Do this with culling
            for (uint32 i = 0; i < ai_face.mNumIndices; ++i)
            {
                int32 index = ai_face.mIndices[indices[i]];

                const aiVector3D& v = ai_mesh->mVertices[index];
                vertex.vertex_location = {v.x, v.y, v.z};

                // Calculate mesh bounds for later
                submesh_data.bounds.include(vertex.vertex_location);
                bounds.include(vertex.vertex_location);

                const aiVector3D& n = ai_mesh->mNormals[index];
                vertex.vertex_normal = {n.x, n.y, n.z};

                vertex.vertex_color = material_color;

                //TODO: Support multiple colors
                if (has_uvs)
                {
                    for (uint32 u = 0; u < ai_mesh->GetNumUVChannels(); ++u)
                    {
                        if (ai_mesh->mTextureCoords[u])
                        {
                            const aiVector3D& uv = ai_mesh->mTextureCoords[u][index];
                            vertex.vertex_texture_coordinate = {uv.x, uv.y};
                            break;
                        }
                    }
                }

                submesh_data.vertices.add(vertex);
            }
        }

        submeshes.add(submesh_data);
    }

    return true;
}

Mesh::Mesh(const Shared_Ptr<Mesh_Resource> &in_mesh_resource)
    : mesh_resource(in_mesh_resource)
{
}