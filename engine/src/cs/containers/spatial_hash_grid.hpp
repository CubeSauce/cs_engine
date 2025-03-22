// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/math/math.hpp"
#include "cs/engine/name_id.hpp"
#include "cs/containers/dynamic_array.hpp"

#include <unordered_map>

class Spatial_Hash_Grid
{
public:
    struct Cell
    {
        bool dirty { false };
        Dynamic_Array<Name_Id> object_ids;
    };

public:
    Spatial_Hash_Grid() = default;
    Spatial_Hash_Grid(float cell_size);
    void add(const Name_Id& in_id, const AABB& in_bounds);
    void update(const Name_Id& in_id, const AABB& in_bounds);
    int32 get_potential_collisions(const Name_Id& in_id, const AABB& in_bounds, Dynamic_Array<Name_Id>& out_potential_colliders);

    void sweep_and_prune_cells(Dynamic_Array<Pair<Name_Id, Name_Id>>& out_potential_collision_pairs);

private:
    float _cell_size { 5.0f };
    std::unordered_map<uint32, Dynamic_Array<int32>> _id_to_hash;
    std::unordered_map<uint32, AABB> _bounds;
    std::unordered_map<int32, Cell> _cells;

private:
    bool _check_aabb_intersection(const Name_Id& a_id, const Name_Id& b_id);
    void _get_cells_for_bounds(const AABB& in_bounds, ivec3& out_min, ivec3& out_max) const;
    int32 _hash(int32 x, int32 y, int32 z) const;
};
