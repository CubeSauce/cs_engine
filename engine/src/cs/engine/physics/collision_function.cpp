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
        if (a <= EPSILON && e <= EPSILON) 
        {
            s = t = 0.0f; // Both segments are points
        }
        else if (a <= EPSILON)
        {
            s = 0.0f;
            t = clamp(f / e, 0.0f, 1.0f);
        }
        else
        {
            float c = d1.dot(r);
            if (e <= EPSILON)
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

    // Finding the point furthest from the origin in a given direction using the Separation Axis theorem
    vec3 sat_max_distance(const vec3 (&vertices)[CONVEX_HULL_MAX_NUM_VERTICES], int32 count, const vec3& p, const vec3& direction)
    {
        vec3 furthest_vertex;
        float max_distance = -FLT_MAX;

        //TODO: Add rotations
        vec3 transformed_direction = direction;

        for (int v = 0; v < count; ++v)
        {
            const float distance = vertices[v].dot(transformed_direction);
            if (distance > max_distance)
            {
                max_distance = distance;
                furthest_vertex = vertices[v];
            }
        }

        return furthest_vertex + p;
    }

    // Used for getting the "mesh" differences, optimizing by getting the difference between furthest point for a given direction
    vec3 minkowski_difference(const vec3 (&vertices_a)[CONVEX_HULL_MAX_NUM_VERTICES], int32 count_a, const vec3& p_a,
        const vec3 (&vertices_b)[CONVEX_HULL_MAX_NUM_VERTICES], int32 count_b, const vec3& p_b, const vec3& direction)
    {
        return sat_max_distance(vertices_a, count_a, p_a, direction) - sat_max_distance(vertices_b, count_b, p_b, -direction);
    }
    
    // Just a more common name for minkowski_difference
    vec3 support(const vec3 (&vertices_a)[CONVEX_HULL_MAX_NUM_VERTICES], int32 count_a, const vec3& p_a,
        const vec3 (&vertices_b)[CONVEX_HULL_MAX_NUM_VERTICES], int32 count_b, const vec3& p_b, const vec3& direction)
    {
        return minkowski_difference(vertices_a, count_a, p_a, vertices_b, count_b, p_b, direction);
    }

    void add_to_simplex(vec3 (&simplex)[4], int32& count, const vec3& support)
    {
        assert(count > 0 && count < 4);

        for (int i = count; i > 0; --i)
        {
            simplex[i] = simplex[i - 1];
        }

        simplex[0] = support;
        count++;
    }

    bool same_direction(const vec3& direction_a, const vec3& direction_b)
    {
        return direction_a.dot(direction_b) > 0;
    }

    bool line_simplex(vec3 (&simplex)[4], int32& count, vec3& direction)
    {
        assert(count == 2);
        const vec3 a = simplex[0];
        const vec3 b = simplex[1];

        const vec3 ab = b - a;
        const vec3 ao = -a;

        if (same_direction(ab, ao))
        {
            vec3 normal = ab.cross(ao);
            if (normal.length_squared() < 1e-6)
            {
                direction = direction.perpendicular();
            }
            else
            {
                direction = normal.cross(ab);
            }
        }
        else
        {
            // Otherwise find another minkowski diff in the oposite direction in next cycle
            // same point
            count = 1;
            direction = ao;
        }

        return false;
    }

    bool triangle_simplex(vec3 (&simplex)[4], int32& count, vec3& direction)
    {
        assert(count == 3);
        const vec3 a = simplex[0];
        const vec3 b = simplex[1];
        const vec3 c = simplex[2];

        const vec3 ab = b - a;
        const vec3 ac = c - a;
        const vec3 ao = -a;

        vec3 abc = ab.cross(ac);

        if (same_direction(abc.cross(ac), ao))
        {
            // Do we already have something in this dir
            if (same_direction(ac, ao))
            {
                simplex[1] = c;
                count = 2;

                // Look perpendicular to it
                direction = ac.cross(ao).cross(ac);
            }
            else
            {
                // same points
                count = 2;
                return line_simplex(simplex, count, direction);
            }
        }
        else
        {
            if (same_direction(ab.cross(abc), ao))
            {
                count = 2;
                return line_simplex(simplex, count, direction);
            }
            else
            {
                if (same_direction(abc, ao))
                {
                    direction = abc;
                }
                else
                {
                    // swap simplex points
                    simplex[1] = c;
                    simplex[2] = b;
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
        const vec3 abd = ad.cross(ab);

        if (same_direction(abc, ao))
        {
            count = 3;
            return triangle_simplex(simplex, count, direction);
        }
        
        if (same_direction(acd, ao))
        {
            count = 3;
            simplex[2] = d;
            return triangle_simplex(simplex, count, direction);
        }
        
        if (same_direction(abd, ao))
        {
            count = 3;
            simplex[1] = d;
            return triangle_simplex(simplex, count, direction);
        }

        return true;
    }

    bool next_simplex(vec3 (&simplex)[4], int32& count, vec3& direction)
    {
        assert(count >= 2 && count <= 4);

        switch(count)
        {
        case 2: return line_simplex(simplex, count, direction);
        case 3: return triangle_simplex(simplex, count, direction);
        case 4: return tetrahedron_simplex(simplex, count, direction);
        }

        // Something went very wrong
        return false;
    }

    // Gilbert-Johnson-Keerthi [GJK] algorithm - https://winter.dev/articles/gjk-algorithm
    // We try to construct a simplex (4 - point mesh), that contains the distance 
    // between furthest points, since convex hulls are in question, if the 
    // simplex contains the origin inside it, there must be a collision between the two meshes.
    bool gjk(const vec3 (&vertices_a)[CONVEX_HULL_MAX_NUM_VERTICES], int32 count_a, const vec3& p_a,
    const vec3 (&vertices_b)[CONVEX_HULL_MAX_NUM_VERTICES], int32 count_b, const vec3& p_b, const vec3& initial_direction, vec3 (&simplex)[4])
    {
        // choose an initial direction, for the first support
        vec3 support = Collision_Helpers::support(vertices_a, count_a, p_a, vertices_b, count_b, p_b, initial_direction);

        // First point of simplex is the initial furthest distance
        int32 count_s = 1;
        simplex[0] = support;
        //printf("adding to simplex support: %f %f %f\n", support.x, support.y, support.z);

        // As the support function shows us a result away from the origin, we flip it to be towards the origin, so the next point can 
        // point away from the first point.
        vec3 direction = -support;

        while(true)
        {
            //printf("direction: %f %f %f\n", direction.x, direction.y, direction.z);
            // We get the next distance
            support = Collision_Helpers::support(vertices_a, count_a, p_a, vertices_b, count_b, p_b, direction);

            // If a found support (diff between two vertices) is behind the origin,
            // then the origin can't be inside the simplex - no collision
            if (support.dot(direction) <= 0)
            {
                return false;
            }

            Collision_Helpers::add_to_simplex(simplex, count_s, support);

            printf("adding to simplex support: %f %f %f\n", support.x, support.y, support.z);
            if (Collision_Helpers::next_simplex(simplex, count_s, direction))
            {
                return true;
            }
        }
    }

    void get_face_normals(const std::vector<vec3>& polytope, const std::vector<size_t>& faces, size_t& min_face, std::vector<vec4>& normals)
    {
        float  minDistance = FLT_MAX;
    
        for (size_t i = 0; i < faces.size(); i += 3) 
        {
            vec3 a = polytope[faces[i    ]];
            vec3 b = polytope[faces[i + 1]];
            vec3 c = polytope[faces[i + 2]];
    
            vec3 normal = (b-a).cross(c-a).normalized();
            float distance = normal.dot(a);
    
            if (distance < 0) 
            {
                normal   *= -1;
                distance *= -1;
            }
    
            normals.emplace_back(normal, distance);
    
            if (distance < minDistance) 
            {
                min_face = i / 3;
                minDistance = distance;
            }
        }
    }

    void add_edge_if_unique(std::vector<std::pair<size_t, size_t>>& edges, const std::vector<size_t>& faces, size_t a, size_t b)
    {
        auto reverse = std::find(                       //      0--<--3
            edges.begin(),                              //     / \ B /   A: 2-0
            edges.end(),                                //    / A \ /    B: 0-2
            std::make_pair(faces[b], faces[a]) //   1-->--2
        );
    
        if (reverse != edges.end()) {
            edges.erase(reverse);
        }
    
        else {
            edges.emplace_back(faces[a], faces[b]);
        }
    }

    // Expanding polytope algorithm - https://winter.dev/articles/epa-algorithm
    bool epa(const vec3 (&vertices_a)[CONVEX_HULL_MAX_NUM_VERTICES], int32 count_a, const vec3& p_a,
        const vec3 (&vertices_b)[CONVEX_HULL_MAX_NUM_VERTICES], int32 count_b, const vec3& p_b, 
        const vec3 (&simplex)[4], vec3& out_normal, float& out_penetration)
    {
        std::vector<vec3> polytope = { simplex[0], simplex[1], simplex[2], simplex[3] };
        std::vector<size_t> faces = {
            0, 1, 2,
            0, 3, 1,
            0, 2, 3,
            1, 3, 2
        };

        std::vector<vec4> normals;
        size_t min_face;
        get_face_normals(polytope, faces, min_face, normals);

        vec3 min_normal;
        float min_distance = FLT_MAX;

        while (min_distance == FLT_MAX)
        {
            min_normal = normals[min_face].xyz;
            min_distance = normals[min_face].w;
            
            vec3 support = Collision_Helpers::support(vertices_a, count_a, p_a, vertices_b, count_b, p_b, min_normal);
            float support_distance = min_normal.dot(support);

            if (abs(support_distance - min_distance) > 0.001f)
            {
                min_distance = FLT_MAX;

                std::vector<std::pair<size_t, size_t>> uniqueEdges;
                for (size_t i = 0; i < normals.size(); i++)
                {
                    if (Collision_Helpers::same_direction(normals[i].xyz, support))
                    {
                        size_t f = i * 3;

                        Collision_Helpers::add_edge_if_unique(uniqueEdges, faces, f,     f + 1);
                        Collision_Helpers::add_edge_if_unique(uniqueEdges, faces, f + 1, f + 2);
                        Collision_Helpers::add_edge_if_unique(uniqueEdges, faces, f + 2, f    );

                        faces[f + 2] = faces.back(); faces.pop_back();
                        faces[f + 1] = faces.back(); faces.pop_back();
                        faces[f    ] = faces.back(); faces.pop_back();

                        normals[i] = normals.back(); // pop-erase
                        normals.pop_back();

                        i--;
                    }
                }
            
                std::vector<size_t> newFaces;
                for (auto [edgeIndex1, edgeIndex2] : uniqueEdges) 
                {
                    newFaces.push_back(edgeIndex1);
                    newFaces.push_back(edgeIndex2);
                    newFaces.push_back(polytope.size());
                }
                 
                // if (std::find_if(polytope.begin(), polytope.end(), [support](const vec3& val){
                //     return val.nearly_equal(support);
                // }) != polytope.end())
                // {
                //     out_normal = min_normal;
                //     out_penetration = min_distance;
                //     return true;
                // }

                polytope.push_back(support);

                std::vector<vec4> newNormals;
                size_t newMinFace;
                get_face_normals(polytope, newFaces, newMinFace, newNormals);

                float best_distance = FLT_MAX;
                for (size_t i = 0; i < normals.size(); i++) 
                {
                    if (normals[i].w < best_distance) 
                    {
                        best_distance = normals[i].w;
                        min_face = i;
                    }
                }
                
                for (size_t i = 0; i < newNormals.size(); i++) 
                {
                    if (newNormals[i].w < best_distance) 
                    {
                        best_distance = newNormals[i].w;
                        min_face = i + normals.size();
                    }
                }
     
                faces  .insert(faces  .end(), newFaces  .begin(), newFaces  .end());
                normals.insert(normals.end(), newNormals.begin(), newNormals.end());          
            }
        }

        out_normal = min_normal;
        out_penetration = min_distance;

        return true;
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

        const vec3 segment_a(0.0f, 0.0f, a.shape.capsule.length);
        const vec3 start_a = p_a - segment_a * 0.5f;
        const vec3 end_a = p_a + segment_a * 0.5f;
        const float r_a = a.shape.capsule.radius;
        
        const vec3 segment_b(0.0f, 0.0f, b.shape.capsule.length);
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

    // Massive thanks to https://winter.dev/ for extremely intuitive explanation and an implementation example
    bool convex_convex(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, vec3& out_normal, float& out_penetration)
    {
        PROFILE_FUNCTION()

        assert(a.type == Collider::Convex_Hull);
        assert(b.type == Collider::Convex_Hull);

        const int32 count_a = a.shape.convex_hull.count;
        const vec3 (&vertices_a)[CONVEX_HULL_MAX_NUM_VERTICES] = a.shape.convex_hull.vertices;
        
        const int32 count_b = b.shape.convex_hull.count;
        const vec3 (&vertices_b)[CONVEX_HULL_MAX_NUM_VERTICES] = b.shape.convex_hull.vertices;

        vec3 simplex[4];

        Box combined_bounds = a.bounds;
        combined_bounds.expand(b.bounds);

        if (!Collision_Helpers::gjk(vertices_a, count_a, p_a, vertices_b, count_b, p_b, combined_bounds.max, simplex))
        {
            return false;
        }

        return Collision_Helpers::epa(vertices_a, count_a, p_a, vertices_b, count_b, p_b, simplex, out_normal, out_penetration);
    }
}
