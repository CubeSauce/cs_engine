#pragma once

#include "cs/name_id.hpp"
#include "cs/engine/window.hpp"
#include "cs/memory/shared_ptr.hpp"

class Input_Source;

struct GLFWwindow;
struct GLFWgamepadstate;

class GLFW_Window : public Window 
{
public:
    Event<Name_Id, float> input_source;

public:
    virtual ~GLFW_Window();
    
    virtual bool initialize(int width, int height, const char* title) override;
    virtual void poll_events() override;
    virtual bool should_close() const override;
    virtual void swap_buffers() override;
    virtual void* native_handle() const override;

    virtual void get_window_size(uint32& width, uint32& height) const override;
private:
    GLFWwindow* _window { nullptr };
    GLFWgamepadstate *_gamepad_state { nullptr };
    GLFWgamepadstate *_previous_gamepad_state { nullptr };
    uint32 _width { 0 }, _height { 0 };

    void _poll_analog_inputs();
};