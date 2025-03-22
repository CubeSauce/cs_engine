// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/engine/physics/physics_system.hpp"
#include "cs/engine/renderer/renderer.hpp"

AABB Physics_Body::get_transformed_bounds() const
{
    return { collider.bounds.min + transform.position, collider.bounds.max + transform.position };
}

void Physics_Body::update_state(float dt)
{
    if (type == Dynamic)
    {
        state.velocity += state.accumulated_forces * dt;
        state.angular_velocity += inverse_inertia_tensor * state.accumulated_torque * dt;
    }
    else if (type == Kinematic)
    {
        state.velocity = (transform.position - old_transform.position) / dt;
        const quat delta = transform.orientation.mul(old_transform.orientation.conjugate()).normalized();
        const float theta = 2.0f * acosf(delta.w);
        const float s2t = sinf(theta * 0.5f);
        if (!is_nearly_equal(s2t, 0))
        {
            state.angular_velocity = (delta.v / s2t) * (theta / dt);
        }
        else
        {
            state.angular_velocity = vec3::zero_vector;
        }
        old_transform = transform;
    }

    state.accumulated_forces = vec3::zero_vector;
    state.accumulated_torque = vec3::zero_vector;
}

void Physics_Body::update_transform(float dt)
{
    if (type != Dynamic)
    {
        return;
    }

    transform.position += state.velocity * dt;
    transform.orientation = transform.orientation.mul(quat(state.angular_velocity * 0.5f * dt, 1.0f));
    transform.orientation.normalize();

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

void Physics_Body::apply_force_at_offset(const vec3& force, const vec3& force_offset)
{
    if (type != Dynamic)
    {
        return;
    }

    state.accumulated_forces += force;
    state.accumulated_torque += (force_offset - center_of_mass).cross(force);
}

void Physics_Body::apply_impulse_at_offset(const vec3& impulse, const vec3& force_offset)
{
    if (type != Dynamic)
    {
        return;
    }

    state.accumulated_forces += impulse * inverse_mass;
    state.accumulated_torque += (force_offset - center_of_mass).cross(impulse * inverse_mass);
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
    _collision_functions[Collider::Sphere][Collider::Cylinder] = Collision_Test_Function::sphere_cylinder;
    _collision_functions[Collider::Sphere][Collider::Box] = Collision_Test_Function::sphere_box;
    _collision_functions[Collider::Sphere][Collider::Convex_Hull] = Collision_Test_Function::sphere_convex;
    _collision_functions[Collider::Capsule][Collider::Sphere] = Collision_Test_Function::capsule_sphere;
    _collision_functions[Collider::Capsule][Collider::Capsule] = Collision_Test_Function::capsule_capsule;
    _collision_functions[Collider::Capsule][Collider::Cylinder] = Collision_Test_Function::capsule_cylinder;
    _collision_functions[Collider::Capsule][Collider::Box] = Collision_Test_Function::capsule_box;
    _collision_functions[Collider::Capsule][Collider::Convex_Hull] = Collision_Test_Function::capsule_convex;
    _collision_functions[Collider::Cylinder][Collider::Sphere] = Collision_Test_Function::cylinder_sphere;
    _collision_functions[Collider::Cylinder][Collider::Capsule] = Collision_Test_Function::cylinder_capsule;
    _collision_functions[Collider::Cylinder][Collider::Cylinder] = Collision_Test_Function::cylinder_cylinder;
    _collision_functions[Collider::Cylinder][Collider::Box] = Collision_Test_Function::cylinder_box;
    _collision_functions[Collider::Cylinder][Collider::Convex_Hull] = Collision_Test_Function::cylinder_convex;
    _collision_functions[Collider::Box][Collider::Sphere] = Collision_Test_Function::box_sphere;
    _collision_functions[Collider::Box][Collider::Capsule] = Collision_Test_Function::box_capsule;
    _collision_functions[Collider::Box][Collider::Cylinder] = Collision_Test_Function::box_cylinder;
    _collision_functions[Collider::Box][Collider::Box] = Collision_Test_Function::box_box;
    _collision_functions[Collider::Box][Collider::Convex_Hull] = Collision_Test_Function::box_convex;
    _collision_functions[Collider::Convex_Hull][Collider::Sphere] = Collision_Test_Function::convex_sphere;
    _collision_functions[Collider::Convex_Hull][Collider::Capsule] = Collision_Test_Function::convex_capsule;
    _collision_functions[Collider::Convex_Hull][Collider::Cylinder] = Collision_Test_Function::convex_cylinder;
    _collision_functions[Collider::Convex_Hull][Collider::Box] = Collision_Test_Function::convex_box;
    _collision_functions[Collider::Convex_Hull][Collider::Convex_Hull] = Collision_Test_Function::convex_convex;
}

void Physics_System::_execute_broadphase(float dt)
{
    PROFILE_FUNCTION()

    //TODO: paralelize
    for (Physics_Body& body : _bodies)
    {
        body.update_state(dt);

        _hash_grid.update(body.id, body.get_transformed_bounds());
    }

    _hash_grid.sweep_and_prune_cells(_broadphase_collision_pairs);
}

void Physics_System::_execute_narrowphase(float dt)
{
    PROFILE_FUNCTION()

    _narrowphase_collisions.clear();

    for (const Pair<Name_Id, Name_Id>& collision_pair : _broadphase_collision_pairs)
    {
        const int32 this_index = _id_to_index[collision_pair.a];
        Physics_Body& this_body = _bodies[this_index];
        const int32 other_index = _id_to_index[collision_pair.b];
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
            result))
        {
            _narrowphase_collisions[collision_pair.a].add(result);
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
}

void Physics_System::_resolve_collision(Physics_Body& a, Physics_Body& b, const Collision_Result& collision)
{
    // Position correction vectors
    const float inverse_total_mass = a.inverse_mass + b.inverse_mass;
    if (inverse_total_mass > 0.0f)
    {
        // Apply a fraction of position correction
        const vec3 c = collision.normal * 0.1f * (collision.penetration / inverse_total_mass);

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

    const vec3 r_a = collision.contact_point - a.transform.position;
    const vec3 r_b = collision.contact_point - b.transform.position;

    const vec3 vel_a = a.state.velocity + a.state.angular_velocity.cross(r_a);
    const vec3 vel_b = b.state.velocity + b.state.angular_velocity.cross(r_b);
    const vec3 relative_velocity = vel_b - vel_a;
    const float normal_velocity = relative_velocity.dot(collision.normal);

    // Already separated
    if (normal_velocity > 0) 
    {
        return;
    }

    const mat4& iit_a = a.inverse_inertia_tensor;
    const mat4& iit_b = b.inverse_inertia_tensor;

    const float denominator = inverse_total_mass + 
        r_a.cross(collision.normal).dot(iit_a * r_a.cross(collision.normal)) +
        r_b.cross(collision.normal).dot(iit_b * r_b.cross(collision.normal));

    // Stiffer body regulates outcome
    const float impulse_magnitude = -(1 + std::min(a.restitution, b.restitution)) * normal_velocity / denominator;

    vec3 impulse_vector = collision.normal * impulse_magnitude;
    if (a.type == Physics_Body::Dynamic)
    {
        a.state.velocity -= impulse_vector * a.inverse_mass;
        a.state.angular_velocity -= iit_a * r_a.cross(impulse_vector);
        a.dirty = true;
    } 
    if (b.type == Physics_Body::Dynamic)
    {
        b.state.velocity += collision.normal * impulse_magnitude * b.inverse_mass;
        b.state.angular_velocity += iit_b * r_b.cross(impulse_vector);
        b.dirty = true;
    } 
}
