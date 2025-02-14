// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/engine/cvar.hpp"
#include "cs/engine/event.hpp"
#include "cs/engine/window.hpp"
#include "cs/memory/unique_ptr.hpp"
#include "cs/engine/net/net_instance.hpp"
#include "cs/containers/dynamic_array.hpp"
#include "cs/engine/renderer/renderer.hpp"

#include <string>

#include "openvr.h"

namespace Renderer_API
{
    enum Type : uint8
    {
        None,
        OpenGL,
        Vulkan,
        DirectX11,
        DirectX12,
    };
}

namespace Window_Framework
{
    enum Type : uint8
    {
        None,
        GLFW,
        SDL
    };
}

struct Engine_Create_Descriptor
{
    Window_Framework::Type window_framework { Window_Framework::GLFW };
    uint32 window_width { 800 };
    uint32 window_height { 600 };
    const char* window_title = "CS Engine Window";
    Renderer_API::Type renderer_api { Renderer_API::DirectX11 };
};

class CVar_Registry;
class Net_Instance;
class Game_Instance;
class VR_System;
class Engine
{
public:
    Shared_Ptr<Game_Instance> game_instance;

public:
    Engine() = default;

    void initialize(const Dynamic_Array<std::string>& args);
    void shutdown();

    void run();

private:
    Shared_Ptr<CVar_Registry> _cvar_registry;
    Shared_Ptr<Net_Instance> _net_instance;
    Shared_Ptr<Renderer> _renderer;
    Shared_Ptr<VR_System> _vr_system;

    // TODO: Add these as a separate system

    bool _should_close { false };

private:
    Shared_Ptr<CVar_T<uint8>> _cvar_net_role;
    Shared_Ptr<CVar_T<uint32>> _cvar_window_width;
    Shared_Ptr<CVar_T<uint32>> _cvar_window_height;
    Shared_Ptr<CVar_T<std::string>> _cvar_window_title;
    Shared_Ptr<CVar_T<uint8>> _cvar_renderer_api;
    Shared_Ptr<CVar_T<bool>> _cvar_vr_support;

private:
    void _parse_args(const Dynamic_Array<std::string>& args);
    void _initialize_cvars();

    Shared_Ptr<Window> _create_window();
    Shared_Ptr<Renderer_Backend> _create_renderer_backend(Renderer_API::Type api, const Shared_Ptr<Window>& window);
};