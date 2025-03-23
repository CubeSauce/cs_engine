// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/math.hpp"
#include "cs/containers/dynamic_array.hpp"
#include "cs/containers/spatial_hash_grid.hpp"
#include "cs/engine/name_id.hpp"
#include "cs/engine/profiling/profiler.hpp"
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

#define CONVEX_HULL_MAX_NUM_VERTICES 512
struct Convex_Hull_Shape
{
    int32 count;
    vec3 vertices[CONVEX_HULL_MAX_NUM_VERTICES];
    float volume;
};

struct Collider
{
    enum Type
    {
        Sphere,
        Capsule,
        Cylinder,
        Box,
        Convex_Hull,
        TYPE_COUNT
    };

    Type type;
    union {
        Sphere_Shape sphere;
        Capsule_Shape capsule;
        struct
        {
            float radius;
            float height;
        } cylinder;
        AABB bounding_box;
        Convex_Hull_Shape convex_hull;
    } shape;
    AABB bounds;
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

    vec3 center_of_mass { vec3::zero_vector };
    mat4 inverse_inertia_tensor { mat4(1.0f) };

    float inverse_mass { 1.0f };
    // 0 -> inelastic, 1 -> perfect elastic
    float restitution { 0.5f };

    struct Transform
    {
        vec3 position { vec3::zero_vector };
        quat orientation { quat::zero_quat };
    } transform, old_transform;

    float max_linear_velocity = FLT_MAX;
    vec3 accumulated_forces { vec3::zero_vector };
    vec3 linear_velocity { vec3::zero_vector };

    float max_angular_velocity = FLT_MAX;
    vec3 accumulated_torque { vec3::zero_vector };
    vec3 angular_velocity { vec3::zero_vector };

    Collider collider;

    bool dirty;

    AABB get_transformed_bounds() const;

    void update_state(float dt);
    void update_transform(float dt);

    // Does not produce torque
    void apply_force(const vec3& force);
    void apply_impulse(const vec3& impulse);
    // Produces torque
    void apply_force_at_offset(const vec3& force, const vec3& force_offset);
    void apply_impulse_at_offset(const vec3& impulse, const vec3& force_offset);
};

struct Collision_Result
{
    int32 a_index { -1 }, b_index { -1 };
    vec3 normal { vec3::zero_vector };
    // World space
    vec3 contact_point { vec3::zero_vector };
    float penetration { 0.0f };
};

class Physics_System : public Singleton<Physics_System>
{
public:
    Physics_Body& get_body(const Name_Id& in_id);

    void initialize();
    void update(float dt);

    void render_physics_bodies();

private:
    Dynamic_Array<Physics_Body> _bodies;
    std::unordered_map<uint32, int32> _id_to_index;

    Spatial_Hash_Grid _hash_grid = Spatial_Hash_Grid(1.50f);

    void _init_collision_functions();

    
    Dynamic_Array<Pair<Name_Id, Name_Id>> _broadphase_collision_pairs;
    // std::unordered_map<uint32, Dynamic_Array<Name_Id>> _broadphase_collisions;
    std::unordered_map<uint32, Dynamic_Array<Collision_Result>> _narrowphase_collisions;

    void _execute_broadphase(float dt);
    void _execute_narrowphase(float dt);
    void _resolve_collisions(float dt);
    void _resolve_collision(Physics_Body& a, Physics_Body& b, const Collision_Result& collision);
    
    Collision_Test_Function::Definition _collision_functions[Collider::TYPE_COUNT][Collider::TYPE_COUNT];

};
