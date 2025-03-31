// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/engine/physics/collision_function.hpp"
#include "cs/engine/physics/physics_system.hpp"
#include <algorithm>

namespace Collision_Helpers
{
    mat4 _inertia_tensor_sphere(const Collider& collider, const float mass)
    {
        const float radius = collider.shape.sphere.radius;
        const float inertia = (2.0f/5.0f) * radius * radius * mass;
        return mat4(inertia);
    }

    mat4 _inertia_tensor_capsule(const Collider& collider, const float mass)
    {
        const float radius = collider.shape.capsule.radius;
        const float height = collider.shape.capsule.length;
        
        const float cylinderMass = mass * (height / (height + 2 * radius));
        const float sphereMass = (mass - cylinderMass) / 2.0f;
        const float Ixx_cyl = (1.0f / 12.0f) * cylinderMass * (3 * radius * radius + height * height);
        const float Izz_cyl = (1.0f / 2.0f) * cylinderMass * radius * radius;
        const float Ixx_sphere = (2.0f / 5.0f) * sphereMass * radius * radius * 0.5f;
        const float Izz_sphere = Ixx_sphere;
        const float Ixx = Ixx_cyl + 2.0f * Ixx_sphere;
        const float Iyy = Ixx;
        const float Izz = Izz_cyl + 2.0f * Izz_sphere;
    
        return mat4(
            vec4( Ixx, 0.0f, 0.0f, 0.0f),
            vec4(0.0f,  Iyy, 0.0f, 0.0f),
            vec4(0.0f, 0.0f,  Izz, 0.0f),
            vec4(0.0f, 0.0f, 0.0f, 1.0f)
        );
    }

    mat4 _inertia_tensor_cylinder(const Collider& collider, const float mass)
    {
        const float height = collider.shape.cylinder.height;
        const float radius = collider.shape.cylinder.radius;
        float Ixx = (1.0f / 12.0f) * (3 * radius * radius + height * height) * mass;
        float Iyy = Ixx;
        float Izz = (1.0f / 2.0f) * radius * radius * mass;
        
        return mat4(
            vec4( Ixx, 0.0f, 0.0f, 0.0f),
            vec4(0.0f,  Iyy, 0.0f, 0.0f),
            vec4(0.0f, 0.0f,  Izz, 0.0f),
            vec4(0.0f, 0.0f, 0.0f, 1.0f)
        );
    }

    mat4 _inertia_tensor_box(const Collider& collider, const float mass)
    {
        //TODO: add box collider
        const vec3 box_extents = collider.shape.bounding_box.get_extents();        
        float Ixx = (1.0f / 12.0f) * (box_extents.z * box_extents.z + box_extents.y * box_extents.y) * mass;
        float Iyy = (1.0f / 12.0f) * (box_extents.x * box_extents.x + box_extents.y * box_extents.y) * mass;
        float Izz = (1.0f / 12.0f) * (box_extents.x * box_extents.x + box_extents.z * box_extents.z) * mass;
        
        return mat4(
            vec4( Ixx, 0.0f, 0.0f, 0.0f),
            vec4(0.0f,  Iyy, 0.0f, 0.0f),
            vec4(0.0f, 0.0f,  Izz, 0.0f),
            vec4(0.0f, 0.0f, 0.0f, 1.0f)
        );
    }

    mat4 _inertia_tensor_convex(const Collider& collider, const float mass)
    {
        //TODO: tetrahedral decomposition
        return mat4(1.0f);
    }

    mat4 inertia_tensor(const Collider& collider, const float mass)
    {
        switch(collider.type)
        {
        case Collider::Sphere: return _inertia_tensor_sphere(collider, mass);
        case Collider::Capsule: return _inertia_tensor_capsule(collider, mass);
        case Collider::Cylinder: return _inertia_tensor_cylinder(collider, mass);
        case Collider::Box: return _inertia_tensor_box(collider, mass);
        case Collider::Convex_Hull: return _inertia_tensor_convex(collider, mass);
        default: assert(false);
        }

        return mat4(1.0f);
    }

    void closest_point_on_two_segments(const vec3& s_a, const vec3& e_a, const vec3& s_b, const vec3& e_b, vec3& out_closest_a, vec3& out_closest_b)
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

    vec3 closest_point_on_line(const vec3& a, const vec3& b, const vec3& p)
    {
        vec3 ab = b - a;
        //TODO: check ab
        float t = clamp((p-a).dot(ab) / ab.dot(ab), 0.0f, 1.0f);
        return a + ab * t;
    }

    // Finding the point furthest from the origin in a given direction using the Separation Axis theorem
    vec3 sat_max_distance(const vec3 (&vertices)[CONVEX_HULL_MAX_NUM_VERTICES], int32 count, const vec3& p, const quat& o, const vec3& direction)
    {
        vec3 furthest_vertex;
        float max_distance = -FLT_MAX;

        // Orientate direction in mesh space
        vec3 transformed_direction = o.mul(direction);

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
    vec3 minkowski_difference(const vec3 (&vertices_a)[CONVEX_HULL_MAX_NUM_VERTICES], int32 count_a, const vec3& p_a, const quat& o_a,
        const vec3 (&vertices_b)[CONVEX_HULL_MAX_NUM_VERTICES], int32 count_b, const vec3& p_b, const quat& o_b, const vec3& direction)
    {
        return sat_max_distance(vertices_a, count_a, p_a, o_a, direction) - sat_max_distance(vertices_b, count_b, p_b, o_b, -direction);
    }
    
    // Just a more common name for minkowski_difference
    vec3 support(const vec3 (&vertices_a)[CONVEX_HULL_MAX_NUM_VERTICES], int32 count_a, const vec3& p_a, const quat& o_a,
        const vec3 (&vertices_b)[CONVEX_HULL_MAX_NUM_VERTICES], int32 count_b, const vec3& p_b, const quat& o_b, const vec3& direction)
    {
        return minkowski_difference(vertices_a, count_a, p_a, o_a, vertices_b, count_b, p_b, o_b, direction);
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
    bool gjk(const vec3 (&vertices_a)[CONVEX_HULL_MAX_NUM_VERTICES], int32 count_a, const vec3& p_a, const quat& o_a,
    const vec3 (&vertices_b)[CONVEX_HULL_MAX_NUM_VERTICES], int32 count_b, const vec3& p_b, const quat& o_b, const vec3& initial_direction, vec3 (&simplex)[4])
    {
        // choose an initial direction, for the first support
        vec3 support = Collision_Helpers::support(vertices_a, count_a, p_a, o_a, vertices_b, count_b, p_b, o_b, initial_direction);

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
            support = Collision_Helpers::support(vertices_a, count_a, p_a, o_a, vertices_b, count_b, p_b, o_b, direction);

            // If a found support (diff between two vertices) is behind the origin,
            // then the origin can't be inside the simplex - no collision
            if (support.dot(direction) <= 0)
            {
                return false;
            }

            Collision_Helpers::add_to_simplex(simplex, count_s, support);

            //printf("adding to simplex support: %f %f %f\n", support.x, support.y, support.z);
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
    bool epa(const vec3 (&vertices_a)[CONVEX_HULL_MAX_NUM_VERTICES], int32 count_a, const vec3& p_a, const quat& o_a,
        const vec3 (&vertices_b)[CONVEX_HULL_MAX_NUM_VERTICES], int32 count_b, const vec3& p_b, const quat& o_b, 
        const vec3 (&simplex)[4], Collision_Result& result)
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
        vec3 support;

        while (min_distance == FLT_MAX)
        {
            min_normal = normals[min_face].xyz;
            min_distance = normals[min_face].w;
            
            vec3 support = Collision_Helpers::support(vertices_a, count_a, p_a, o_a, vertices_b, count_b, p_b, o_b, min_normal);
            float support_distance = min_normal.dot(support);

            if (abs(support_distance - min_distance) > 0.001f)
            {
                min_distance = FLT_MAX;

                std::vector<std::pair<size_t, size_t>> unique_edges;
                for (size_t i = 0; i < normals.size(); i++)
                {
                    if (Collision_Helpers::same_direction(normals[i].xyz, support))
                    {
                        size_t f = i * 3;

                        Collision_Helpers::add_edge_if_unique(unique_edges, faces, f,     f + 1);
                        Collision_Helpers::add_edge_if_unique(unique_edges, faces, f + 1, f + 2);
                        Collision_Helpers::add_edge_if_unique(unique_edges, faces, f + 2, f    );

                        faces[f + 2] = faces.back(); faces.pop_back();
                        faces[f + 1] = faces.back(); faces.pop_back();
                        faces[f    ] = faces.back(); faces.pop_back();

                        normals[i] = normals.back(); // pop-erase
                        normals.pop_back();

                        i--;
                    }
                }
            
                std::vector<size_t> new_faces;
                for (auto [edge_1, edge_2] : unique_edges) 
                {
                    new_faces.push_back(edge_1);
                    new_faces.push_back(edge_2);
                    new_faces.push_back(polytope.size());
                }

                polytope.push_back(support);

                std::vector<vec4> new_normals;
                size_t new_min_face;
                get_face_normals(polytope, new_faces, new_min_face, new_normals);

                float best_distance = FLT_MAX;
                for (size_t i = 0; i < normals.size(); i++) 
                {
                    if (normals[i].w < best_distance) 
                    {
                        best_distance = normals[i].w;
                        min_face = i;
                    }
                }
                
                for (size_t i = 0; i < new_normals.size(); i++) 
                {
                    if (new_normals[i].w < best_distance) 
                    {
                        best_distance = new_normals[i].w;
                        min_face = i + normals.size();
                    }
                }
     
                faces  .insert(faces  .end(), new_faces  .begin(), new_faces  .end());
                normals.insert(normals.end(), new_normals.begin(), new_normals.end());          
            }
        }

        result.normal = min_normal;
        result.penetration = min_distance;
        result.contact_point = support - min_normal * min_distance * 0.5f;

        return true;
    }
};

namespace Collision_Test_Function
{
    bool sphere_sphere(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Sphere);
        assert(b.type == Collider::Sphere);

        const vec3 delta = p_b - p_a;
        const float distance_sq = delta.length_squared();
        const float sum_r = a.shape.sphere.radius + b.shape.sphere.radius;
    
        if (distance_sq > (sum_r * sum_r)) 
        {
            return false;
        }

        result.normal = delta.normalized();
        result.penetration = sum_r - sqrt(distance_sq);
        result.contact_point = p_a  + result.normal * a.shape.sphere.radius;
        
        return true;
    }

    bool sphere_capsule(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Sphere);
        assert(b.type == Collider::Capsule);

        //TODO: axis
        const vec3 segment_b(0.0f ,0.0f, b.shape.capsule.length);
        const vec3 start_b = p_b - segment_b * 0.5f;
        const vec3 end_b = p_b + segment_b * 0.5f;

        const vec3 closest_point_capsule = Collision_Helpers::closest_point_on_line(start_b, end_b, p_a);
        const float sum_r = a.shape.sphere.radius + b.shape.capsule.radius;

        const vec3 delta = closest_point_capsule - p_a;
        const float distance_sq = delta.length_squared();
        if (distance_sq > (sum_r * sum_r))
        {
            return false;
        }
        
        result.normal = delta.normalized();
        result.penetration = sum_r - sqrt(distance_sq);
        result.contact_point = p_a - result.normal * a.shape.sphere.radius;

        return true;
    }

    bool sphere_cylinder(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Sphere);
        assert(b.type == Collider::Cylinder);

        const vec3 cylinder_axis = o_b.mul(vec3::up_vector);

        const vec3 cylinder_base = p_b - cylinder_axis * (b.shape.cylinder.height * 0.5f);
        const vec3 to_sphere = p_a - cylinder_base;
        const float t = clamp(to_sphere.dot(cylinder_axis), 0.0f, 1.0f);
        
        const vec3 closest = cylinder_base + cylinder_axis * t;
        const float sum_radii = a.shape.sphere.radius + b.shape.cylinder.radius;
        
        vec3 delta = closest - p_a;
        float dist2 = delta.length_squared();
        if (dist2 > sum_radii * sum_radii)
        {
            return false;
        }

        result.normal = delta.normalized();
        result.penetration = sum_radii - sqrtf(dist2);
        result.contact_point = closest + result.normal * b.shape.cylinder.radius;

        return true;
    }

    bool sphere_box(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Sphere);
        assert(b.type == Collider::Box);

        // Step 1: Transform sphere center to OBB local space
       
        const mat4 rot_b = o_b.to_mat4();
        const vec3 box_axes[3] = {
            rot_b[0].xyz.normalized(),
            rot_b[1].xyz.normalized(),
            rot_b[2].xyz.normalized(),
        };

        vec3 box_half_extents = b.shape.bounding_box.get_half_extents();

        vec3 local_center = p_a - p_b;
        vec3 closest_point = p_b; 
        for (int i = 0; i < 3; ++i)
        {
            // Project onto OBB axis
            const float projection = local_center.dot(box_axes[i]);
            const float clamped_projection = std::clamp(projection, -box_half_extents[i], box_half_extents[i]);
            closest_point += box_axes[i] * clamped_projection;
        }

        const vec3 delta = closest_point - p_a;
        const float dist_sq = delta.length_squared();

        if (dist_sq > a.shape.sphere.radius * a.shape.sphere.radius)
        {
            return false;
        }

        // From other obj to sphere
        result.normal = delta.normalized();
        result.penetration = a.shape.sphere.radius - sqrtf(dist_sq);
        result.contact_point = closest_point;
        
        return true;
    }
    
    bool sphere_convex(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Sphere);
        assert(b.type == Collider::Convex_Hull);

        assert(false);
        return false;
    }

    bool capsule_sphere(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Capsule);
        assert(b.type == Collider::Sphere);

        if (sphere_capsule(b, p_b, o_b, a, p_a, o_a, result))
        {
            result.normal = -result.normal;
            return true;
        }

        return false;
    }

    bool capsule_capsule(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
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
        Collision_Helpers::closest_point_on_two_segments(start_a, end_a, start_b, end_b, closest_a, closest_b);

        Collider sphere_a, sphere_b;
        sphere_a.type = Collider::Sphere;
        sphere_a.shape.sphere.radius = a.shape.capsule.radius;
        sphere_b.type = Collider::Sphere;
        sphere_b.shape.sphere.radius = b.shape.capsule.radius;
        return sphere_sphere(sphere_a, closest_a, o_a, sphere_b, closest_b, o_b, result);
    }

    bool capsule_cylinder(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Capsule);
        assert(b.type == Collider::Cylinder);

        //TODO check if this method is faster then finding closest segment and then check sphere collision

        const vec3 capsule_axis = o_a.mul(vec3::up_vector);
        const vec3 capsule_bot = p_a - a.shape.capsule.length * 0.5f;

        const vec3 cylinder_axis = o_b.mul(vec3::up_vector);

        const vec3 cylinder_base = p_b - cylinder_axis * (b.shape.cylinder.height * 0.5f);

        const float t_capsule = clamp((cylinder_base - capsule_bot).dot(capsule_axis) / (a.shape.capsule.length * a.shape.capsule.length), 0.0f, 1.0f);
        const float t_cylinder = clamp((capsule_bot - cylinder_base).dot(cylinder_axis) / (b.shape.cylinder.height * b.shape.cylinder.height), 0.0f, 1.0f);

        const vec3 closest_cylinder = cylinder_base + cylinder_axis * t_cylinder;
        const vec3 closest_capsule = capsule_bot + capsule_axis * t_capsule;
        const float sum_radii = a.shape.capsule.radius + b.shape.cylinder.radius;

        vec3 delta = closest_capsule - closest_cylinder;
        float dist2 = delta.length_squared();
        if (dist2 > sum_radii * sum_radii)
        {
            return false;
        }

        result.normal = delta.normalized();
        result.penetration = sum_radii - sqrtf(dist2);
        //TODO: CHECK that it's the actual contact point
        result.contact_point = closest_cylinder + result.normal * b.shape.cylinder.radius;
        
        return true;
    }

    bool capsule_box(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Capsule);
        assert(b.type == Collider::Box);
        
        const mat4 rot_a = o_a.to_mat4();
        const vec3 capsule_axis = rot_a[2].xyz.normalized();

        const vec3 capsule_bot = p_a - capsule_axis * (a.shape.capsule.length * 0.5f);
        const vec3 capsule_top = p_a + capsule_axis * (a.shape.capsule.length * 0.5f);

        const mat4 rot_b = o_b.to_mat4();
        const vec3 box_axes[3] = {
            rot_b[0].xyz.normalized(),
            rot_b[1].xyz.normalized(),
            rot_b[2].xyz.normalized(),
        };
        
        vec3 box_half_extents = b.shape.bounding_box.get_half_extents();

        vec3 local_capsule_bot = capsule_bot - p_b;
        vec3 local_capsule_top = capsule_top - p_b;
        vec3 closest_to_bot = p_b; 
        vec3 closest_to_top = p_b; 
        for (int i = 0; i < 3; ++i) 
        {
            const float projection_bot = local_capsule_bot.dot(box_axes[i]);
            const float clamped_projection_bot = std::clamp(projection_bot, -box_half_extents[i], box_half_extents[i]);
            closest_to_bot += box_axes[i] * clamped_projection_bot;
            
            const float projection_top = local_capsule_top.dot(box_axes[i]);
            const float clamped_projection_top = std::clamp(projection_top, -box_half_extents[i], box_half_extents[i]);
            closest_to_top += box_axes[i] * clamped_projection_top;
        }
        
        const vec3 delta_bot = closest_to_bot - capsule_bot;
        const vec3 delta_top = closest_to_top - capsule_top;
        const float dist_sq_bot = delta_bot.length_squared();
        const float dist_sq_top = delta_top.length_squared();

        if (dist_sq_bot > a.shape.capsule.radius * a.shape.capsule.radius && dist_sq_top > a.shape.capsule.radius * a.shape.capsule.radius)
        {
            return false;
        }

        if (dist_sq_bot < dist_sq_top)
        {
            result.normal = delta_bot.normalized();
            result.penetration = a.shape.capsule.radius - sqrtf(dist_sq_bot);
            result.contact_point = closest_to_bot; //TODO: check point
        }
        else
        {

            result.normal = delta_top.normalized();
            result.penetration = a.shape.capsule.radius - sqrtf(dist_sq_top);
            result.contact_point = closest_to_top; //TODO: check point
        }

        return true;
    }

    bool capsule_convex(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Capsule);
        assert(b.type == Collider::Convex_Hull);

        assert(false);
        return false;
    }

    bool cylinder_sphere(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Cylinder);
        assert(b.type == Collider::Sphere);

        if (sphere_cylinder(b, p_b, o_b, a, p_a, o_a, result))
        {
            result.normal = -result.normal;
            return true;
        }

        return false;
    }

    bool cylinder_capsule(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Cylinder);
        assert(b.type == Collider::Sphere);

        if (capsule_cylinder(b, p_b, o_b, a, p_a, o_a, result))
        {
            result.normal = -result.normal;
            return true;
        }

        return false;
    }

    bool cylinder_cylinder(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Cylinder);
        assert(b.type == Collider::Cylinder);

        const vec3 axis_a = o_a.mul(vec3::up_vector);
        const vec3 axis_b = o_b.mul(vec3::up_vector);

        const vec3 cylinder_base_a = p_a - a.shape.cylinder.height * 0.5f;
        const vec3 cylinder_base_b = p_b - b.shape.cylinder.height * 0.5f;

        float t_a = clamp((cylinder_base_b - cylinder_base_a).dot(axis_a) / (a.shape.cylinder.height * a.shape.cylinder.height), 0.0f, 1.0f);
        float t_b = clamp((cylinder_base_a - cylinder_base_b).dot(axis_b) / (b.shape.cylinder.height * b.shape.cylinder.height), 0.0f, 1.0f);

        const vec3 contact_point_a = cylinder_base_a + axis_a * t_a;
        const vec3 contact_point_b = cylinder_base_b + axis_b * t_b;

        vec3 delta = contact_point_a - contact_point_b;
        float dist2 = delta.length_squared();
        float sum_radii = a.shape.cylinder.radius + b.shape.cylinder.radius;

        if (dist2 > sum_radii * sum_radii)
        {
            return false;
        }

        result.normal = delta.normalized();
        result.penetration = sum_radii - sqrtf(dist2);
        //TODO: CHECK that it's the actual contact point
        result.contact_point = contact_point_a;

        return true;
    }

    bool cylinder_box(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Cylinder);
        assert(b.type == Collider::Box);

        const mat4 rot_a = o_a.to_mat4();
        const vec3 cylinder_axis = rot_a[2].xyz.normalized();

        const mat4 rot_b = o_b.to_mat4();
        const vec3 box_axes[3] = {
            rot_b[0].xyz.normalized(),
            rot_b[1].xyz.normalized(),
            rot_b[2].xyz.normalized(),
        };
        
        vec3 box_half_extents = b.shape.bounding_box.get_half_extents();

        const float t = clamp((p_b - p_a).dot(cylinder_axis) / (a.shape.cylinder.height * a.shape.cylinder.height), -0.5f, 0.5f);
        const vec3 closest_point = p_a + cylinder_axis * (t * a.shape.cylinder.height);

        vec3 local_point = closest_point - p_b; 
        vec3 closest_projected = p_b; 
        for (int i = 0; i < 3; ++i) 
        {
            float clamped_projection = clamp(local_point.dot(box_axes[i]), -box_half_extents[i], box_half_extents[i]);
            closest_projected += box_axes[i] * clamped_projection;
        }

        const vec3 delta = closest_projected - closest_point;
        const float dist_sq = delta.length_squared();

        if (dist_sq > a.shape.cylinder.radius * a.shape.cylinder.radius)
        {
            return false;
        }

        result.normal = delta.normalized();
        result.contact_point = closest_projected;

        vec3 d = cylinder_axis.cross(result.normal);
        // Check if cylinder is falling flat - check with height
        if (d.length_squared() < NEARLY_ZERO)
        {
            result.penetration = a.shape.cylinder.height * 0.5f - sqrtf(dist_sq);
        }
        else
        {
            result.penetration = a.shape.cylinder.radius - sqrtf(dist_sq);
        }

        return true;
    }

    bool cylinder_convex(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Cylinder);
        assert(b.type == Collider::Convex_Hull);

        assert(false);
        return false;
    }

    bool box_sphere(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Box);
        assert(b.type == Collider::Sphere);

        if (sphere_box(b, p_b, o_b, a, p_a, o_a, result))
        {
            result.normal = -result.normal;
            return true;
        }

        return false;
    }

    bool box_capsule(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Box);
        assert(b.type == Collider::Capsule);

        if (capsule_box(b, p_b, o_b, a, p_a, o_a, result))
        {
            result.normal = -result.normal;
            return true;
        }

        return false;
    }

    bool box_cylinder(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Box);
        assert(b.type == Collider::Cylinder);

        if (cylinder_box(b, p_b, o_b, a, p_a, o_a, result))
        {
            return true;
        }

        return false;
    }

    bool box_box(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Box);
        assert(b.type == Collider::Box);

        assert(false);
        return false;
    }

    bool box_convex(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Box);
        assert(b.type == Collider::Convex_Hull);

        assert(false);
        return false;
    }

    bool convex_sphere(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Convex_Hull);
        assert(b.type == Collider::Sphere);

        if (sphere_convex(b, p_b, o_b, a, p_a, o_a, result))
        {
            result.normal = -result.normal;
            return true;
        }

        return false;
    }

    bool convex_capsule(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Convex_Hull);
        assert(b.type == Collider::Capsule);

        if (capsule_convex(b, p_b, o_b, a, p_a, o_a, result))
        {
            result.normal = -result.normal;
            return true;
        }

        return false;
    }

    bool convex_cylinder(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Convex_Hull);
        assert(b.type == Collider::Cylinder);

        if (cylinder_convex(b, p_b, o_b, a, p_a, o_a, result))
        {
            result.normal = -result.normal;
            return true;
        }

        return false;
    }

    bool convex_box(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()
        
        assert(a.type == Collider::Convex_Hull);
        assert(b.type == Collider::Box);

        if (box_convex(b, p_b, o_b, a, p_a, o_a, result))
        {
            result.normal = -result.normal;
            return true;
        }

        return false;
    }

    // Massive thanks to https://winter.dev/ for extremely intuitive explanation and an implementation example
    bool convex_convex(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result)
    {
        PROFILE_FUNCTION()

        assert(a.type == Collider::Convex_Hull);
        assert(b.type == Collider::Convex_Hull);

        const int32 count_a = a.shape.convex_hull.count;
        const vec3 (&vertices_a)[CONVEX_HULL_MAX_NUM_VERTICES] = a.shape.convex_hull.vertices;
        
        const int32 count_b = b.shape.convex_hull.count;
        const vec3 (&vertices_b)[CONVEX_HULL_MAX_NUM_VERTICES] = b.shape.convex_hull.vertices;

        vec3 simplex[4];

        AABB combined_bounds = a.bounds;
        combined_bounds.expand(b.bounds);

        if (!Collision_Helpers::gjk(vertices_a, count_a, p_a, o_a, vertices_b, count_b, p_b, o_b, combined_bounds.max, simplex))
        {
            return false;
        }

        return Collision_Helpers::epa(vertices_a, count_a, p_a, o_a, vertices_b, count_b, p_b, o_b, simplex, result);
    }
}
