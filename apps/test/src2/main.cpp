#include "cs/cs.hpp"
#include "cs/engine/engine.hpp"
#include "cs/engine/game/game_instance.hpp"
#include "cs/engine/profiling/profiler.hpp"
#include "cs/engine/physics/physics_system.hpp"
#include "component.hpp"
#include <shared_mutex>

class Game : public Game_Instance
{
    Shared_Ptr<Mesh_Resource> unit_sphere;
    Shared_Ptr<Mesh_Resource> unit_capsule;
    Shared_Ptr<Mesh_Resource> unit_cylinder;
    Shared_Ptr<Mesh_Resource> unit_box;
    Shared_Ptr<Mesh_Resource> unit_convex;

    Shared_Ptr<Mesh_Resource> floor;

public:
    virtual void init() override
    {
        PROFILE_FUNCTION()

        Mesh_Import_Settings import_settings = Mesh_Import_Settings::default_import_settings;
        import_settings.import_rotation = quat::from_euler_angles(vec3(0_deg, 0_deg, 0_deg));

        unit_sphere = Shared_Ptr<Mesh_Resource>::create("assets/meshes/test/unit_sphere.obj", import_settings);
        unit_capsule = Shared_Ptr<Mesh_Resource>::create("assets/meshes/test/unit_capsule.obj", import_settings);
        unit_cylinder = Shared_Ptr<Mesh_Resource>::create("assets/meshes/test/unit_cylinder.obj", import_settings);
        unit_box = Shared_Ptr<Mesh_Resource>::create("assets/meshes/test/unit_box.obj", import_settings);
        unit_convex = Shared_Ptr<Mesh_Resource>::create("assets/meshes/test/unit_convex.obj", import_settings);
        floor = Shared_Ptr<Mesh_Resource>::create("assets/meshes/test/floor.obj", import_settings);
        {
            Transform_Component& player_transform = _transform_components.add("player");
                
            Transform_Component& camera_transform = _transform_components.add("player_camera");
            camera_transform.parent_id = "player";
            camera_transform.local_position = vec3(-10.0f, -10.0f, 2.0f);
            camera_transform.local_orientation = quat::from_euler_angles(vec3(0_deg, 0_deg, -45_deg));
        }

        {
            Name_Id name("floor");
            Transform_Component& transform = _transform_components.add(name);
            transform.local_orientation = quat::from_euler_angles(vec3(-0_deg, -0_deg, 0_deg));

            Render_Component& render = _render_components.add(name);
            render.mesh = floor;
            //render.visible = false;
            
            Physics_Body_Component& pb = _physics_body_components.add(name);
            pb.type = Physics_Body::Static;
            pb.component_id = name;
            pb.mass = FLT_MAX;
            pb.collider.type = Collider::Box;
            pb.collider.bounds = floor->bounds;
            pb.collider.shape.bounding_box = floor->bounds;
        }

        if (1)
        {
            Name_Id name("sphere");
            Transform_Component& transform = _transform_components.add(name);
            transform.local_position = vec3(0.0f, 0.0f, 10.0f);
            transform.local_orientation = quat::from_euler_angles(vec3(0_deg, 0_deg, 0_deg));

            Render_Component& render = _render_components.add(name);
            render.mesh = unit_sphere;

            Physics_Body_Component& pb = _physics_body_components.add(name);
            pb.type = Physics_Body::Dynamic;
            pb.component_id = name;
            pb.mass = 1.0f;  //Is this valid?
            pb.collider.type = Collider::Sphere;
            pb.collider.bounds = unit_sphere->bounds;
            pb.collider.shape.sphere.radius = 1.0f;
        }

        if (0)
        {
            Name_Id name("sphere2");
            Transform_Component& transform = _transform_components.add(name);
            transform.local_position = vec3(5.0f, 0.0f, 10.0f);
            transform.local_orientation = quat::from_euler_angles(vec3(0_deg, 0_deg, 0_deg));

            Render_Component& render = _render_components.add(name);
            render.mesh = unit_sphere;

            Physics_Body_Component& pb = _physics_body_components.add(name);
            pb.type = Physics_Body::Dynamic;
            pb.component_id = name;
            pb.mass = 1.0f;  //Is this valid?
            pb.collider.type = Collider::Sphere;
            pb.collider.bounds = unit_sphere->bounds;
            pb.collider.shape.sphere.radius = 1.0f;
        }

        if (1)
        {
            Name_Id name("capsule");
            Transform_Component& transform = _transform_components.add(name);
            transform.local_position = vec3(5.0f, 0.0f, 10.0f);
            transform.local_orientation = quat::from_euler_angles(vec3(0_deg, 0_deg, 0_deg));

            Render_Component& render = _render_components.add(name);
            render.mesh = unit_capsule;

            Physics_Body_Component& pb = _physics_body_components.add(name);
            pb.type = Physics_Body::Dynamic;
            pb.component_id = name;
            pb.mass = 1.0f;  //Is this valid?
            pb.collider.type = Collider::Capsule;
            pb.collider.bounds = unit_capsule->bounds;
            pb.collider.shape.capsule.radius = 1.0f;
            pb.collider.shape.capsule.length = 1.0f;
        }

        if (0)
        {
            Name_Id name("cylinder");
            Transform_Component& transform = _transform_components.add(name);
            transform.local_position = vec3(-5.0f, 0.0f, 3.0f);
            transform.local_orientation = quat::from_euler_angles(vec3(0_deg, 0_deg, 0_deg));
            Render_Component& render = _render_components.add(name);
            render.mesh = unit_cylinder;

            Physics_Body_Component& pb = _physics_body_components.add(name);
            pb.type = Physics_Body::Static;
            pb.component_id = name;
            pb.mass = 1.0f;  //Is this valid?
            pb.collider.type = Collider::Cylinder;
            pb.collider.bounds = unit_cylinder->bounds;
            pb.collider.shape.cylinder.radius = 1.0f;
            pb.collider.shape.cylinder.height = 1.0f;
        }

        if (0)
        {
            Name_Id name("box");
            Transform_Component& transform = _transform_components.add(name);
            transform.local_position = vec3(-2.5f, 0.0f, 5.0f);

            Render_Component& render = _render_components.add(name);
            render.mesh = unit_box;

            Physics_Body_Component& pb = _physics_body_components.add(name);
            pb.type = Physics_Body::Dynamic;
            pb.component_id = name;
            pb.mass = 1.0f;  //Is this valid?
            pb.collider.type = Collider::Box;
            pb.collider.bounds = unit_box->bounds;
            pb.collider.shape.bounding_box = unit_box->bounds;
        }

        if (0)
        {
            Name_Id name("convex");
            Transform_Component& transform = _transform_components.add(name);
            transform.local_position = vec3(-2.5f, 0.0f, 10.0f);

            Render_Component& render = _render_components.add(name);
            render.mesh = unit_convex;

            Physics_Body_Component& pb = _physics_body_components.add(name);
            pb.type = Physics_Body::Static;
            pb.component_id = name;
            pb.mass = 1.0f;  //Is this valid?
            pb.collider.type = Collider::Convex_Hull;
            pb.collider.bounds = unit_convex->bounds;
            pb.collider.shape.convex_hull.count = (uint32) unit_convex->submeshes[0].vertices.size();
            assert(pb.collider.shape.convex_hull.count < CONVEX_HULL_MAX_NUM_VERTICES);

            uint32 count = 0;
            for (const Vertex_Data& vertex : unit_convex->submeshes[0].vertices)
            {
                pb.collider.shape.convex_hull.vertices[count++] = vertex.vertex_location;
            }
        }

        //_add_object(std::format("kimono{}", 2).c_str(), vec3(0.0f, 0.0f, 10.0f), quat::zero_quat, sphere_res, Physics_Body::Dynamic);
    }

    float t = 0;
    virtual void pre_physics_update(float dt) override
    {
        PROFILE_FUNCTION()

        t += dt;

        if (t > 10.0f)
        {
            t = 0.0f;
            Physics_System& physics_system = Physics_System::get();
            physics_system.get_body("sphere").apply_impulse(vec3(500.0f, 0.0f, 0.0f));
        }

        //_transform_components.get("kimono")->local_position.x = cosf(t * 3.0f) * 2.5f;
        //_transform_components.get("kimono")->local_position.y = sinf(t * 3.0f) * 2.5f;
        //_transform_components.get("kimono")->local_position.z = 0.0f;
        //_transform_components.get("kimono")->dirty = true;
        
        _update_transform_components();
        _update_physics_components();
    }

    float timer = 0;
    int32 count = 0;
    virtual void post_physics_update(float dt) override
    {
        PROFILE_FUNCTION()

        // Writing to transform, mutex needed
        Physics_System& physics_system = Physics_System::get();
        for (int64 i = 0; i < _physics_body_components.components.size(); ++i)
        {
            Name_Id* p_entity_id = _physics_body_components.index_to_id.find(i);
            assert(p_entity_id);

            Transform_Component* transform_component = _transform_components.get(*p_entity_id);
            assert(transform_component);

            Physics_Body_Component& component = _physics_body_components.components[i];
            Physics_Body& body = physics_system.get_body(component.component_id);
            
            if (body.type != Physics_Body::Dynamic)
            {
                continue;
            }

            if (!body.dirty)
            {
                continue;
            }

            body.dirty = false;
            
            transform_component->local_position = body.transform.position;
            transform_component->local_orientation = body.transform.orientation;
            transform_component->dirty = true;
        }

        timer += dt;
        if (timer > 0.25f)
        {
            timer = 0.0f;
            count++;
            float x = -2.0f + 5.0f * (rand() % 1000) / 1000.0f;
            float y = -2.0f + 5.0f * (rand() % 1000) / 1000.0f;
            
            //_add_object(std::format("dynamic{}", count).c_str(), vec3(x, y, 0.0f), quat::zero_quat, sphere_res, Physics_Body::Dynamic);
        }
    }

    virtual void render(const Shared_Ptr<Renderer>& renderer, VR_Eye::Type eye = VR_Eye::None) override
    {
        PROFILE_FUNCTION()

        std::shared_lock lock(_mutex);
    
        Shared_Ptr<Renderer_Backend> renderer_backend = renderer->backend;
        const Transform_Component* p_camera_transform = _transform_components.get("player_camera");
        if (!p_camera_transform)
        {
          return;
        }
    
        if (VR_System::get().is_valid())
        {
            VR_System::get().set_custom_camera_pose(p_camera_transform->get_world_matrix());
        }
        else
        {
            Shared_Ptr<Camera> camera = renderer->get_active_camera();
            camera->position = p_camera_transform->get_world_position();
            camera->orientation = p_camera_transform->get_world_orientation();
        }
    
        for (int64 i = 0; i < _render_components.components.size(); ++i)
        {
            const Render_Component& render_component = _render_components.components[i];
            if (!render_component.visible)
            {
                continue;
            }

            const Shared_Ptr<Mesh_Resource>& mesh_resource = render_component.mesh;
            const Shared_Ptr<Mesh>& mesh = renderer_backend->get_mesh(mesh_resource);

            Name_Id* p_entity_id = _render_components.index_to_id.find(i);
            assert(p_entity_id);

            if (Transform_Component* transform_component = _transform_components.get(*p_entity_id))
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
    Component_Container<Physics_Body_Component> _physics_body_components;
    Component_Container<Render_Component> _render_components;

    void _update_transform_components()
    {
        PROFILE_FUNCTION()

        // For task graph
        Dynamic_Array<Pair<int64, int64>> dependencies;
        for (int64 i = 0; i < _transform_components.components.size(); ++i)
        {
            Transform_Component& component = _transform_components.components[i];
            bool parent_dirty = false;

            const Transform_Component* parent_transform_component = nullptr;
            if (component.parent_id != empty_name_id)
            {
                parent_transform_component = _transform_components.get(component.parent_id);
                if (parent_transform_component->dirty)
                {
                    parent_dirty = true;

                    const int64* p_parent_index = _transform_components.id_to_index.find(component.parent_id);
                    assert(p_parent_index);

                    dependencies.push_back({i, *p_parent_index});
                }
            }

            if (!parent_dirty && !component.dirty)
            {
                continue;
            }

            if (parent_transform_component)
            {
                component.local_to_world = parent_transform_component->local_to_world * component.local_to_world;
            }
        }

        //Task graph dependencies
        //for (const Pair<int32>& dependency_pair : dependencies)
        //{
        //  tasks[dependency_pair.a]->add_dependency(tasks[dependency_pair.b]);
        //}
    }

    void _update_physics_components()
    {
        PROFILE_FUNCTION()

        Physics_System& physics_system = Physics_System::get();
        for (int32 i = 0; i < _physics_body_components.components.size(); ++i)
        {
            Physics_Body_Component& component = _physics_body_components.components[i];
            
            Name_Id* p_entity_id = _physics_body_components.index_to_id.find(i);
            assert(p_entity_id);

            const Transform_Component* transform_component = _transform_components.get(*p_entity_id);
            assert(transform_component);
            
            const vec3 current_world_position = transform_component->get_world_position();

            // This assigns the body the component_id
            Physics_Body& body = physics_system.get_body(component.component_id);
            if (body.type == Physics_Body::None)
            {
                body.type = component.type;
                
                body.transform.position = current_world_position;
                body.transform.orientation = transform_component->get_world_orientation();
    
                body.collider = component.collider;
                if (body.type == Physics_Body::Static)
                {
                    body.inverse_mass = 0.0f;
                }
                else
                {
                    body.inverse_mass = 1.0f / component.mass;
                    body.inverse_inertia_tensor = Collision_Helpers::inertia_tensor(body.collider, component.mass);
                }

                //body.apply_impulse(vec3(100.0f, 0.0f, 0.0f));
            }

            if (body.type != Physics_Body::Kinematic ||
                current_world_position.nearly_equal(body.transform.position))
            {
                if (body.type == Physics_Body::Dynamic)
                {
                    body.apply_force(vec3(0.0f, 0.0f, -9.81f));
                }

                continue;
            }

            body.transform.position = current_world_position;
            body.transform.orientation = transform_component->get_world_orientation();
        }
    }
};

int main(int argc, char** argv)
{
    Dynamic_Array<std::string> args;
    for (int32 a = 1; a < argc; ++a)
    {
        args.push_back(argv[a]);
    }

    args.push_back("cs_dt_mult=1.0");
    args.push_back("cs_vr_support=0");
    args.push_back("cs_num_threads=0");

    Engine engine;
    engine.initialize(args);
    engine.game_instance = Shared_Ptr<Game>::create();
    engine.run();
    engine.shutdown();

    Profiler::get().write_to_chrometracing_json("profiling/angle.json");

  return 0;
}