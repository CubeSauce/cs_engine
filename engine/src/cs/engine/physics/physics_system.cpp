// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/engine/physics/physics_system.hpp"
#include "cs/engine/renderer/renderer.hpp"

Box Physics_Body::get_transformed_bounds() const
{
    return { collider.bounds.min + transform.position, collider.bounds.max + transform.position };
}

void Physics_Body::update_state(float dt)
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

void Physics_Body::update_transform(float dt)
{
    if (type != Dynamic)
    {
        return;
    }

    transform.position += state.velocity * dt;
    dirty = true;
}

void Physics_Body::apply_force(const vec3& force)
{
    if (type != Dynamic)
    {
        return;
    }

    state.accumulated_forces += force;
}

void Physics_Body::apply_impulse(const vec3& impulse)
{
    if (type != Dynamic)
    {
        return;
    }

    state.accumulated_forces += impulse * inverse_mass;
}

void Physics_Body::clear_forces()
{
    state.accumulated_forces = vec3::zero_vector;
}

template<> 
Physics_System* Singleton<Physics_System>::_singleton { nullptr };

void Physics_System::initialize()
{
    _init_collision_functions();
}

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
    
    _execute_broadphase(dt);
    _execute_narrowphase(dt);
    _resolve_collisions(dt);
}

void Physics_System::render_physics_bodies()
{
    Shared_Ptr<Renderer_Backend> renderer_backend = Renderer::get().backend;
}

void Physics_System::_init_collision_functions()
{
    _collision_functions[Collider::Sphere][Collider::Sphere] = Collision_Test_Function::sphere_sphere;
    _collision_functions[Collider::Sphere][Collider::Capsule] = Collision_Test_Function::sphere_capsule;
    _collision_functions[Collider::Capsule][Collider::Sphere] = Collision_Test_Function::capsule_sphere;
    _collision_functions[Collider::Capsule][Collider::Capsule] = Collision_Test_Function::capsule_capsule;
    _collision_functions[Collider::Convex_Hull][Collider::Convex_Hull] = Collision_Test_Function::convex_convex;
}

void Physics_System::_execute_broadphase(float dt)
{
    PROFILE_FUNCTION()

    _broadphase_collisions.clear();

    //TODO: paralelize
    for (Physics_Body& body : _bodies)
    {
        body.update_state(dt);

        _hash_grid.update(body.id, body.get_transformed_bounds());
    }

    for (const Physics_Body& body : _bodies)
    {
        _hash_grid.get_potential_collisions(body.id, body.get_transformed_bounds(), _broadphase_collisions[body.id]);
    }
}

void Physics_System::_execute_narrowphase(float dt)
{
    PROFILE_FUNCTION()

    _narrowphase_collisions.clear();

    for (const auto& potential_collisions : _broadphase_collisions)
    {
        const uint32 this_hash = potential_collisions.first;
        
        const int32 this_index = _id_to_index[this_hash];
        Physics_Body& this_body = _bodies[this_index];

        for (const Name_Id& other_id : potential_collisions.second)
        {
            const int32 other_index = _id_to_index[other_id];
            Physics_Body& other_body = _bodies[other_index];

            Collision_Test_Function::Definition f = _collision_functions[this_body.collider.type][other_body.collider.type];
            if (f == nullptr)
            {
                printf("Can't find collision test function for given collider types.\n");
                continue;
            }

            Collision_Result result;
            result.a_index = this_index;
            result.b_index = other_index;
            if (f(this_body.collider, this_body.transform.position, this_body.transform.orientation, 
                other_body.collider, other_body.transform.position, other_body.transform.orientation, 
                result.normal, result.penetration))
            {
                _narrowphase_collisions[this_hash].add(result);
            }
        }
    }
}

void Physics_System::_resolve_collisions(float dt)
{
    PROFILE_FUNCTION()
    
    for (const auto& potential_collisions : _narrowphase_collisions)
    {
        for (const Collision_Result& collision : potential_collisions.second)
        {
            Physics_Body& this_body =  _bodies[collision.a_index];
            Physics_Body& other_body = _bodies[collision.b_index];

            _resolve_collision(this_body, other_body, collision);
        }
    }

    for (Physics_Body& body : _bodies)
    {
        body.update_transform(dt);
    }

    for (Physics_Body& body : _bodies)
    {
        body.clear_forces();
    }
}

void Physics_System::_resolve_collision(Physics_Body& a, Physics_Body& b, const Collision_Result& collision)
{
    // Position correction vectors
    const float inverse_total_mass = a.inverse_mass + b.inverse_mass;
    if (inverse_total_mass > 0.0f)
    {
        const vec3 c = collision.normal * (collision.penetration / inverse_total_mass);

        if (a.type == Physics_Body::Dynamic)
        {
            a.transform.position -= c * a.inverse_mass;
            a.dirty = true;
        } 
        if (b.type == Physics_Body::Dynamic)
        {
            b.transform.position += c * b.inverse_mass;
            b.dirty = true;
        } 
    }

    const vec3 relative_velocity = b.state.velocity - a.state.velocity;
    const float normal_velocity = relative_velocity.dot(collision.normal);

    // Already separated
    if (normal_velocity > 0) return;

    const float impulse_magnitude = -(1 + a.restitution) * normal_velocity / inverse_total_mass;
    const vec3 impulse = collision.normal * impulse_magnitude;

    if (a.type == Physics_Body::Dynamic)
    {
        a.state.velocity -= impulse * a.inverse_mass;
        a.dirty = true;
    } 
    if (b.type == Physics_Body::Dynamic)
    {
        b.state.velocity += impulse * b.inverse_mass;
        b.dirty = true;
    } 
}
