// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/engine/physics/collision_function.hpp"
#include "cs/engine/physics/physics_system.hpp"
#include <algorithm>

namespace Collision_Helpers
{
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

    void sat_test(const vec3& axis, const vec3 (&vertices)[CONVEX_HULL_MAX_VERTICES], int32 count, float& min, float& max)
    {
        assert(axis.length_squared() > 0);
        assert(count <= CONVEX_HULL_MAX_VERTICES);

        min = FLT_MAX;
        max = -FLT_MAX;

        for (int v = 0; v < count; ++v)
        {
            float dot = vertices[v].dot(axis);
            if (dot < min) min = dot;
            else if (dot > max) max = dot;
        }
    }

    vec3 sat_test(const vec3 (&vertices)[CONVEX_HULL_MAX_VERTICES], int32 count, const vec3& p, const vec3& direction)
    {
        PROFILE_FUNCTION()
        
        assert(count <= CONVEX_HULL_MAX_VERTICES);

        // Modified SAT test
        float max_dot = -FLT_MAX;
        vec3 best_vertex;
        for (int v = 0; v < count; ++v)
        {
            const vec3& transformed_vertex = vertices[v];// + p;
            const float dot = transformed_vertex.dot(direction);
            if (dot > max_dot)
            {
                max_dot = dot;
                best_vertex = transformed_vertex;
            }
        }

        return best_vertex;
    }

    vec3 minkowski_diff(const vec3 (&vertices_a)[CONVEX_HULL_MAX_VERTICES], int32 count_a, const vec3& p_a, 
        const vec3 (&vertices_b)[CONVEX_HULL_MAX_VERTICES], int32 count_b, const vec3& p_b, const vec3& direction)
    {
        PROFILE_FUNCTION()
        
        return sat_test(vertices_a, count_a, p_a, direction) - sat_test(vertices_b, count_b, p_b, -direction);
    }

    bool is_same_direction(const vec3& dir_a, const vec3& dir_b)
    {
        return dir_a.dot(dir_b) > 0;
    }

    bool line_simplex(vec3 (&simplex)[4], int32& count, vec3& direction)
    {
        assert(count == 2);

        vec3 a = simplex[2];
        vec3 b = simplex[3];

        vec3 ab = b - a;
        vec3 ao = -a;

        if (is_same_direction(ab, ao))
        {
            direction = ab.cross(ao).cross(ab);
        }
        else
        {
            simplex[3] = a;
            count = 1;
        }

        return false;
    }

    bool triangle_simplex(vec3 (&simplex)[4], int32& count, vec3& direction)
    {
        assert(count == 3);
      
        const vec3 a = simplex[1];
        const vec3 b = simplex[2];
        const vec3 c = simplex[3];

        const vec3 ab = b - a;
        const vec3 ac = c - a;
        const vec3 ao = -a;

        const vec3 abc = ab.cross(ac);

        if (is_same_direction(abc.cross(ac), ao))
        {
            if (is_same_direction(ac, ao))
            {
                simplex[2] = a;
                simplex[3] = c;
                count = 2;
                direction = ac.cross(ao).cross(ac);
            }
            else
            {
                simplex[2] = a;
                simplex[3] = b;
                count = 2;
                return line_simplex(simplex, count, direction);
            }
        }
        else
        {
            if (is_same_direction(ab.cross(abc), ao))
            {
                simplex[2] = a;
                simplex[3] = b;
                count = 2;
                return line_simplex(simplex, count, direction);
            }
            else
            {
                if (is_same_direction(abc, ao))
                {
                    direction = abc;
                }
                else
                {
                    simplex[1] = a;
                    simplex[2] = c;
                    simplex[3] = b;
                    direction = -abc;
                }
            }
        }

        return false;
    }

    bool tetrahedron_simplex(vec3 (&simplex)[4], int32& count, vec3& direction)
    {
        assert(count == 4);
        
        const vec3 a = simplex[0];
        const vec3 b = simplex[1];
        const vec3 c = simplex[2];
        const vec3 d = simplex[3];

        const vec3 ab = b - a;
        const vec3 ac = c - a;
        const vec3 ad = d - a;
        const vec3 ao = -a;

        const vec3 abc = ab.cross(ac);
        const vec3 acd = ac.cross(ad);
        const vec3 adb = ad.cross(ab);

        if (is_same_direction(abc, ao))
        {
            simplex[1] = a;
            simplex[2] = b;
            simplex[3] = c;
            count = 3;
            return triangle_simplex(simplex, count, direction);
        }
        
        if (is_same_direction(acd, ao))
        {
            simplex[1] = a;
            simplex[2] = c;
            simplex[3] = d;
            count = 3;
            return triangle_simplex(simplex, count, direction);
        }
        
        if (is_same_direction(adb, ao))
        {
            simplex[1] = a;
            simplex[2] = d;
            simplex[3] = b;
            count = 3;
            return triangle_simplex(simplex, count, direction);
        }

        return true;
    }

    bool next_simplex(vec3 (&simplex)[4], int32& count, vec3& direction)
    {
        PROFILE_FUNCTION()
        
        assert(count > 1 && count <= 4);
      
        switch(count)
        {
        case 2: return line_simplex(simplex, count, direction);
        case 3: return triangle_simplex(simplex, count, direction);
        case 4: return tetrahedron_simplex(simplex, count, direction);
        }

        // We should never have only 1 point here...
        return false;
    }

    bool gjk_test(const vec3 (&vertices_a)[CONVEX_HULL_MAX_VERTICES], int32 count_a, const vec3& p_a, 
        const vec3 (&vertices_b)[CONVEX_HULL_MAX_VERTICES], int32 count_b, const vec3& p_b, vec3 (&out_simplex)[4])
    {
        PROFILE_FUNCTION()
        
        vec3 direction(0.0f, 0.0f, 1.0f);
        vec3 support = Collision_Helpers::minkowski_diff(vertices_a, count_a, p_a, vertices_b, count_b, p_b, direction);
        
        out_simplex[3] = support;
        int32 simplex_count = 1;

        direction = -support;

        while (true)
        {
            support = Collision_Helpers::minkowski_diff(vertices_a, count_a, p_a, vertices_b, count_b, p_b, direction);

            if (support.dot(direction) <= 0)
            {
                return false;
            }

            simplex_count += 1;
            out_simplex[4 - simplex_count] = support;

            if (next_simplex(out_simplex, simplex_count, direction))
            {
                return true;
            }
        }
    }

    size_t calculate_face_normals(const std::vector<vec3>& vertices, const std::vector<size_t>& indices, std::vector<vec4>& out_normal_distances)
    {
        PROFILE_FUNCTION()
        
        assert(indices.size() % 3 == 0);
        size_t min_face = 0;
        float min_distance = FLT_MAX;

        for (size_t i = 0; i < indices.size(); i += 3)
        {
            const vec3& a = vertices[(int32)indices[i]];
            const vec3& b = vertices[(int32)indices[i + 1]];
            const vec3& c = vertices[(int32)indices[i + 2]];
            const vec3 ab = b - a;
            const vec3 ac = c - a;

            vec3 normal = ab.cross(ac).normalized();
            float distance = normal.dot(a);

            if (distance < 0)
            {
                normal *= -1;
                distance *= -1;
            }

            out_normal_distances.emplace_back(normal, distance);

            if (distance < min_distance)
            {
                min_face = i / 3;
                min_distance = distance;
            }
        }

        return min_face;
    }

    void epa_add_if_unique_edge(std::vector<std::pair<size_t, size_t>>& edges, const std::vector<size_t>& faces, size_t a, size_t b)
    {
        PROFILE_FUNCTION()
        
        auto reverse = std::find(                       //      0--<--3
            edges.begin(),                              //     / \ B /   A: 2-0
            edges.end(),                                //    / A \ /    B: 0-2
            std::make_pair(faces[b], faces[a])          //   1-->--2
        );
     
        if (reverse != edges.end())
        {
            edges.erase(reverse);
        }
        else 
        {
            edges.emplace_back(faces[a], faces[b]);
        }
    }

    // Expanding Polytope Algorithm - https://winter.dev/articles/epa-algorithm
    void epa(const vec3 (&vertices_a)[CONVEX_HULL_MAX_VERTICES], int32 count_a, const vec3& p_a, 
    const vec3 (&vertices_b)[CONVEX_HULL_MAX_VERTICES], int32 count_b, const vec3& p_b,
    const vec3 (&gjk_simplex)[4], vec3& out_normal, float& out_penetration)
    {
        PROFILE_FUNCTION()
        
        std::vector<vec3> polytope({gjk_simplex[0], gjk_simplex[1], gjk_simplex[2], gjk_simplex[3]});
        std::vector<size_t> faces = {
            0, 1, 2,
            0, 3, 1,
            0, 2, 3,
            1, 3, 2
        };

        std::vector<vec4> normal_distances;
        size_t min_face = Collision_Helpers::calculate_face_normals(polytope, faces, normal_distances);

        vec3 min_normal;
        float min_distance = FLT_MAX;
        while (min_distance == FLT_MAX)
        {
            min_normal = normal_distances[(int32)min_face].xyz;
            min_distance = normal_distances[(int32)min_face].w;

            vec3 support = Collision_Helpers::minkowski_diff(vertices_a, count_a, p_a, vertices_b, count_b, p_b, min_normal);
            float distance = min_normal.dot(support);

            float diff = distance - min_distance;
            if (fabs(diff) > 0.001f)
            {
                min_distance = FLT_MAX;

                std::vector<std::pair<size_t, size_t>> unique_edges;

                for (size_t i = 0; i < normal_distances.size(); ++i)
                {
                    if (Collision_Helpers::is_same_direction(normal_distances[i].xyz, support))
                    {
                        size_t f = i * 3;

                        Collision_Helpers::epa_add_if_unique_edge(unique_edges, faces, f + 0, f + 1);
                        Collision_Helpers::epa_add_if_unique_edge(unique_edges, faces, f + 1, f + 2);
                        Collision_Helpers::epa_add_if_unique_edge(unique_edges, faces, f + 2, f + 0);

                        faces[f + 2] = faces.back(); faces.pop_back();
                        faces[f + 1] = faces.back(); faces.pop_back();
                        faces[f    ] = faces.back(); faces.pop_back();
    
                        normal_distances[i] = normal_distances.back(); // pop-erase
                        normal_distances.pop_back();

                        i--;
                    }
                }

                std::vector<size_t> new_faces;
                for (auto [e_i1, e_i2] : unique_edges) 
                {
                    new_faces.push_back(e_i1);
                    new_faces.push_back(e_i2);
                    new_faces.push_back(polytope.size());
                }
                 
                polytope.push_back(support);

                std::vector<vec4> new_normal_distances;
                size_t new_min_face = Collision_Helpers::calculate_face_normals(polytope, new_faces, new_normal_distances);

                float old_min_distance = FLT_MAX;
                for (size_t i = 0; i < normal_distances.size(); i++) 
                {
                    if (normal_distances[i].w < old_min_distance) 
                    {
                        old_min_distance = normal_distances[i].w;
                        min_face = i;
                    }
                }
    
                if (new_normal_distances[new_min_face].w < old_min_distance) 
                {
                    min_face = new_min_face + normal_distances.size();
                }
    
                faces.insert(faces.end(), new_faces.begin(), new_faces.end());
                normal_distances.insert(normal_distances.end(), new_normal_distances.begin(), new_normal_distances.end());
            }
        }
    
        // Total overlap of meshes
        out_normal = min_normal;
        out_penetration = min_distance + NEARLY_ZERO;
    }
};

namespace Collision_Test_Function
{
    bool sphere_sphere(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, vec3& out_normal, float& out_penetration)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Sphere);
        assert(b.type == Collider::Sphere);

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

    bool sphere_capsule(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, vec3& out_normal, float& out_penetration)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Sphere);
        assert(b.type == Collider::Capsule);

        const vec3 segment_b(0.0f ,0.0f, b.shape.capsule.length);
        const vec3 start_b = p_b - segment_b * 0.5f;
        const vec3 end_b = p_b + segment_b * 0.5f;
        const float r_b = b.shape.capsule.radius;

        vec3 closest_a, closest_b;
        Collision_Helpers::closest_line_point_segment(p_a, p_a, start_b, end_b, closest_a, closest_b);
        
        Collider sphere_b;
        sphere_b.type = Collider::Sphere;
        sphere_b.shape.sphere.radius = b.shape.capsule.radius;
        return sphere_sphere(a, p_a, o_a, sphere_b, closest_b, o_b, out_normal, out_penetration);
    }

    bool capsule_sphere(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, vec3& out_normal, float& out_penetration)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Capsule);
        assert(b.type == Collider::Sphere);

        return sphere_capsule(b, p_b, o_b, a, p_a, o_a, out_normal, out_penetration);
    }

    bool capsule_capsule(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, vec3& out_normal, float& out_penetration)
    {
        PROFILE_FUNCTION()

        assert(a.type == Collider::Capsule);
        assert(b.type == Collider::Capsule);

        const vec3 segment_a(0.0f ,0.0f, a.shape.capsule.length);
        const vec3 start_a = p_a - segment_a * 0.5f;
        const vec3 end_a = p_a + segment_a * 0.5f;
        const float r_a = a.shape.capsule.radius;
        
        const vec3 segment_b(0.0f ,0.0f, b.shape.capsule.length);
        const vec3 start_b = p_b - segment_b * 0.5f;
        const vec3 end_b = p_b + segment_b * 0.5f;
        const float r_b = b.shape.capsule.radius;

        vec3 closest_a, closest_b;
        Collision_Helpers::closest_line_point_segment(start_a, end_a, start_b, end_b, closest_a, closest_b);

        Collider sphere_a, sphere_b;
        sphere_a.type = Collider::Sphere;
        sphere_a.shape.sphere.radius = a.shape.capsule.radius;
        sphere_b.type = Collider::Sphere;
        sphere_b.shape.sphere.radius = b.shape.capsule.radius;
        return sphere_sphere(sphere_a, closest_a, o_a, sphere_b, closest_b, o_b, out_normal, out_penetration);
    }

    bool convex_convex(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, vec3& out_normal, float& out_penetration)
    {
        PROFILE_FUNCTION()

        assert(a.type == Collider::Convex_Hull);
        assert(b.type == Collider::Convex_Hull);

        int32 count_a = a.shape.convex_hull.count;
        vec3 vertices_a[CONVEX_HULL_MAX_VERTICES];
        for (int i = 0; i < count_a; ++i)
        {
            vertices_a[i] = a.shape.convex_hull.vertices[i] + p_a;
        }

        int32 count_b = b.shape.convex_hull.count;
        vec3 vertices_b[CONVEX_HULL_MAX_VERTICES];
        for (int i = 0; i < count_b; ++i)
        {
            vertices_b[i] = b.shape.convex_hull.vertices[i] + p_b;
        }
        //vec3 (&vertices_a)[CONVEX_HULL_MAX_VERTICES] = a.shape.convex_hull.vertices;
        //vec3 (&vertices_b)[CONVEX_HULL_MAX_VERTICES] = b.shape.convex_hull.vertices;

        vec3 gjk_simplex[4];
        if (!Collision_Helpers::gjk_test(vertices_a, count_a, p_a, vertices_b, count_b, p_b, gjk_simplex))
        {
            return false;
        }

        Collision_Helpers::epa(vertices_a, count_a, p_a, vertices_b, count_b, p_b, gjk_simplex, out_normal, out_penetration);

        return true;
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