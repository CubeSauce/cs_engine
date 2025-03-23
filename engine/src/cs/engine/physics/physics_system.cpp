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
        linear_velocity += accumulated_forces * dt;        
        if (linear_velocity.length_squared() > max_linear_velocity * max_linear_velocity)
        {
            linear_velocity = linear_velocity.normalize() * max_linear_velocity;
        }

        const mat4 rot = transform.orientation.to_mat4();
        angular_velocity += (rot * inverse_inertia_tensor * rot.transposed()) * accumulated_torque * dt;
        
        if (angular_velocity.length_squared() > max_angular_velocity * max_angular_velocity)
        {
            angular_velocity = angular_velocity.normalize() * max_angular_velocity;
        }
    }
    else if (type == Kinematic)
    {
        linear_velocity = (transform.position - old_transform.position) / dt;
        const quat delta = transform.orientation.mul(old_transform.orientation.conjugate()).normalized();
        const float theta = 2.0f * acosf(delta.w);
        const float s2t = sinf(theta * 0.5f);
        if (!is_nearly_equal(s2t, 0))
        {
            angular_velocity = (delta.v / s2t) * (theta / dt);
        }
        else
        {
            angular_velocity = vec3::zero_vector;
        }
        old_transform = transform;
    }

    accumulated_forces = vec3::zero_vector;
    accumulated_torque = vec3::zero_vector;
}

void Physics_Body::update_transform(float dt)
{
    if (type != Dynamic)
    {
        return;
    }

    transform.position += linear_velocity * dt;
    transform.orientation = transform.orientation.mul(quat(angular_velocity * 0.5f * dt, 1.0f));
    transform.orientation.normalize();

    dirty = true;
}

void Physics_Body::apply_force(const vec3& force)
{
    if (type != Dynamic)
    {
        return;
    }

    accumulated_forces += force;
    dirty = true;
}

void Physics_Body::apply_impulse(const vec3& impulse)
{
    if (type != Dynamic)
    {
        return;
    }

    accumulated_forces += impulse * inverse_mass;
    dirty = true;
}

void Physics_Body::apply_force_at_offset(const vec3& force, const vec3& force_offset)
{
    if (type != Dynamic)
    {
        return;
    }

    accumulated_forces += force;
    accumulated_torque += (force_offset - center_of_mass).cross(force);
    dirty = true;
}

void Physics_Body::apply_impulse_at_offset(const vec3& impulse, const vec3& force_offset)
{
    if (type != Dynamic)
    {
        return;
    }

    accumulated_forces += impulse * inverse_mass;
    accumulated_torque += (force_offset - center_of_mass).cross(impulse * inverse_mass);
    dirty = true;
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

    const vec3 vel_a = a.linear_velocity + a.angular_velocity.cross(r_a);
    const vec3 vel_b = b.linear_velocity + b.angular_velocity.cross(r_b);
    const vec3 relative_velocity = vel_b - vel_a;
    const float normal_velocity = relative_velocity.dot(collision.normal);

    // Already separated
    if (normal_velocity > 0) 
    {
        return;
    }

    const mat4& a_rot = a.transform.orientation.to_mat4();
    const mat4& iit_a = a_rot * a.inverse_inertia_tensor * a_rot.transposed();
    const mat4& b_rot = b.transform.orientation.to_mat4();
    const mat4& iit_b = b_rot * b.inverse_inertia_tensor * b_rot.transposed();

    const float denominator = inverse_total_mass + 
        r_a.cross(collision.normal).dot(iit_a * r_a.cross(collision.normal)) +
        r_b.cross(collision.normal).dot(iit_b * r_b.cross(collision.normal));

    // Stiffer body regulates outcome
    const float impulse_magnitude = -(1 + std::min(a.restitution, b.restitution)) * normal_velocity / denominator;

    vec3 impulse_vector = collision.normal * impulse_magnitude;
    if (a.type == Physics_Body::Dynamic)
    {
        a.linear_velocity -= impulse_vector * a.inverse_mass;
        a.angular_velocity -= iit_a * r_a.cross(impulse_vector);
        a.dirty = true;
    } 
    if (b.type == Physics_Body::Dynamic)
    {
        b.linear_velocity += collision.normal * impulse_magnitude * b.inverse_mass;
        b.angular_velocity += iit_b * r_b.cross(impulse_vector);
        b.dirty = true;
    } 
    
    const vec3 tangent_velocity = relative_velocity - collision.normal * relative_velocity.dot(collision.normal);
    if (tangent_velocity.length_squared() < 1e-8f)
    {
        //Static friction
        return;
    }

    const vec3 tangent = -tangent_velocity.normalized();
    const vec3 tangent_inertia_a = (iit_a * r_a.cross(tangent)).cross(r_a);
    const vec3 tangent_inertia_b = (iit_b * r_b.cross(tangent)).cross(r_b);

    const float effective_mass = inverse_total_mass  + (tangent_inertia_a + tangent_inertia_b).dot(tangent);
    if (is_nearly_equal(effective_mass, 0.0f))
    {
        return;
    }

    const float static_friction = 0.5f; //TODO: where?
    const float max_s_friction_impulse = impulse_magnitude * static_friction;
    const float j_t = - relative_velocity.dot(tangent) / effective_mass;

    vec3 friction_impulse;
    if (fabs(j_t) < max_s_friction_impulse)
    {
        // Static friction
        friction_impulse = tangent * j_t;
    }
    else
    {
        // Dynamic friction
        const float kinetic_friction = 0.3f; //TODO: where?
        friction_impulse = tangent * (kinetic_friction * impulse_magnitude);
    }    
    
    if (a.type == Physics_Body::Dynamic)
    {
        const vec3 torque = r_a.cross(friction_impulse);
        const vec3 ang_vel = iit_a * torque;
        a.angular_velocity -= ang_vel;
        a.dirty = true;
    } 
    if (b.type == Physics_Body::Dynamic)
    {
        const vec3 torque = r_b.cross(friction_impulse);
        const vec3 ang_vel = iit_b * torque;
        b.angular_velocity -= ang_vel;
        b.dirty = true;
    } 
}
