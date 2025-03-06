#include "cs/cs.hpp"
#include "cs/engine/engine.hpp"
#include "cs/engine/cvar.hpp"
#include "cs/engine/net/net_instance.hpp"
#include "cs/engine/renderer/camera.hpp"

#include "cs/engine/game/game_instance.hpp"
#include "cs/engine/renderer/renderer.hpp"
#include "cs/engine/renderer/mesh.hpp"

#include "cs/engine/input.hpp"

#include "cs/engine/profiling/profiler.hpp"

#include <chrono>
#include <shared_mutex>

template<typename Type>
struct Pair
{
  Type a, b;
};

struct Time_Entry
{
  std::string name;
  float duration;
};

struct Input_Component
{
    vec3 look_analog { vec3::zero_vector };
    vec3 look_mouse { vec3::zero_vector };
    vec3 move_analog { vec3::zero_vector };
    vec3 move_keyboard { vec3::zero_vector };
    bool start_button_pressed = false;
    float speed { 2.5f };
};

struct Transform_Component
{
    vec3 local_position { vec3::zero_vector };
    quat local_orientation { quat::zero_quat };
    vec3 local_scale { vec3::one_vector };
    mat4 local_matrix { mat4(1.0f) };

    Name_Id parent { Name_Id::Empty };
    vec3 world_position { vec3::zero_vector };
    quat world_orientation { quat::zero_quat };
    vec3 world_scale { vec3::one_vector };
    mat4 world_matrix { mat4(1.0f) };
};

class Mesh_Resource;
struct Render_Component
{
    Shared_Ptr<Mesh_Resource> mesh;
};

struct Camera_Component
{
  Shared_Ptr<Camera> camera;
};

template<typename Type>
struct Component_Container
{
    Dynamic_Array<Type> components;
    Hash_Map<int32> id_to_index = Hash_Map<int32>(1024);
    Hash_Map<Name_Id> index_to_id = Hash_Map<Name_Id>(1024);

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
  Camera_Component camera_component;
};

class Test_Game_Instance : public Game_Instance
{
public:
  Game_State game_state;
  std::shared_mutex mutex;
  int32 num_frames = 0;

public:

  virtual void init() override;
  virtual void update(float dt) override;
  virtual void render(const Shared_Ptr<Renderer>& renderer, VR_Eye::Type eye = VR_Eye::None) override;
  virtual void shutdown() override;

protected:
  void _init_player();
  void _init_test();
  void _init_dust2();

  void _update_inputs(float dt);
  void _update_transforms(float dt);

  Task_Graph _input_task_graph;
  void _input_task_worker(float dt, int32 component_index);
  void _construct_input_task_graph(float dt, Task_Graph& graph);

  Task_Graph _transform_task_graph;
  void _transform_task_worker(float dt, int32 component_index);
  void _construct_transform_task_graph(float dt, Task_Graph& graph);
};

void Test_Game_Instance::_init_player()
{
  PROFILE_FUNCTION()

  game_state.transform_components.add("player", {
    .local_position = vec3(0.0f, 0.0f, 0.0f), 
  });

  game_state.transform_components.add("player_mesh", {
    .local_orientation = quat::from_euler_angles(vec3(MATH_DEG_TO_RAD(-90.0f), 0.0f, 0.0f)),
    .parent = "player"
  });

  game_state.transform_components.add("camera", {
    .local_position = vec3(0.0f, 0.0f, 1.5f),
    .parent = "player"
  });
  //game_state.render_components.add("player", { .mesh = kimono, .mesh_transform_id = "player_mesh" });
  game_state.input_components.add("player", { .move_analog = vec3(0.0f), .speed = 100.0f });

  Input_System::get().register_input("l_lr", {{"GAMEPAD_AXIS_RIGHT_X", -1.0f}}).bind([&](float value, float multiplier) {
    Input_Component* input_component = game_state.input_components.get("player");
    input_component->look_analog.x = axis_map_deadzone(value, 0.2f);
  });

  Input_System::get().register_input("l_lr", {{"GAMEPAD_AXIS_RIGHT_Y", -1.0f}}).bind([&](float value, float multiplier) {
    Input_Component* input_component = game_state.input_components.get("player");
    input_component->look_analog.y = axis_map_deadzone(value, 0.2f);
  });

  Input_System::get().register_input("a_forward", {{"GAMEPAD_AXIS_LEFT_Y", 1.0f}}).bind([&](float value, float multiplier) {
    Input_Component* input_component = game_state.input_components.get("player");
    input_component->move_analog.y = axis_map_deadzone(value, 0.2f) * multiplier;
  });

  Input_System::get().register_input("a_right", {{"GAMEPAD_AXIS_LEFT_X", 1.0f}}).bind([&](float value, float multiplier) {
    Input_Component* input_component = game_state.input_components.get("player");
    input_component->move_analog.x = axis_map_deadzone(value, 0.2f) * multiplier;
  });
  
  Input_System::get().register_input("d_forward", {{"KEY_W", -1.0f}, {"KEY_S", 1.0f}}).bind([&](float value, float multiplier){
    Input_Component* input_component = game_state.input_components.get("player");
    input_component->move_keyboard.y = value * multiplier;
  });

  Input_System::get().register_input("d_right", {{"KEY_D", 1.0f}, {"KEY_A", -1.0f}}).bind([&](float value, float multiplier){
    Input_Component* input_component = game_state.input_components.get("player");
    input_component->move_keyboard.x = value * multiplier;
  });

  Input_System::get().register_input("trigger", {{"GAMEPAD_AXIS_RIGHT_TRIGGER", 1.0f}}).bind([&](float value, float multiplier) {
    Input_Component* input_component = game_state.input_components.get("player");
  });

  Input_System::get().register_input("start", {{"GAMEPAD_BUTTON_START", 1.0f}}).bind([&](float value, float multiplier) {
    Input_Component* input_component = game_state.input_components.get("player");
    input_component->start_button_pressed = value > 0.0f;
  });
  
}

void Test_Game_Instance::_init_test()
{
  PROFILE_FUNCTION()
  
  Shared_Ptr<Mesh_Resource> kimono = Shared_Ptr<Mesh_Resource>::create("assets/mesh/kimono.obj");
  Shared_Ptr<Mesh_Resource> plane = Shared_Ptr<Mesh_Resource>::create("assets/mesh/plane.obj");

  _init_player();
}

void Test_Game_Instance::_init_dust2()
{
  PROFILE_FUNCTION()

  _init_player();

  Shared_Ptr<Mesh_Resource> dust2 = Shared_Ptr<Mesh_Resource>::create("assets/mesh/de_dust2.obj");

  //game_state.transform_components.add("dust2", {
  //  .local_position = vec3(0.0f, 0.0f, 0.0f),
  //  .local_scale = vec3(0.01f)
  //  //.orientation = quat::from_euler_angles(vec3(MATH_DEG_TO_RAD(-90.0f), 0.0f, 0.0f))
  //});
  //game_state.render_components.add("dust2", { .mesh = dust2 });

  Shared_Ptr<Mesh_Resource> kimono = Shared_Ptr<Mesh_Resource>::create("assets/mesh/kimono.obj");
  const int N = 1000;
  for (int32 i = 0; i < N; ++i)
  {
    float x_rand = -N * 0.5f + (rand() % N) * 1.0f;
    float y_rand = -N * 0.5f + (rand() % N) * 1.0f;

    game_state.transform_components.add(std::format("test {}", i).c_str(), {
      .local_position = vec3(x_rand, y_rand, 0.0f),
      .local_orientation = quat::from_euler_angles(vec3(MATH_DEG_TO_RAD(-90.0f), 0.0f, 0.0f))
    });
    game_state.render_components.add(std::format("test {}", i).c_str(), { .mesh = kimono });
  }
}

void Test_Game_Instance::init()
{
  PROFILE_FUNCTION()
  
  _init_dust2();
}

void Test_Game_Instance::_transform_task_worker(float dt, int32 component_index)
{
  PROFILE_FUNCTION()

  std::unique_lock lock(mutex);

  Transform_Component& component = game_state.transform_components.components[component_index];
    
  component.local_matrix = translate(mat4(1.0f), component.local_position);
  component.local_matrix = component.local_matrix * component.local_orientation.to_mat4();

  if (component.parent != Name_Id::Empty)
  {
    mat4 parent_transform(1.0f);
    if (const Transform_Component* parent_transform_component = game_state.transform_components.get(component.parent))
    {
      parent_transform = translate(parent_transform, parent_transform_component->world_position);
      parent_transform = parent_transform * parent_transform_component->world_orientation.to_mat4();
    }

    component.world_matrix = component.local_matrix * parent_transform;
    component.world_position = component.world_matrix[3].xyz;
    component.world_orientation = quat::from_mat4(component.world_matrix);
  }
  else
  {
    component.world_matrix = component.local_matrix;
    component.world_position = component.local_position;
    component.world_orientation = component.local_orientation;
  }
}

void Test_Game_Instance::_construct_transform_task_graph(float dt, Task_Graph& graph)
{
  PROFILE_FUNCTION()

  graph.clear();

  Dynamic_Array<Shared_Ptr<Task>> tasks;
  Dynamic_Array<Pair<int32>> dependencies;
  for (int32 i = 0; i < game_state.transform_components.components.size(); ++i)
  {
    Transform_Component& component = game_state.transform_components.components[i];
    if (component.parent != Name_Id::Empty)
    {
      if (int32* found_index = game_state.transform_components.id_to_index.find(component.parent); *found_index >= 0 && *found_index != i)
      {
        dependencies.add({.a = i, .b = *found_index});
      }
    }

    tasks.add(graph.create_task(std::bind(&Test_Game_Instance::_transform_task_worker, this, dt, i)));
  }

  for (const Pair<int32>& dependency_pair : dependencies)
  {
    tasks[dependency_pair.a]->add_dependency(tasks[dependency_pair.b]);
  }
} 

void Test_Game_Instance::_input_task_worker(float dt, int32 component_index)
{
  PROFILE_FUNCTION()

  std::unique_lock lock(mutex);

  Input_Component& input_component = game_state.input_components.components[component_index];
  const Name_Id* p_id = game_state.input_components.index_to_id.find(component_index);
  if (!p_id)
  {
    return;
  }

  Transform_Component* transform_component = game_state.transform_components.get(*p_id);
  if (!transform_component)
  {
    return;
  }

  vec3 input(0.0f);
  input.x = clamp(input_component.move_analog.x + input_component.move_keyboard.x, -1.0f, 1.0f);
  input.y = clamp(input_component.move_analog.y + input_component.move_keyboard.y, -1.0f, 1.0f);

  vec3 camera_forward;
  if (VR_System::get().is_valid())
  {
    const mat4& camera_view = VR_System::get().get_camera(VR_Eye::None)->get_view().inverse();
    camera_forward = camera_view[1].xyz;
    //camera_forward.z = 0;
    camera_forward.normalize();
  } 
  else if (Transform_Component* camera_transform = game_state.transform_components.get("camera"))
  {
    camera_forward = transform_component->world_orientation.get_direction().normalize();
  }

  const vec3 camera_right = vec3::up_vector.cross(camera_forward).normalize();
  vec3 movement_direction = (camera_forward * input.y + camera_right * input.x).normalize();
  
  transform_component->local_position += movement_direction * input_component.speed * dt;
}

void Test_Game_Instance::_construct_input_task_graph(float dt, Task_Graph& graph)
{
  PROFILE_FUNCTION()

  graph.clear();

  Dynamic_Array<Shared_Ptr<Task>> tasks;
  for (int32 i = 0; i < game_state.input_components.components.size(); ++i)
  {
    tasks.add(graph.create_task(std::bind(&Test_Game_Instance::_input_task_worker, this, dt, i)));
  }
} 

void Test_Game_Instance::update(float dt)
{
  PROFILE_FUNCTION()

  if (1)
  {
    _construct_input_task_graph(dt, _input_task_graph);
    _construct_transform_task_graph(dt, _transform_task_graph);
  
    // Executes wait for finish
    _input_task_graph.execute();
    _transform_task_graph.execute();
  }
  else
  {
    _update_inputs(dt);
    _update_transforms(dt);
  }

  num_frames++;
}

void Test_Game_Instance::_update_inputs(float dt)
{
  PROFILE_FUNCTION()

  for (int32 i = 0; i < game_state.input_components.components.size(); ++i)
  {
    Input_Component& input_component = game_state.input_components.components[i];
    const Name_Id* p_id = game_state.input_components.index_to_id.find(i);
    if (!p_id)
    {
      continue;
    }

    Transform_Component* transform_component = game_state.transform_components.get(*p_id);
    if (!transform_component)
    {
      continue;
    }

    vec3 input(0.0f);
    input.x = clamp(input_component.move_analog.x + input_component.move_keyboard.x, -1.0f, 1.0f);
    input.y = clamp(input_component.move_analog.y + input_component.move_keyboard.y, -1.0f, 1.0f);

    vec3 camera_forward;
    if (VR_System::get().is_valid())
    {
      const mat4& camera_view = VR_System::get().get_camera(VR_Eye::None)->get_view().inverse();
      camera_forward = camera_view[1].xyz;
      //camera_forward.z = 0;
      camera_forward.normalize();
    } 
    else if (Transform_Component* camera_transform = game_state.transform_components.get("camera"))
    {
      camera_forward = transform_component->world_orientation.get_direction().normalize();
    }

    const vec3 camera_right = vec3::up_vector.cross(camera_forward).normalize();
    vec3 movement_direction = (camera_forward * input.y + camera_right * input.x).normalize();
    
    transform_component->local_position += movement_direction * input_component.speed * dt;
  }
}

void Test_Game_Instance::_update_transforms(float dt)
{
  PROFILE_FUNCTION()

  for (int32 i = 0; i < game_state.transform_components.components.size(); ++i)
  {
    Scoped_Profiler(std::format("Transform {}", i).c_str());

    Transform_Component& component = game_state.transform_components.components[i];
    
    component.local_matrix = translate(mat4(1.0f), component.local_position);
    component.local_matrix = component.local_matrix * component.local_orientation.to_mat4();

    if (component.parent != Name_Id::Empty)
    {
      mat4 parent_transform(1.0f);
      if (const Transform_Component* parent_transform_component = game_state.transform_components.get(component.parent))
      {
        parent_transform = translate(parent_transform, parent_transform_component->world_position);
        parent_transform = parent_transform * parent_transform_component->world_orientation.to_mat4();
      }

      component.world_matrix = component.local_matrix * parent_transform;
      component.world_position = component.world_matrix[3].xyz;
      component.world_orientation = quat::from_mat4(component.world_matrix);
    }
    else
    {
      component.world_matrix = component.local_matrix;
      component.world_position = component.local_position;
      component.world_orientation = component.local_orientation;
    }
  }
}

Hash_Map<Shared_Ptr<Mesh>> meshes;
void Test_Game_Instance::render(const Shared_Ptr<Renderer>& renderer, VR_Eye::Type eye)
{
    PROFILE_FUNCTION()

    std::unique_lock lock(mutex);

    Shared_Ptr<Renderer_Backend> renderer_backend = renderer->backend;
    Transform_Component* p_camera_transform = game_state.transform_components.get("camera");
    if (!p_camera_transform)
    {
      return;
    }

    if (VR_System::get().is_valid())
    {
        VR_System::get().set_custom_camera_pose(p_camera_transform->world_matrix);
    }
    else
    {
      renderer->get_active_camera()->position = p_camera_transform->world_position;
      renderer->get_active_camera()->orientation = p_camera_transform->world_orientation;
    }

    for (int32 i = 0; i < game_state.render_components.components.size(); ++i)
    {
        const Render_Component& render_component = game_state.render_components.components[i];
        const Name_Id* p_id = game_state.render_components.index_to_id.find(i);

        if (!p_id)
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


        if (Transform_Component* transform_component = game_state.transform_components.get(*p_id))
        {
          renderer_backend->draw_mesh(mesh, transform_component->world_matrix, eye);
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

  args.add("cs_vr_support=0");
  args.add("cs_num_threads=8");

  Engine engine;
  engine.initialize(args);
  engine.game_instance = Shared_Ptr<Test_Game_Instance>::create();
  engine.run();
  engine.shutdown();

  Profiler::get().write_to_chrometracing_json("test_8_thread.json");

  return 0;
}