// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/engine/resource/renderer_resources.hpp"
#include "cs/engine/engine.hpp"

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "stb_image.h"

#include <cstring>
#include <filesystem>

Texture_Resource::Texture_Resource(const std::string& filepath)
{
    assert(initialize_from_file(filepath));
}

Texture_Resource::~Texture_Resource()
{
    if (data)
    {
        stbi_image_free(data);
        data = nullptr;
    }
}

bool Texture_Resource::initialize_from_file(const std::string& filepath)
{
    int width_i, height_i, num_channels_i;
    stbi_set_flip_vertically_on_load(true);
    data = stbi_load(filepath.c_str(), &width_i, &height_i, &num_channels_i, 4);

    if (data == nullptr)
    {
        return false;
    }

    width = (uint32)width_i;
    height = (uint32)height_i;
    num_channels = 4; // Always set because of stbi_load param
    
    return true;
}

Mesh_Import_Settings Mesh_Import_Settings::default_import_settings = { vec3::zero_vector, quat::zero_quat, vec3(1.0f) };

Mesh_Resource::Mesh_Resource(const std::string& filepath, const Mesh_Import_Settings& import_settings)
{
    initialize_from_file(filepath, import_settings);
}

bool Mesh_Resource::initialize_from_file(const std::string& filepath, const Mesh_Import_Settings& import_settings)
{
    name = filepath.c_str();

    const mat4 import_mat = import_settings.import_rotation.to_mat4();
    const mat4 import_mat_inv = import_mat.inverse();

    const char* extension = strrchr(filepath.c_str(), '.');
    assert(extension);
    assert(aiIsExtensionSupported(extension) == AI_TRUE);

    const char* folder = strrchr(filepath.c_str(), '/');
    size_t folder_index = folder - filepath.c_str() + 1;

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
        submesh_data.bounds = AABB::empty_box;

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
            submesh_data.indices.push_back(ai_mesh->mFaces[f].mIndices[2]);
            submesh_data.indices.push_back(ai_mesh->mFaces[f].mIndices[1]);
            submesh_data.indices.push_back(ai_mesh->mFaces[f].mIndices[0]);
        }
        
        Vertex_Data vertex;

        int32 indices[] = {2, 1, 0};  // TODO: Do this with culling
        for (uint32 i = 0; i < ai_mesh->mNumVertices; ++i)
        {                
            const aiVector3D& v = ai_mesh->mVertices[i];
            vertex.vertex_location = import_mat * vec3(v.x, v.y, v.z);

            const aiVector3D& n = ai_mesh->mNormals[i];
            vertex.vertex_normal = import_mat_inv * vec3(n.x, n.y, n.z);

            vertex.vertex_color = material_color;

            //TODO: Support multiple colors
            if (has_uvs)
            {
                for (uint32 u = 0; u < ai_mesh->GetNumUVChannels(); ++u)
                {
                    if (ai_mesh->mTextureCoords[u])
                    {
                        const aiVector3D& uv = ai_mesh->mTextureCoords[u][i];
                        vertex.vertex_texture_coordinate = {uv.x, uv.y};
                        break;
                    }
                }
            }


            // Calculate mesh bounds for later
            submesh_data.bounds.expand(vertex.vertex_location);
            bounds.expand(vertex.vertex_location);
            submesh_data.vertices.push_back(vertex);
        }

        submeshes.push_back(submesh_data);
    }

    return true;
}