// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/engine/event.hpp"
#include "cs/name_id.hpp"
#include "cs/engine/singleton.hpp"
#include "cs/memory/shared_ptr.hpp"
#include "cs/containers/hash_map.hpp"

namespace Input_Modifier
{
enum Type : uint8_t
{
    Shift       = 1 << 0,
    Control     = 1 << 1,
    Alt         = 1 << 2,
    Super       = 1 << 3,
    Caps_Lock   = 1 << 4,
    Num_Lock    = 1 << 5
};
}

class Input_System : public Singleton<Input_System>
{
public:
    void update();

    void register_input_source(Event<Name_Id, float>& on_input_generated);
    Event<float>& register_event(const Name_Id &event_id,
        const Dynamic_Array<Pair<Name_Id, float>>& inputs);
    void deregister_event(const Name_Id &event_id);

private:
    struct Event_State
    {
        Event<float> event;
        Dynamic_Array<Pair<Name_Id, float>> _input_to_multiplier_map;
    };

    Hash_Map<Name_Id, float> _input_value_map;
    Hash_Map<Name_Id, Dynamic_Array<Name_Id>> _input_to_events_map;
    Hash_Map<Name_Id, Event_State> _event_map;
    Dynamic_Array<Name_Id> _changed_events;
};

float axis_deadzone(float value, float threshold);
float axis_map_deadzone(float value, float threshold);