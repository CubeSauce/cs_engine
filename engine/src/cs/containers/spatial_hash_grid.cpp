// CS Engine
// Author: matija.martinec@protonmail.com

#include "cs/containers/spatial_hash_grid.hpp"
#include "cs/engine/profiling/profiler.hpp"

#include <algorithm>

Spatial_Hash_Grid::Spatial_Hash_Grid(float cell_size)
    :_cell_size(cell_size)
{
    assert(fabs(_cell_size) > NEARLY_ZERO);
}

void Spatial_Hash_Grid::add(const Name_Id& in_id, const AABB& in_bounds)
{
    PROFILE_FUNCTION()

    if (_id_to_hash.find(in_id) != _id_to_hash.end())
    {
        return;
    }

    _bounds[in_id] = in_bounds;

    ivec3 min, max;
    _get_cells_for_bounds(in_bounds, min, max);
    for (int32 x = min.x; x <= max.x; x++)
    {
        for (int32 y = min.y; y <= max.y; y++)
        {
            for (int32 z = min.z; z <= max.z; z++)
            {
                int32 hash = _hash(x, y, z);

                Cell& cell = _cells[hash];
                cell.dirty = true;
                cell.object_ids.push_back(in_id);
                _id_to_hash[in_id].push_back(hash);
            }   
        }   
    }
}

void Spatial_Hash_Grid::update(const Name_Id& in_id, const AABB& in_bounds)
{
    PROFILE_FUNCTION()
    
    if (_id_to_hash.find(in_id) != _id_to_hash.end())
    {
        Dynamic_Array<int32>& previous_hashes = _id_to_hash[in_id];
        for (int32 hash : previous_hashes)
        {
            Cell& cell = _cells.at(hash);
            cell.dirty = true;
            cell.object_ids.erase_if([in_id](const Name_Id& value){ return value == in_id; });
        }
        _id_to_hash.erase(in_id);
    }

    add(in_id, in_bounds);
}

int32 Spatial_Hash_Grid::get_potential_collisions(const Name_Id& in_id, const AABB& in_bounds, Dynamic_Array<Name_Id>& out_potential_colliders)
{
    PROFILE_FUNCTION()

    int32 count = 0;

    ivec3 min, max;
    _get_cells_for_bounds(in_bounds, min, max);

    for (int32 x = min.x - 1; x <= max.x + 1; x++)
    {
        for (int32 y = min.y - 1; y <= max.y + 1; y++)
        {
            for (int32 z = min.z - 1; z <= max.z + 1; z++)
            {
                int32 hash = _hash(x, y, z);
                if (_cells.find(hash) == _cells.end())
                {
                    continue;
                }

                for (const Name_Id& id : _cells[hash].object_ids)
                {
                    if (id == in_id || !_check_aabb_intersection(in_id, id))
                    {
                        continue;
                    }

                    if (out_potential_colliders.find_first(id) == -1)
                    {
                        out_potential_colliders.push_back(id);
                    }
                    count++;
                }
            }   
        }   
    }

    return count;
}

void Spatial_Hash_Grid::sweep_and_prune_cells(Dynamic_Array<Pair<Name_Id, Name_Id>>& out_potential_collision_pairs)
{
    PROFILE_FUNCTION()

    //TODO: remove only unchanged ones
    out_potential_collision_pairs.clear();
    for (auto& [id, cell] : _cells)
    {
        std::sort(cell.object_ids.begin(), cell.object_ids.end(), [&](const Name_Id& a, const Name_Id& b){
            return _bounds[a].min.x < _bounds[b].min.x;
        });

        for (int32 ai = 0; ai < cell.object_ids.size(); ++ai)
        {
            const Name_Id& a = cell.object_ids[ai];
            const AABB& bounds_a = _bounds[a];
            for (int32 bi = ai + 1; bi < cell.object_ids.size(); ++bi)
            {
                const Name_Id& b = cell.object_ids[bi];
                if (a == b)
                {
                    continue;
                }

                const AABB& bounds_b = _bounds[b];

                if (bounds_b.min.x > bounds_a.max.x)
                {
                    break;
                }

                if (bounds_a.intersects(bounds_b))
                {
                    // Don't add already detected pairs
                    Pair<Name_Id, Name_Id> pair(a, b);
                    Pair<Name_Id, Name_Id> inverse_pair(b, a);
                    if (out_potential_collision_pairs.find_first(pair) == -1 &&
                    out_potential_collision_pairs.find_first(inverse_pair) == -1)
                    {
                        out_potential_collision_pairs.push_back(pair);
                    }
                }
            }
        }
    }
}

bool Spatial_Hash_Grid::_check_aabb_intersection(const Name_Id& a_id, const Name_Id& b_id)
{
    return _bounds[a_id].intersects(_bounds[b_id]);
}

void Spatial_Hash_Grid::_get_cells_for_bounds(const AABB& in_bounds, ivec3& out_min, ivec3& out_max) const 
{
    out_min = { 
        static_cast<int32>(std::floor(in_bounds.min.x / _cell_size)),
        static_cast<int32>(std::floor(in_bounds.min.y / _cell_size)),
        static_cast<int32>(std::floor(in_bounds.min.z / _cell_size))
    };

    out_max = { 
        static_cast<int32>(std::floor(in_bounds.max.x / _cell_size)),
        static_cast<int32>(std::floor(in_bounds.max.y / _cell_size)),
        static_cast<int32>(std::floor(in_bounds.max.z / _cell_size))
    };
}

int32 Spatial_Hash_Grid::_hash(int32 x, int32 y, int32 z) const
{
    int32 _x = static_cast<int>(std::floor(x / _cell_size));
    int32 _y = static_cast<int>(std::floor(y / _cell_size));
    int32 _z = static_cast<int>(std::floor(z / _cell_size));
    return (_x * 73856093) ^ (_y * 19349663) ^ (_z * 83492791);
}
