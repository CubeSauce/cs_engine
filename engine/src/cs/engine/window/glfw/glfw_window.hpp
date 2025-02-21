#pragma once

#include "cs/engine/window.hpp"
#include "cs/memory/shared_ptr.hpp"

class Input_Source;

struct GLFWwindow;
struct GLFWgamepadstate;

class GLFW_Window : public Window 
{
public:
    Shared_Ptr<Input_Source> input_source;

public:
    virtual ~GLFW_Window();
    
    virtual bool initialize(int width, int height, const char* title) override;
    virtual void poll_events() override;
    virtual bool should_close() const override;
    virtual void swap_buffers() override;
    virtual void* native_handle() const override;

private:
    GLFWwindow* _window { nullptr };
    GLFWgamepadstate *_gamepad_state { nullptr };
    GLFWgamepadstate *_previous_gamepad_state { nullptr };

    void _poll_analog_inputs();
};