// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/engine/physics/physics_system.hpp"
#include "cs/engine/renderer/renderer.hpp"

AABB Physics_Body::get_transformed_bounds() const
{
    vec3 center = collider.bounds.get_center();
    vec3 half_extents = collider.bounds.get_half_extents();
    center += transform.position;
    const mat4 rot = transform.orientation.to_mat4();

    vec3 new_half_extents(
        fabs(rot[0].x) * half_extents.x + fabs(rot[1].x) * half_extents.y + fabs(rot[2].x) * half_extents.z,
        fabs(rot[0].y) * half_extents.x + fabs(rot[1].y) * half_extents.y + fabs(rot[2].y) * half_extents.z,
        fabs(rot[0].z) * half_extents.x + fabs(rot[1].z) * half_extents.y + fabs(rot[2].z) * half_extents.z 
    );

    return { center - new_half_extents, center + new_half_extents };
}

void Physics_Body::update_state(float dt)
{
    if (!is_awake)
    {
        return;
    }

    const float linear_v_sq = linear_velocity.length_squared();
    const float angular_v_sq = angular_velocity.length_squared();

    if (type == Dynamic)
    {
        linear_velocity += accumulated_forces * dt;
        if (linear_v_sq > max_linear_velocity * max_linear_velocity)
        {
            linear_velocity = linear_velocity.normalize() * max_linear_velocity;
        }

        const mat4 rot = transform.orientation.to_mat4();
        angular_velocity += (rot * inverse_inertia_tensor * rot.transposed()) * accumulated_torque * dt;
        
        if (angular_v_sq > max_angular_velocity * max_angular_velocity)
        {
            angular_velocity = angular_velocity.normalize() * max_angular_velocity;
        }
        //printf("%s %s\n", linear_velocity.to_string("lv").c_str(), angular_velocity.to_string("lv").c_str());
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

void Physics_Body::update_transform_euler(float dt)
{
    if (type != Dynamic)
    {
        return;
    }
    
    transform.position += linear_velocity * dt;
    transform.orientation = transform.orientation.mul(quat(angular_velocity * 0.5f * dt, 1.0f));
    transform.orientation.normalize();

    dirty = true;

    const float linear_v_sq = linear_velocity.length_squared();
    const float angular_v_sq = angular_velocity.length_squared();

    if (linear_v_sq > (sleep_linear_velocity_threshold * sleep_linear_velocity_threshold) ||
    angular_v_sq > (sleep_angular_velocity_threshold * sleep_angular_velocity_threshold) )
    {
        wake_up();
    }
    else if (is_awake)
    {
        sleep_timer += dt;
        //printf("sleep_timer: %f\n", sleep_timer);
        if (sleep_timer >= sleep_time_threshold)
        {
            printf("%s is now asleep.\n", id.str);
            is_awake = false;
            linear_velocity = vec3::zero_vector;
            angular_velocity = vec3::zero_vector;
        }
    }
}

void Physics_Body::wake_up()
{
    if (is_awake)
    {
        // Don't reset the timer if already awake
        return;
    }
    
    printf("%s is now awake.\n", id.str);

    is_awake = true;
    sleep_timer = 0.0f;
    // Reset any accumulated forces
    accumulated_forces = vec3::zero_vector;
    accumulated_torque = vec3::zero_vector;
    printf("%s is now awake.\n", id.str);
}

void Physics_Body::apply_force(const vec3& force)
{
    if (type != Dynamic || !is_awake)
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

    wake_up();
    accumulated_forces += impulse * inverse_mass;
    dirty = true;
}

void Physics_Body::apply_force_at_offset(const vec3& force, const vec3& force_offset)
{
    if (type != Dynamic && !is_awake)
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

    wake_up();
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
        if (body.is_awake)
        {
            body.update_state(dt);
        }

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

        if (!this_body.is_awake && !other_body.is_awake)
        {
            continue;
        }

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
    
    for (const auto& collision_pairs : _narrowphase_collisions)
    {
        for (const Collision_Result& collision : collision_pairs.second)
        {
            Physics_Body& this_body =  _bodies[collision.a_index];
            Physics_Body& other_body = _bodies[collision.b_index];

            if (!this_body.is_awake && !other_body.is_awake)
            {
                continue;
            }

            _resolve_collision(this_body, other_body, collision);
            _position_correction(this_body, other_body, collision);
        }
    }

    for (Physics_Body& body : _bodies)
    {
        body.update_transform_euler(dt);
    }
}

void Physics_System::_resolve_collision(Physics_Body& a, Physics_Body& b, const Collision_Result& collision)
{
    const vec3 r_a = collision.contact_point - a.transform.position;
    const vec3 r_b = collision.contact_point - b.transform.position;

    const vec3 v_a = a.linear_velocity + a.angular_velocity.cross(r_a);
    const vec3 v_b = b.linear_velocity + b.angular_velocity.cross(r_b);

    const vec3 v_relative = v_a - v_b;
    const float v_normal = v_relative.dot(collision.normal);

    //printf("a:%s->b:%s v_normal %f\n", a.id.str, b.id.str, v_normal);

    // No changes needed as the bodies are separating.
    if (v_normal < 0) return;

    const float restitution = std::min(a.restitution, b.restitution);

    const vec3 r_a_n = r_a.cross(collision.normal);
    const vec3 r_b_n = r_b.cross(collision.normal);

    const vec3 term_a = (a.inverse_inertia_tensor * r_a_n).cross(r_a_n);
    const vec3 term_b = (b.inverse_inertia_tensor * r_b_n).cross(r_b_n);
    
    const float total_inverse_mass = a.inverse_mass + b.inverse_mass;
    const float angular_term = collision.normal.dot(term_a + term_b);
    const float denominator = total_inverse_mass + angular_term;
    const float j = -(1 + restitution) * v_normal / denominator;

    //printf("a:%s->b:%s j:%f\n", a.id.str, b.id.str, j);
    if (fabs(j) > 0.1)
    {
        a.wake_up();
        b.wake_up();
    }

    const vec3 impulse = collision.normal * j;
    if (a.type == Physics_Body::Dynamic)
    {
        a.linear_velocity += impulse * a.inverse_mass;
        a.angular_velocity += a.inverse_inertia_tensor * r_a.cross(impulse);
    }
    if (b.type == Physics_Body::Dynamic)
    {
        b.linear_velocity -= impulse * b.inverse_mass;
        b.angular_velocity -= b.inverse_inertia_tensor * r_b.cross(impulse);
    }

    return;
    vec3 tangent = v_relative - collision.normal * v_relative.dot(collision.normal);
    if (tangent.length_squared() < EPSILON)
    {
        return;
    }

    tangent.normalize();
    const float u = (a.dynamic_friction + b.dynamic_friction) * 0.5f;
    const float j_T = clamp(-v_relative.dot(tangent) / denominator, -u * j, u * j);

    const vec3 friction_impulse = tangent * j_T;

    if (a.type == Physics_Body::Dynamic)
    {
        a.linear_velocity += friction_impulse * a.inverse_mass;
        a.angular_velocity += a.inverse_inertia_tensor * r_a.cross(friction_impulse);
    }
    if (b.type == Physics_Body::Dynamic)
    {
        b.linear_velocity -= friction_impulse * b.inverse_mass;
        b.angular_velocity -= b.inverse_inertia_tensor * r_b.cross(friction_impulse);
    }
}

void Physics_System::_position_correction(Physics_Body& a, Physics_Body& b, const Collision_Result& collision)
{
    const float correction_factor = 0.5f;
    const float slop = 0.01f; // Small allowed penetration to prevent jittering

    const float total_inverse_mass = a.inverse_mass + b.inverse_mass;
    const vec3 positional_correction = collision.normal * ((collision.penetration - slop) * correction_factor);

    if (a.type == Physics_Body::Dynamic)
    {
        a.transform.position -= positional_correction * (a.inverse_mass / total_inverse_mass);
    }
    if (b.type == Physics_Body::Dynamic)
    {
        b.transform.position += positional_correction * (b.inverse_mass / total_inverse_mass);
    }
}