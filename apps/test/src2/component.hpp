#pragma once

#include "cs/cs.hpp"
#include "cs/math/math.hpp"
#include "cs/name_id.hpp"
#include "cs/memory/shared_ptr.hpp"
#include "cs/containers/dynamic_array.hpp"
#include "cs/containers/hash_map.hpp"
#include "cs/engine/physics/physics_system.hpp"

struct Component
{
    Name_Id component_id;
    bool dirty;
};
DECLARE_PRINT_TYPE(Component)

template<Derived<Component> Type>
struct Component_Container
{
    static inline const Name_Id component_type_name = Name_Id(print_type<Type>());

    Dynamic_Array<Type> components;
    Hash_Map<Name_Id, int64> id_to_index;
    Hash_Map<int64, Name_Id> index_to_id;

    Type& add(const Name_Id& id)
    {
        // Don't add we already have something with this id
        int64* p_index = id_to_index.find(id);
        if (p_index)
        {
            return components[*p_index];
        }

        int64 new_index = components.size();
        id_to_index.insert(id, new_index);
        index_to_id.insert(new_index, id);
        components.push_back(Type());
        //components[new_index].entity_id = id;
        components[new_index].dirty = true;
        return components[new_index]; 
    }

    Type* get(const Name_Id& id)
    {
        int64* p_index = id_to_index.find(id);
        if (p_index == nullptr)
        {
            return nullptr;
        }

        return &components[*p_index];
    }
};

struct Transform_Component : public Component
{
    Name_Id parent_id;

    vec3 local_position { vec3::zero_vector };
    quat local_orientation { quat::zero_quat };
    mat4 local_to_world { mat4(1.0f) };

    mat4 get_world_matrix() const;
    vec3 get_world_position() const;
    quat get_world_orientation() const;
};
DECLARE_PRINT_TYPE(Transform_Component)

struct Physics_Body_Component : public Component
{
    Physics_Body::Type type;
    Collider collider;
    float mass{1};
};
DECLARE_PRINT_TYPE(Physics_Body_Component)

class Mesh_Resource;
struct Render_Component : public Component
{
    Shared_Ptr<Mesh_Resource> mesh;
    bool visible { true };
};
DECLARE_PRINT_TYPE(Render_Component)
