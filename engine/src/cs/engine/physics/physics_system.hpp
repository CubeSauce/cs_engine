// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/math.hpp"
#include "cs/engine/name_id.hpp"
#include "cs/containers/dynamic_array.hpp"
#include "cs/engine/profiling/profiler.hpp"
#include "cs/containers/spatial_hash_grid.hpp"
#include "cs/engine/physics/collision_function.hpp"

#include <unordered_map>

struct Sphere_Shape
{
    float radius;
};

struct Capsule_Shape
{
    float radius;
    float length;
};

struct Collider
{
    enum Type
    {
        Sphere,
        Capsule,
        Cylinder,
        OBB,
        Convex_Hull,
        TYPE_COUNT
    };

    Type type;
    union {
        Sphere_Shape sphere;
        Capsule_Shape capsule;
    } shape;

    Collider& operator=(const Collider& other);

    Box get_bounds() const
    {
        switch(type)
        {
        case Sphere:{
            return {
                -vec3(shape.sphere.radius),
                 vec3(shape.sphere.radius)
            };
        }
        case Capsule:{
            return {
                vec3(0.0f, 0.0f, -shape.capsule.length * 0.5f) - vec3(shape.sphere.radius),
                vec3(0.0f, 0.0f, +shape.capsule.length * 0.5f) + vec3(shape.sphere.radius)
            };
            break;
        }
        case Cylinder:
        case OBB:
        case Convex_Hull:
        default:
            return Box();
        }
    }
};

struct Physics_Body
{
    enum Type
    {
        None,
        // Static objects that won't change in shape, transform etc.
        Static,
        // Movable objects that are controlled by us
        Kinematic,
        // Movable objects that are controlled by the physics engine
        Dynamic
    };

    Name_Id id;
    Type type { None };

    float inverse_mass { 1.0f };
    // 0 -> inelastic, 1 -> perfect elastic
    float restitution { 0.5f };

    struct Transform
    {
        vec3 position { vec3::zero_vector };
        quat orientation { quat::zero_quat };
    } transform, old_transform;

    struct State
    {
        vec3 accumulated_forces { vec3::zero_vector };

        vec3 velocity { vec3::zero_vector };
        vec3 angular_velocity { vec3::zero_vector };
        
        /*
        vec3 acceleration;
        vec3 angular_acceleration;
        */
    } state;

    Collider collider;

    bool dirty;

    Box get_transformed_bounds() const;

    Physics_Body& operator=(const Physics_Body& other);

    void update_state(float dt)
    {
        if (type == Dynamic)
        {
            state.velocity += state.accumulated_forces * dt;
        }
        else if (type == Kinematic)
        {
            state.velocity = (transform.position - old_transform.position) / dt;
            old_transform = transform;
        }
    }

    void update_transform(float dt)
    {
        if (type != Dynamic)
        {
            return;
        }

        transform.position += state.velocity * dt;
        dirty = true;
    }

    void apply_force(const vec3& force)
    {
        if (type != Dynamic)
        {
            return;
        }

        state.accumulated_forces += force;
    }

    void apply_impulse(const vec3& impulse)
    {
        if (type != Dynamic)
        {
            return;
        }

        state.accumulated_forces += impulse * inverse_mass;
    }

    void clear_forces()
    {
        state.accumulated_forces = vec3::zero_vector;
    }
};

struct Collision_Result
{
    int32 a_index { -1 }, b_index { -1 };
    vec3 normal { vec3::zero_vector };
    float penetration { 0.0f };
};

class Physics_System : public Singleton<Physics_System>
{
public:
    Physics_Body& get_body(const Name_Id& in_id);

    void initialize();
    void update(float dt);

private:
    Dynamic_Array<Physics_Body> _bodies;
    std::unordered_map<uint32, int32> _id_to_index;

    Spatial_Hash_Grid _hash_grid = Spatial_Hash_Grid(3.0f);

    void _init_collision_functions();

    std::unordered_map<uint32, Dynamic_Array<Name_Id>> _broadphase_collisions;
    std::unordered_map<uint32, Dynamic_Array<Collision_Result>> _narrowphase_collisions;

    void _execute_broadphase(float dt);
    void _execute_narrowphase(float dt);
    void _resolve_collisions(float dt);
    
    Collision_Test_Function::Definition _collision_functions[Collider::TYPE_COUNT][Collider::TYPE_COUNT];
    Collision_Resolve_Function::Definition _resolve_functions[Collider::TYPE_COUNT][Collider::TYPE_COUNT];
};
