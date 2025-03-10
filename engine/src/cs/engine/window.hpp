// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/engine/event.hpp"

namespace Window_Framework
{
    enum Type : uint8
    {
        None,
        GLFW,
        SDL
    };
}

class Window
{
public:
    Event<> on_window_should_close;
    Event<uint32, uint32> on_window_resize;

public:
    virtual ~Window() = default;
    virtual bool initialize(int width, int height, const char* title) = 0;
    virtual void poll_events() = 0;
    virtual bool should_close() const = 0;
    virtual void swap_buffers() = 0;
    virtual void* native_handle() const = 0;
};