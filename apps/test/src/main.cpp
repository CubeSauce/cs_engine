#include "cs/cs.hpp"
#include "cs/engine/engine.hpp"
#include "cs/engine/cvar.hpp"
#include "cs/engine/net/net_instance.hpp"
#include "cs/engine/renderer/camera.hpp"

#include "cs/engine/game/game_instance.hpp"
#include "cs/engine/renderer/renderer.hpp"
#include "cs/engine/renderer/mesh.hpp"

#include "cs/engine/input.hpp"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Shared_Ptr<Material_Resource> default_material_resource;

Shared_Ptr<Mesh_Resource> import(const char* filepath)
{
  Shared_Ptr<Mesh_Resource> mesh_resource = Shared_Ptr<Mesh_Resource>::create();
  mesh_resource->name = filepath;

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

    submesh_data.material_resource = default_material_resource;
    
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

struct Input_Component
{
    vec3 analog_input { vec3::zero_vector };
    vec3 digital_input { vec3::zero_vector };
    float rotation { 0.0f };
};

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

template<typename Type>
struct Component_Container
{
    Dynamic_Array<Type> components;
    Hash_Map<int32> id_to_index;
    Hash_Map<Name_Id> index_to_id;

    void add(const Name_Id& id, const Type& type)
    {
        // Don't add we already have something with this id
        int32* p_index = id_to_index.find(id);
        if (p_index)
        {
            return;
        }

        int32 new_index = components.size();
        id_to_index.add(id, new_index);
        index_to_id.add(new_index, id);
        components.add(type);
    }

    Type* get(const Name_Id& id)
    {
        int32* p_index = id_to_index.find(id);
        if (p_index == nullptr)
        {
            return nullptr;
        }

        return &components[*p_index];
    }
};

struct Game_State
{
  Component_Container<Input_Component> input_components;
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
  virtual void render(const Shared_Ptr<Renderer>& renderer, VR_Eye::Type eye = VR_Eye::None) override;
  virtual void shutdown() override;
};

void Test_Game_Instance::init()
{
  Shared_Ptr<Mesh_Resource> kimono = import("assets/mesh/kimono.obj");
  Shared_Ptr<Mesh_Resource> plane = import("assets/mesh/plane.obj");

  game_state.transform_components.add("player", {
    .position = vec3(0.0f, 5.0f, 0.0f), 
    .orientation = quat::from_euler_angles(vec3(MATH_DEG_TO_RAD(-90.0f), 0.0f, 0.0f))
  });
  game_state.render_components.add("player", { .mesh = kimono });
  game_state.input_components.add("player", { .analog_input = vec3(0.0f) });

  Input_System::get().register_input("a_forward", {{"GAMEPAD_AXIS_RIGHT_Y", -1.0f}}).bind([&](float value, float multiplier) {
    Input_Component* input_component = game_state.input_components.get("player");
    input_component->analog_input.y = value * multiplier;
  });

  Input_System::get().register_input("right", {{"GAMEPAD_AXIS_RIGHT_X", 1.0f}}).bind([&](float value, float multiplier) {
    Input_Component* input_component = game_state.input_components.get("player");
    input_component->analog_input.x = value;
  });
  
  Input_System::get().register_input("d_forward", {{"KEY_W", 1.0f}, {"KEY_S", -1.0f}}).bind([&](float value, float multiplier){
    Input_Component* input_component = game_state.input_components.get("player");
    input_component->digital_input.y = value * multiplier;
  });

  Input_System::get().register_input("d_right", {{"KEY_D", 1.0f}, {"KEY_A", -1.0f}}).bind([&](float value, float multiplier){
    Input_Component* input_component = game_state.input_components.get("player");
    input_component->digital_input.x = value * multiplier;
  });

  Input_System::get().register_input("rotation", {{"GAMEPAD_AXIS_RIGHT_TRIGGER", 1.0f}, {"GAMEPAD_AXIS_LEFT_TRIGGER", -1.0f}}).bind([&](float value, float multiplier) {
    Input_Component* input_component = game_state.input_components.get("player");
    // Axis returns [-1, 1] but trigger can go in just one dir
    input_component->rotation = ((value + 1) * 0.5f) * multiplier;
  });

  game_state.transform_components.add("npc1", {
    .position = vec3(-5.0f, 5.0f, 0.0f), 
    .orientation = quat::from_euler_angles(vec3(MATH_DEG_TO_RAD(-90.0f), 0.0f, 0.0f))
  });
  game_state.render_components.add("npc1", { .mesh = kimono });

  game_state.transform_components.add("npc2", {
    .position = vec3(5.0f, -5.0f, 0.0f), 
    .orientation = quat::from_euler_angles(vec3(MATH_DEG_TO_RAD(-90.0f), 0.0f, 0.0f))
  });
  game_state.render_components.add("npc2", { .mesh = kimono });
  
  game_state.transform_components.add("npc3", {
    .position = vec3(-5.0f, -5.0f, 0.0f), 
    .orientation = quat::from_euler_angles(vec3(MATH_DEG_TO_RAD(-90.0f), 0.0f, 0.0f))
  });
  game_state.render_components.add("npc3", { .mesh = kimono });

  game_state.transform_components.add("npc4", {
    .position = vec3(5.0f, 5.0f, 0.0f), 
    .orientation = quat::from_euler_angles(vec3(MATH_DEG_TO_RAD(-90.0f), 0.0f, 0.0f))
  });
  game_state.render_components.add("npc4", { .mesh = kimono });

  game_state.transform_components.add("plane", {
    .position = vec3(0.0f, 0.0f, -2.0f), 
    .orientation = quat::from_euler_angles(vec3(MATH_DEG_TO_RAD(-90.0f), 0.0f, 0.0f))
  });
  game_state.render_components.add("plane", { .mesh = plane });
}

Hash_Map<mat4> transforms;
void Test_Game_Instance::update(float dt)
{
    for (int32 i = 0; i < game_state.transform_components.components.size(); ++i)
    {
        Transform_Component& component = game_state.transform_components.components[i];
        const Name_Id* p_id = game_state.transform_components.index_to_id.find(i);
        if (p_id == nullptr)
        {
          continue;
        }

        if (const Input_Component* input_component = game_state.input_components.get(*p_id))
        {
          vec3 input(0.0f);
          input.x = clamp(input_component->analog_input.x + input_component->digital_input.x, -1.0f, 1.0f);
          input.y = clamp(input_component->analog_input.y + input_component->digital_input.y, -1.0f, 1.0f);

          component.position += input * 3.0f * dt;
          component.orientation = component.orientation.mul(quat::from_rotation_axis(vec3(0.0f, 0.0f, 1.0f), input_component->rotation * dt));
        }

        mat4& transform = transforms.find_or_add(*p_id);
        transform = translate(mat4(1.0f), component.position);
        transform = transform * component.orientation.to_mat4();
    }
}

Hash_Map<Shared_Ptr<Mesh>> meshes;
void Test_Game_Instance::render(const Shared_Ptr<Renderer>& renderer, VR_Eye::Type eye)
{
    Shared_Ptr<Renderer_Backend> renderer_backend = renderer->backend;

    for (int32 i = 0; i < game_state.render_components.components.size(); ++i)
    {
        const Render_Component& render_component = game_state.render_components.components[i];
        const Name_Id* p_id = game_state.transform_components.index_to_id.find(i);
        if (p_id == nullptr)
        {
          continue;
        }

        Shared_Ptr<Mesh_Resource> mesh_resource = render_component.mesh;
        Shared_Ptr<Mesh>& mesh = meshes.find_or_add(mesh_resource->name);
        if (!mesh)
        {
            mesh = renderer->backend->create_mesh(mesh_resource);
            // TODO: Track when to do it
            mesh->upload_data();
        }

        if (mat4* p_transform = transforms.find(*p_id))
        {
          renderer_backend->draw_mesh(mesh, *p_transform);
        }
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
  default_material_resource = engine.default_material_resource; // Currently just to get it somehow
  engine.game_instance = Shared_Ptr<Test_Game_Instance>::create();
  engine.run();
  engine.shutdown();

  return 0;
}