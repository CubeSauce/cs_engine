#include "cs/cs.hpp"
#include "cs/engine/engine.hpp"
#include "cs/engine/game/game_instance.hpp"
#include "cs/engine/profiling/profiler.hpp"

#include "component.hpp"

#include <shared_mutex>

class Game : public Game_Instance
{
public:
    virtual void init() override
    {
        _init_player();

        Shared_Ptr<Mesh_Resource> kimono = Shared_Ptr<Mesh_Resource>::create("assets/mesh/kimono.obj");
        _add_static_object("kimono", vec3(5.0f, 0.0f, 0.0f), quat::from_rotation_axis(vec3::right_vector, MATH_DEG_TO_RAD(-90.0f)), kimono);
        _add_static_object("kimono2", vec3(-5.0f, 0.0f, 0.0f), quat::from_rotation_axis(vec3::right_vector, MATH_DEG_TO_RAD(-90.0f)), kimono);
    }

    virtual void update(float dt) override
    {
        for (int32 i = 0; i < _transform_components.components.size(); ++i)
        {
            Transform_Component& component = _transform_components.components[i];
          
            if (component.parent_id != Name_Id::Empty)
            {
                if (const Transform_Component* parent_transform_component = _transform_components.get(component.parent_id))
                {
                    component.local_to_world = component.local_to_world * parent_transform_component->local_to_world;
                }
            }
        }
    }

    virtual void render(const Shared_Ptr<Renderer>& renderer, VR_Eye::Type eye = VR_Eye::None) override
    {
        PROFILE_FUNCTION()

        std::shared_lock lock(_mutex);
    
        Shared_Ptr<Renderer_Backend> renderer_backend = renderer->backend;
        const Transform_Component* p_camera_transform = _transform_components.get(camera_id);
        if (!p_camera_transform)
        {
          return;
        }
    
        if (VR_System::get().is_valid())
        {
            VR_System::get().set_custom_camera_pose(p_camera_transform->local_to_world);
        }
        else
        {
            Shared_Ptr<Camera> camera = renderer->get_active_camera();
            camera->position = p_camera_transform->get_world_position();
            camera->orientation = p_camera_transform->get_world_orientation();
        }
    
        for (int32 i = 0; i < _render_components.components.size(); ++i)
        {
            const Render_Component& render_component = _render_components.components[i];
            const Shared_Ptr<Mesh_Resource>& mesh_resource = render_component.mesh;
            const Shared_Ptr<Mesh>& mesh = renderer_backend->get_mesh(mesh_resource);

            if (Transform_Component* transform_component = _transform_components.get(render_component.entity_id))
            {
                renderer_backend->draw_mesh(mesh, transform_component->get_world_matrix(), eye);
            }
        }
    }

    virtual void shutdown() override
    {

    }

private:
    std::shared_mutex _mutex;

    Component_Container<Transform_Component> _transform_components;
    Component_Container<Rigid_Body_Component> _rigid_body_components;
    Component_Container<Render_Component> _render_components;

    static inline const Name_Id player_id = Name_Id("player");
    static inline const Name_Id camera_id = Name_Id("player_camera");

    void _init_player()
    {
        Transform_Component& player_transform = _transform_components.add(player_id);
            
        Transform_Component& camera_transform = _transform_components.add(camera_id);
        camera_transform.parent_id = player_id;
        camera_transform.local_position = vec3(0.0f, 10.0f, 1.5f);
    }

    void _add_static_object(const Name_Id& name, const vec3& position, const quat& orientation, const Shared_Ptr<Mesh_Resource>& mesh_resource)
    {
        Transform_Component& transform = _transform_components.add(name);
        transform.local_position = position;
        transform.local_orientation = orientation;

        Render_Component& render = _render_components.add(name);
        render.mesh = mesh_resource;
    }
};

int main(int argc, char** argv)
{
  Dynamic_Array<std::string> args;
  for (int32 a = 1; a < argc; ++a)
  {
      args.add(argv[a]);
  }

  args.add("cs_vr_support=0");
  args.add("cs_num_threads=0");

  Engine engine;
  engine.initialize(args);
  engine.game_instance = Shared_Ptr<Game>::create();
  engine.run();
  engine.shutdown();

  Profiler::get().write_to_chrometracing_json("profiling/test_0_thread.json");

  return 0;
}