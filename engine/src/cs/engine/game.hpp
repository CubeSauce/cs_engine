#pragma once

#include "cs/cs.hpp"
#include "cs/memory/shared_ptr.hpp"
#include "cs/containers/dynamic_array.hpp"

class Game_State
{
    void update(float dt);
    void serialize(Dynamic_Array<uint8>& out_data);
    void deserialize(const Dynamic_Array<uint8>& in_data);
};

class App
{
    
};
