#pragma once

#include "cs/engine/window.hpp"

struct GLFWwindow;
class GLFW_Window : public Window 
{
public:
    virtual ~GLFW_Window();
    
    virtual bool initialize(int width, int height, const char* title) override;
    virtual void poll_events() override;
    virtual bool should_close() const override;
    virtual void swap_buffers() override;
    virtual void* native_handle() const override;

private:
    GLFWwindow* _window = nullptr;
};