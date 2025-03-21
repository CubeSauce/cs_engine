// CS Engine
// Author: matija.martinec@protonmail.com

#pragma once

#include "cs/cs.hpp"
#include "cs/engine/cvar.hpp"
#include "cs/engine/event.hpp"
#include "cs/engine/window.hpp"
#include "cs/memory/unique_ptr.hpp"
#include "cs/engine/input.hpp"
#include "cs/engine/task_system.hpp"
#include "cs/engine/game/game_instance.hpp"
#include "cs/engine/net/net_instance.hpp"
#include "cs/containers/dynamic_array.hpp"
#include "cs/engine/renderer/renderer.hpp"

#include <string>

#include "openvr.h"

class VR_System;
class CVar_Registry;
class Net_Instance;
class Game_Instance;
class Physics_System;
class Thread_Pool;
class Profiler;
class Engine : public Singleton<Engine>
{
public:
    Shared_Ptr<Game_Instance> game_instance;

    Shared_Ptr<Shader_Resource> default_texture_shader_resource;
    Shared_Ptr<Shader_Resource> default_color_shader_resource;

    Shared_Ptr<Texture_Resource> default_texture_resource;

public:
    Engine() = default;

    void initialize(const Dynamic_Array<std::string>& args);
    void shutdown();

    void run();

private:
    Shared_Ptr<Profiler> _profiler;
    Shared_Ptr<CVar_Registry> _cvar_registry;
    Shared_Ptr<Thread_Pool> _thread_pool;
    Shared_Ptr<Renderer> _renderer;
    Shared_Ptr<Window> _window;
    Shared_Ptr<Input_System> _input_system;
    Shared_Ptr<Physics_System> _physics_system;

    Shared_Ptr<VR_System> _vr_system;
    Shared_Ptr<Net_Instance> _net_instance;

    bool _should_close { false };

    Task_Graph game_loop_task_graph;

private:
    Shared_Ptr<CVar_T<uint32>> _cvar_num_threads;
    Shared_Ptr<CVar_T<uint8>> _cvar_net_role;
    Shared_Ptr<CVar_T<uint32>> _cvar_window_width;
    Shared_Ptr<CVar_T<uint32>> _cvar_window_height;
    Shared_Ptr<CVar_T<std::string>> _cvar_window_title;
    Shared_Ptr<CVar_T<uint8>> _cvar_renderer_api;
    Shared_Ptr<CVar_T<bool>> _cvar_vr_support;
    Shared_Ptr<CVar_T<float>> _cvar_dt_mult;

private:
    void _parse_args(const Dynamic_Array<std::string>& args);
    void _initialize_cvars();

    Shared_Ptr<Window> _create_window();
    Shared_Ptr<Renderer_Backend> _create_renderer_backend(Renderer_API::Type api, const Shared_Ptr<Window>& window);

    void _initialize_defaults();

    void _task_physics();
    void _task_animation();
    void _task_render();
};