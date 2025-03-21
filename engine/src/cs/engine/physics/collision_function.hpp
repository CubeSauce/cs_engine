// CS Engine
// Author: matija.martinec@protonmail.com

// Huge thanks to https://winter.dev/

#pragma once

#include "cs/cs.hpp"
#include "cs/math/math.hpp"
#include "cs/engine/name_id.hpp"

struct Collider;
struct Collision_Result;
namespace Collision_Test_Function
{
    typedef bool (*Definition)(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result);

    bool sphere_sphere(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result);
    bool sphere_capsule(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result);
    bool capsule_sphere(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result);
    bool capsule_capsule(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result);
    bool convex_convex(const Collider& a, const vec3& p_a, const quat& o_a, const Collider& b, const vec3& p_b, const quat& o_b, Collision_Result& result);
}

namespace Collision_Helpers
{
    mat4 inertia_tensor(const Collider& collider, const float mass);
};
