// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/engine/physics/collision_function.hpp"
#include "cs/engine/physics/physics_system.hpp"

namespace Collision_Test_Function
{
    bool sphere_sphere(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, vec3& out_normal, float& out_penetration)
    {
        const vec3 direction = p_b - p_a;
        const float distance_squared = direction.length_squared();
        const float sum_r = a.shape.sphere.radius + b.shape.sphere.radius;
    
        if (distance_squared > (sum_r * sum_r)) 
        {
            return false;
        }

        const float dist = sqrt(distance_squared);
        if (dist > 0.0f)
        {
            out_normal = direction / dist;
        }
        else
        {
            out_normal = vec3::up_vector;
        }

        out_penetration = sum_r - dist;

        return true;
    }

    void closest_line_point_segment(const vec3& s_a, const vec3& e_a, const vec3& s_b, const vec3& e_b, vec3& out_closest_a, vec3& out_closest_b)
    {
        const vec3 d1 = e_a - s_a; // Direction of segment A
        const vec3 d2 = e_b - s_b; // Direction of segment B
        const vec3 r = s_a - s_b;
    
        const float a = d1.dot(d1);
        const float e = d2.dot(d2);
        const float f = d2.dot(r);
    
        float s, t;
        if (a <= 1e-6f && e <= 1e-6f) 
        {
            s = t = 0.0f; // Both segments are points
        }
        else if (a <= 1e-6f)
        {
            s = 0.0f;
            t = clamp(f / e, 0.0f, 1.0f);
        }
        else
        {
            float c = d1.dot(r);
            if (e <= 1e-6f)
            {
                t = 0.0f;
                s = clamp(-c / a, 0.0f, 1.0f);
            } 
            else
            {
                const float b = d1.dot(d2);
                const float denom = a * e - b * b;
                if (denom != 0.0f)
                {
                    s = clamp((b * f - c * e) / denom, 0.0f, 1.0f);
                } 
                else
                {
                    s = 0.0f;
                }
                t = (b * s + f) / e;
                t = clamp(t, 0.0f, 1.0f);
                s = (b * t - c) / a;
                s = clamp(s, 0.0f, 1.0f);
            }
        }

        out_closest_a = s_a + d1 * s;
        out_closest_b = s_b + d2 * t;
    }

    bool capsule_capsule(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, vec3& out_normal, float& out_penetration)
    {
        const vec3 segment_a(0.0f ,0.0f, a.shape.capsule.length);
        const vec3 start_a = p_a - segment_a;
        const vec3 end_a = p_a + segment_a;
        const float r_a = a.shape.capsule.radius;
        
        const vec3 segment_b(0.0f ,0.0f, b.shape.capsule.length);
        const vec3 start_b = p_b - segment_b;
        const vec3 end_b = p_b + segment_b;
        const float r_b = b.shape.capsule.radius;

        vec3 closest_a, closest_b;
        closest_line_point_segment(start_a, end_a, start_b, end_b, closest_a, closest_b);

        Collider sphere_a, sphere_b;
        sphere_a.shape.sphere.radius = a.shape.capsule.radius;
        sphere_b.shape.sphere.radius = b.shape.capsule.radius;
        return sphere_sphere(sphere_a, p_a, o_a, sphere_b, p_b, o_b, out_normal, out_penetration);
    }
}

namespace Collision_Resolve_Function
{
    void sphere_sphere(Physics_Body& a, Physics_Body& b, const Collision_Result& collision)
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
}