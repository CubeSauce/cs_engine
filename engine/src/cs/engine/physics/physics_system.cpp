// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/engine/physics/physics_system.hpp"

Box Physics_Body::get_transformed_bounds() const
{
    return { aabb_bounds.min + transform.position, aabb_bounds.max + transform.position };
}

template<> 
Physics_System* Singleton<Physics_System>::_singleton { nullptr };

Physics_Body& Physics_System::get_body(const Name_Id& in_id)
{
    if (_id_to_index.find(in_id) != _id_to_index.end())
    {
        return _bodies[_id_to_index[in_id]];
    }

    int32 new_index = _bodies.size();
    _id_to_index[in_id] = new_index;
    
    Physics_Body body;
    body.id = in_id;

    _bodies.add(body);
    return _bodies[new_index];
}

void Physics_System::update(float dt)
{
    PROFILE_FUNCTION()
    
    _execute_broadphase();
    _execute_narrowphase();
    _resolve_collisions(dt);
}

void Physics_System::_execute_broadphase()
{
    PROFILE_FUNCTION()

    //TODO: paralelize
    _broadphase_collisions.clear();

    for (const Physics_Body& body : _bodies)
    {
        _hash_grid.update(body.id, body.get_transformed_bounds());
    }

    for (const Physics_Body& body : _bodies)
    {
        _hash_grid.get_potential_collisions(body.id, body.get_transformed_bounds(), _broadphase_collisions[body.id]);
    }
}

void Physics_System::_execute_narrowphase()
{
    PROFILE_FUNCTION()

    for (const auto& potential_collisions : _broadphase_collisions)
    {
        const uint32 this_id = potential_collisions.first;
        const int32 this_index = _id_to_index[this_id];

        Physics_Body& this_body = _bodies[this_index];

        for (const Name_Id& collider_id : potential_collisions.second)
        {
            const int32 collider_index = _id_to_index[collider_id];
            Physics_Body& collider_body = _bodies[collider_index];

            _narrowphase_collisions;
        }
    }
}

void Physics_System::_resolve_collisions(float dt)
{
    PROFILE_FUNCTION()
    
    for (const auto& potential_collisions : _narrowphase_collisions)
    {
        //do
    }
}
