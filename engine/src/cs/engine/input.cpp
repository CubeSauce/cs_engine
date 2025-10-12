#include "cs/engine/input.hpp"

#include <absl/strings/internal/str_format/extension.h>

#include "cs/math/math.hpp"
#include "profiling/profiler.hpp"

template<> 
Input_System* Singleton<Input_System>::_singleton { nullptr };

void Input_System::update()
{
    PROFILE_FUNCTION()

    for (const Name_Id& event_id : _changed_events)
    {
        Event_State& event_state = _event_map.find_or_add(event_id);

        float event_value = 0.0f;
        for (const Pair<Name_Id, float>& pair : event_state._input_to_multiplier_map)
        {
            event_value += _input_value_map.find_or_add(pair.a) * pair.b;
        }

        event_state.event.broadcast(event_value);
    }

    _changed_events.clear();
}

void Input_System::register_input_source(Event<Name_Id, float>& on_input_generated)
{
    PROFILE_FUNCTION()

    on_input_generated.bind([this](const Name_Id& input_id, float value)
    {
        PROFILE_FUNCTION()

        _input_value_map.find_or_add(input_id) = value;

        Dynamic_Array<Name_Id>* p_event_ids = _input_to_events_map.find(input_id);
        if (p_event_ids == nullptr)
        {
            return;
        }

        for (const Name_Id& event_id : *p_event_ids)
        {
            _changed_events.push_back(event_id);
        }
    });
}

Event<float>& Input_System::register_event(const Name_Id &event_id, const Dynamic_Array<Pair<Name_Id, float>>& inputs)
{
    PROFILE_FUNCTION()

    Event_State& event_state = _event_map.find_or_add(event_id);

    for (const Pair<Name_Id, float>& pair : inputs)
    {
        if (event_state._input_to_multiplier_map.find_first(pair) == -1)
        {
            event_state._input_to_multiplier_map.push_back(pair);
        }

        _input_value_map.insert(pair.a, 0.0f);

        Dynamic_Array<Name_Id>& events = _input_to_events_map.find_or_add(pair.a);
        if (events.find_first(event_id) == -1)
        {
            events.push_back(event_id);
        }
    }

    return event_state.event;
}

void Input_System::deregister_event(const Name_Id &event_id)
{
    PROFILE_FUNCTION()

    Event_State* event_state = _event_map.find(event_id);
    if (event_state == nullptr)
    {
        return;
    }

    for (const Pair<Name_Id, float>& input : event_state->_input_to_multiplier_map)
    {
        _input_value_map.erase(input.a);
        _input_to_events_map.find_or_add(input.a).erase_if([event_id](const Name_Id& id){ return id == event_id; });
    }
}

float axis_deadzone(float value, float threshold)
{
    return (fabs(value) < threshold) ? 0.0f : value;
}

float axis_map_deadzone(float value, float threshold)
{
    if ((fabs(value) < threshold))
    {
        return 0;
    }

    if (SIGN(value) < 0)
    {
        // (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
        float t = -map(-value, 0.0f, 1.0f, threshold, 1.0f);
        return t;
    }
 
    float t = map(value, 0.0f, 1.0f, threshold, 1.0f);
    return t;
}
