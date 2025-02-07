#include "cs/cs.hpp"
#include "cs/engine/engine.hpp"
#include "cs/engine/cvar.hpp"
#include "cs/engine/net/net_instance.hpp"
#include "cs/engine/renderer/camera.hpp"

#include "cs/engine/game/game_instance.hpp"
#include "cs/engine/renderer/renderer.hpp"
#include "cs/engine/renderer/mesh.hpp"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Shared_Ptr<Material_Resource> default_material;

Shared_Ptr<Mesh_Resource> import(const char* filepath)
{
  Shared_Ptr<Mesh_Resource> mesh_resource = Shared_Ptr<Mesh_Resource>::create();

  mat4 qmat = quat::from_euler_angles(vec3(MATH_DEG_TO_RAD(-90.0f), 0.0f, 0.0f)).to_mat4();

  const char* extension = strrchr(filepath, '.');
	assert(extension);
  assert(aiIsExtensionSupported(extension) == AI_TRUE);

  const aiScene* ai_scene = aiImportFile(filepath, aiProcessPreset_TargetRealtime_MaxQuality);
  assert(ai_scene);

  uint32 offset = 0;
  for (uint32 m = 0; m < ai_scene->mNumMeshes; ++m)
  {
    const aiMesh* ai_mesh = ai_scene->mMeshes[m];

    Submesh_Data submesh_data;

    submesh_data.material_resource = default_material;
    
    const uint32 num_vertices = (int32)(ai_mesh->mNumFaces * 3);
    for (uint32 f = 0; f < ai_mesh->mNumFaces; ++f)
    {
      const aiFace& ai_face = ai_mesh->mFaces[f];
      int32 indices[] = {2, 1, 0};  // TODO: Do this with culling
      for (uint32 i = 0; i < ai_face.mNumIndices; ++i)
      {
        const aiVector3D& v = ai_mesh->mVertices[ai_face.mIndices[indices[i]]];
        const aiVector3D& n = ai_mesh->mNormals[ai_face.mIndices[indices[i]]];
        
        submesh_data.vertices.add({{v.x, v.y, v.z}, {n.x, n.y, n.z}, {0.0f, 0.0f}});
      }
    }

    mesh_resource->submeshes.add(submesh_data);
  }

  return mesh_resource;
}

struct Transform_Component
{
    vec3 position;
    quat orientation;
};

class Mesh_Resource;
struct Render_Component
{
    Shared_Ptr<Mesh_Resource> mesh;
};

struct Game_State
{
    Component_Container<Transform_Component> transform_components;
    Component_Container<Render_Component> render_components;
};

class Test_Game_Instance : public Game_Instance
{
public:
  Game_State game_state;
public:

  virtual void init() override;
  virtual void update(float dt) override;
  virtual void render(const Shared_Ptr<Renderer>& renderer) override;
  virtual void shutdown() override;
};

void Test_Game_Instance::init()
{
  //TODO: Engine defaults
  Shared_Ptr<Shader_Resource> shader_resource = Shared_Ptr<Shader_Resource>::create();
  //shader_resource->vertex_filepath = "assets/shaders/directx/main.vs.hlsl";
  //shader_resource->pixel_filepath = "assets/shaders/directx/main.ps.hlsl";
  shader_resource->vertex_filepath = "assets/shaders/opengl/main.vs.glsl";
  shader_resource->pixel_filepath = "assets/shaders/opengl/main.ps.glsl";

  default_material = Shared_Ptr<Material_Resource>::create();
  default_material->shader_resource = shader_resource;

  game_state.transform_components.add("player", {vec3(0.0f), quat::from_euler_angles(vec3(MATH_DEG_TO_RAD(-90.0f), 0.0f, 0.0f))});
  game_state.render_components.add("player", { import("assets/mesh/kimono.obj") });
}

float times = 0.0f;
Hash_Table<mat4> transforms;
void Test_Game_Instance::update(float dt)
{
    times += dt;

    if (Transform_Component* component = game_state.transform_components.get("player"))
    {
      component->position.x = sinf(times) * 5.0f;
      component->position.y = cosf(times) * 5.0f;
      component->position.z = cosf(times * 2.0f) * 5.0f + 2.5f;
    }

    for (int32 i = 0; i < game_state.transform_components.components.size(); ++i)
    {
        const Transform_Component& component = game_state.transform_components.components[i];
        const Name_Id& id = game_state.transform_components.index_to_id[i];
        mat4& transform = transforms[id];
        transform = mat4(1.0f);
        transform = transform * translate(mat4(1.0f), component.position);
        transform = transform * component.orientation.to_mat4();
    }
}

Hash_Table<Shared_Ptr<Mesh>> meshes;
void Test_Game_Instance::render(const Shared_Ptr<Renderer>& renderer)
{
    Shared_Ptr<Renderer_Backend> renderer_backend = renderer->backend;

    for (int32 i = 0; i < game_state.render_components.components.size(); ++i)
    {
        const Render_Component& render_component = game_state.render_components.components[i];
        const Name_Id& id = game_state.transform_components.index_to_id[i];

        Shared_Ptr<Mesh_Resource> mesh_resource = render_component.mesh;
        Shared_Ptr<Mesh>& mesh = meshes[mesh_resource->name];
        if (!mesh)
        {
            mesh = renderer->backend->create_mesh(mesh_resource);
            // TODO: Track when to do it
            mesh->upload_data();
        }

        renderer_backend->draw_mesh(mesh, transforms[id]);
    }
}

void Test_Game_Instance::shutdown()
{

}

int main(int argc, char** argv)
{
  Dynamic_Array<std::string> args;
  for (int32 a = 1; a < argc; ++a)
  {
      args.add(argv[a]);
  }

  Engine engine;
  engine.initialize(args);
  engine.game_instance = Shared_Ptr<Test_Game_Instance>::create();
  engine.run();
  engine.shutdown();

  return 0;
}