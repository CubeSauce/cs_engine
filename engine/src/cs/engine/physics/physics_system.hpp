// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/math.hpp"
#include "cs/engine/name_id.hpp"
#include "cs/containers/dynamic_array.hpp"
#include "cs/engine/profiling/profiler.hpp"
#include "cs/containers/spatial_hash_grid.hpp"

#include <unordered_map>

struct Physics_Body
{
    enum Type
    {
        // Static objects that won't change in shape, transform etc.
        Static,
        // Movable objects that are controlled by us
        Kinematic,
        // Movable objects that are controlled by the physics engine
        Dynamic
    };

    Name_Id id;
    Type type;

    struct Transform
    {
        vec3 position;
        quat orientation;
    } transform;

    struct State
    {
        vec3 velocity;
        vec3 angular_velocity;
        
        /*
        vec3 acceleration;
        vec3 angular_acceleration;
        */
    } state;

    Box aabb_bounds;

    Box get_transformed_bounds() const;
};

class Physics_System : public Singleton<Physics_System>
{
public:
    Physics_Body& get_body(const Name_Id& in_id);

    void update(float dt);

private:
    Dynamic_Array<Physics_Body> _bodies;
    std::unordered_map<uint32, int32> _id_to_index;

    Spatial_Hash_Grid _hash_grid = Spatial_Hash_Grid(3.0f);

    std::unordered_map<uint32, Dynamic_Array<Name_Id>> _broadphase_collisions;
    std::unordered_map<uint32, Dynamic_Array<Name_Id>> _narrowphase_collisions;

    void _execute_broadphase();
    void _execute_narrowphase();
    void _resolve_collisions(float dt);
};
