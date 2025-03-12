// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/math.hpp"
#include "cs/engine/name_id.hpp"
#include "cs/containers/dynamic_array.hpp"
#include "cs/containers/hash_table.hpp"

#include <unordered_map>

struct AABB_Box
{
    vec3 min, max;   

    bool intersects(const AABB_Box& other)
    {
        return 
            min.x < other.max.x && max.x >= other.min.x &&
            min.y < other.max.y && max.y >= other.min.y &&
            min.z < other.max.z && max.z >= other.min.z;
    } 
};

struct Physics_Object
{
    Name_Id id;
    AABB_Box bounds;
};

class Spatial_Hash_Grid
{
public:
    struct Cell
    {
        bool dirty { false };
        Dynamic_Array<Name_Id> ids;
    };

public:
    void add(const Name_Id& id, const AABB_Box& bounds)
    {
        PROFILE_FUNCTION()

        Physics_Object obj { .id = id, .bounds = bounds };

        _objects[id] = obj;

        vec3 min_cell = { std::floor(obj.bounds.min.x / _cell_size),
                        std::floor(obj.bounds.min.y / _cell_size),
                        std::floor(obj.bounds.min.z / _cell_size) };

        vec3 max_cell = { std::floor(obj.bounds.max.x / _cell_size),
                        std::floor(obj.bounds.max.y / _cell_size),
                        std::floor(obj.bounds.max.z / _cell_size) };

        for (int x = static_cast<int>(min_cell.x); x <= static_cast<int>(max_cell.x); x++)
        {
            for (int y = static_cast<int>(min_cell.y); y <= static_cast<int>(max_cell.y); y++)
            {
                for (int z = static_cast<int>(min_cell.z); z <= static_cast<int>(max_cell.z); z++)
                {
                    size_t hashKey = _hash(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
                    _grid[hashKey].ids.add(obj.id);
                    _grid[hashKey].dirty = true;
                }
            }
        }
    }

    void update_obj(const Name_Id& in_id, const AABB_Box& bounds)
    {
        PROFILE_FUNCTION()

        if (_objects.find(in_id) == _objects.end())
        {
            add(in_id, bounds);
            return;
        }

        Physics_Object& obj = _objects[in_id];

        {
            vec3 min_cell = obj.bounds.min;
            vec3 max_cell = obj.bounds.max;
            for (int x = static_cast<int>(min_cell.x); x <= static_cast<int>(max_cell.x); x++)
            {
                for (int y = static_cast<int>(min_cell.y); y <= static_cast<int>(max_cell.y); y++)
                {
                    for (int z = static_cast<int>(min_cell.z); z <= static_cast<int>(max_cell.z); z++)
                    {
                        size_t hash_key = _hash(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
                        if (_grid.find(hash_key) != _grid.end())
                        {
                            _grid[hash_key].ids.remove_first(in_id);
                            _grid[hash_key].dirty = true;
                        }
                    }
                }
            }
        }
        {
            vec3 min_cell = bounds.min;
            vec3 max_cell = bounds.max;
            for (int x = static_cast<int>(min_cell.x); x <= static_cast<int>(max_cell.x); x++)
            {
                for (int y = static_cast<int>(min_cell.y); y <= static_cast<int>(max_cell.y); y++)
                {
                    for (int z = static_cast<int>(min_cell.z); z <= static_cast<int>(max_cell.z); z++)
                    {
                        size_t hash_key = _hash(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
                        if (_grid.find(hash_key) != _grid.end())
                        {
                            _grid[hash_key].ids.add(in_id);
                            _grid[hash_key].dirty = true;
                        }
                    }
                }
            }
        }
    }

    int32 get_potential_collisions(const Name_Id& in_id, Dynamic_Array<Name_Id>& out_potential_collisions)
    {
        PROFILE_FUNCTION()

        if (_objects.find(in_id) == _objects.end())
        {
            return 0;
        }

        Physics_Object& obj = _objects[in_id];

        vec3 min_cell = { std::floor(obj.bounds.min.x / _cell_size),
                        std::floor(obj.bounds.min.y / _cell_size),
                        std::floor(obj.bounds.min.z / _cell_size) };

        vec3 max_cell = { std::floor(obj.bounds.max.x / _cell_size),
                        std::floor(obj.bounds.max.y / _cell_size),
                        std::floor(obj.bounds.max.z / _cell_size) };

        int32 count = 0;
        // Query neighboring grid cells
        for (int x = static_cast<int>(min_cell.x); x <= static_cast<int>(max_cell.x); x++)
        {
            for (int y = static_cast<int>(min_cell.y); y <= static_cast<int>(max_cell.y); y++)
            {
                for (int z = static_cast<int>(min_cell.z); z <= static_cast<int>(max_cell.z); z++)
                {
                    size_t hash_key = _hash(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
                    if (_grid.find(hash_key) != _grid.end())
                    {
                        for (const Name_Id& id : _grid[hash_key].ids)
                        {
                            if (id != in_id)
                            {
                                out_potential_collisions.add(id);
                                count++;
                            }
                        }
                    }
                }
            }
        }

        return count;
    }
    
    void check_collision(const Name_Id& a_id, const Name_Id& b_id)
    {
        if (!_objects[a_id].bounds.intersects(_objects[b_id].bounds))
        {
            return;
        }


    }

public:
    float _cell_size { 5.0f };
    std::unordered_map<uint32, Physics_Object> _objects;
    std::unordered_map<size_t, Cell> _grid;

    int32 _hash(float x, float y, float z) const 
    {
        int _x = static_cast<int>(std::floor(x / _cell_size));
        int _y = static_cast<int>(std::floor(y / _cell_size));
        int _z = static_cast<int>(std::floor(z / _cell_size));
        return (_x * 73856093) ^ (_y * 19349663) ^ (_z * 83492791);
    }
};
