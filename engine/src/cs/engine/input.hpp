// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/engine/event.hpp"
#include "cs/name_id.hpp"
#include "cs/engine/singleton.hpp"
#include "cs/memory/shared_ptr.hpp"
#include "cs/containers/hash_map.hpp"

class Input_Source
{
public:
    Event<Name_Id, bool> on_digital_input_generated;
    Event<Name_Id, float> on_analog_input_generated;
};

struct Input_Pair
{
    Name_Id id;
    float multiplier;
};

struct Input
{
    Name_Id id;
    Hash_Map<Name_Id, float> multipliers;
    Event<float, float> on_updated;
};

class Input_System : public Singleton<Input_System>
{
public:
    void register_input_source(const Shared_Ptr<Input_Source>& in_input_source);
    Event<bool>& register_digital_input(const Name_Id& id, const Dynamic_Array<Input_Pair>& inputs);
    Event<float, float>& register_input(const Name_Id& id, const Dynamic_Array<Input_Pair>& inputs);

private:
    Hash_Map<Name_Id, Shared_Ptr<Input>> _inputs;
};

float axis_deadzone(float value, float threshold);
float axis_map_deadzone(float value, float threshold);