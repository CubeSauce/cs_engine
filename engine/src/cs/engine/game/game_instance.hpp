#pragma once

#include "cs/cs.hpp"
#include "cs/math/math.hpp"
#include "cs/engine/name_id.hpp"
#include "cs/memory/shared_ptr.hpp"
#include "cs/containers/hash_table.hpp"
#include "cs/containers/dynamic_array.hpp"

template<typename Type>
struct Component_Container
{
    Dynamic_Array<Type> components;
    Hash_Table<int32> id_to_index;
    Hash_Table<Name_Id> index_to_id;

    void add(const Name_Id& id, const Type& type)
    {
        if (id_to_index[id] != 0)
        {
            return;
        }

        int32 new_index = components.size();
        id_to_index[id] = new_index;
        components.add(type);
    }

    void remove(const Name_Id& id)
    {
        if (id_to_index[id] != 0)
        {
            return;
        }

        int32 index = id_to_index[id];
        int32 last_index = components.size() - 1;

        if (index != last_index) {
            components[index] = components[last_index];
            Name_Id& id = index_to_id[last_index];
            id_to_index[lastEntity] = index;
            index_to_id[index] = lastEntity;
        }

        id_to_index.erase(entity);
        index_to_id.erase(last_index);
        components.pop_back();
    }

    Type* get(const Name_Id& name)
    {
        int32 index = id_to_index[name];
        if (index == -1)
        {
            return nullptr;
        }

        return &components[index];
    }
};

class Renderer;
class Game_Instance
{
public:
    virtual ~Game_Instance() = default;

    virtual void init() = 0;
    virtual void update(float dt) = 0;
    virtual void render(const Shared_Ptr<Renderer>& renderer) = 0;
    virtual void shutdown() = 0;
};
