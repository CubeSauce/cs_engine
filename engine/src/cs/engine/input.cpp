#include "cs/engine/input.hpp"
#include "cs/math/math.hpp"

template<> 
Input_System* Singleton<Input_System>::_singleton { nullptr };

void Input_System::register_input_source(
    const Shared_Ptr<Input_Source>& in_input_source)
{
    in_input_source->on_digital_input_generated.bind([&](const Name_Id& input_id, bool value)
    {
        if (Shared_Ptr<Input>* p_input = _inputs.find(input_id))
        {
            if (Shared_Ptr<Input>& input = (*p_input))
            {
                if (float* p_mul = input->multipliers.find(input_id))
                {
                    input->on_updated.broadcast(value ? 1.0f : 0.0f, *p_mul);
                }
            }
        }
    });

    //  TODO: This doesn't work so well, should be: poll all inputs, then combine their values, and then send out events
    in_input_source->on_analog_input_generated.bind([&](const Name_Id& input_id, float value)
    {
        if (Shared_Ptr<Input>* p_input = _inputs.find(input_id))
        {
            if (Shared_Ptr<Input>& input = (*p_input))
            {
                if (float* p_mul = input->multipliers.find(input_id))
                {
                    input->on_updated.broadcast(value, *p_mul);
                }
            }
        }
    });
}

Event<float, float>& Input_System::register_input(const Name_Id& id, const Dynamic_Array<Input_Pair>& inputs)
{
    Shared_Ptr<Input> input = Shared_Ptr<Input>::create();
    input->id = id;
    
    for (const Input_Pair& input_pair : inputs)
    {
        input->multipliers.insert(input_pair.id, input_pair.multiplier);
        _inputs.insert(input_pair.id, input);
    }

    return input->on_updated;
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
