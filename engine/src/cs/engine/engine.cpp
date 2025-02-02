#include "cs/engine/engine.hpp"
#include "cs/engine/cvar.hpp"
#include "cs/engine/net/net_instance.hpp"
#include "cs/engine/window/glfw/glfw_window.hpp"

#include "cs/engine/renderer/directx/directx_renderer.hpp"
#include "cs/engine/renderer/opengl/opengl_renderer.hpp"

#include "cs/engine/game/game_instance.hpp"


void Engine::initialize(const Dynamic_Array<std::string>& args)
{
    _initialize_cvars();
    _parse_args(args);

    _net_instance = Shared_Ptr<Net_Instance>::create((Net_Role::Type)_cvar_net_role->get());

    if (_net_instance->is_local())
    {
        _renderer = Shared_Ptr<Renderer>::create();
        _create_renderer_backend((Renderer_API::Type)_cvar_renderer_api->get(), _create_window());

        Shared_Ptr<Perspective_Camera> p_camera = Shared_Ptr<Perspective_Camera>::create();
        p_camera->FOV_deg = 45.0f;
        p_camera->near_d = 0.0f;
        p_camera->far_d = 10.0f;
        p_camera->position = {0.0f, -18.0f, 3.0f};
        p_camera->target = {0.0f, 0.0f, 3.0f};

        _renderer->set_active_camera(p_camera);
    }
}

void Engine::shutdown()
{
}

void Engine::run()
{
    // TODO: Separate game and editor instances
    if (_net_instance->is_local())
    {
        _renderer->window->on_window_should_close.bind([&](){
            _should_close = true;
        });
    }

    if (game_instance)
    {
        game_instance->init();
    }

    const float dt = 1/60.0f;
    while(!_should_close)
    {
        _net_instance->update(dt);

        if (game_instance)
        {
            game_instance->update(dt);
        }

        if (_renderer)
        {
            _renderer->begin_frame();
            
            if (game_instance)
            {
                game_instance->render(_renderer);
            }

            _renderer->end_frame();
            _renderer->render_frame();
            
            _renderer->window->swap_buffers();
            _renderer->window->poll_events();
        }
    }

    if (game_instance)
    {
        game_instance->shutdown();
    }
}

Dynamic_Array<std::string> split(const std::string& str, char delim)
{
    Dynamic_Array<std::string> tokens;
    
    size_t last_token = 0;
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == delim)
        {
            tokens.add(str.substr(last_token, i));
            last_token = i + 1;
        }
    }

    tokens.add(str.substr(last_token, str.size()));

    return tokens;
}

void Engine::_parse_args(const Dynamic_Array<std::string>& args)
{
    for (std::string& arg : args)
    {
        Dynamic_Array<std::string> tokens = split(arg, '=');
        if (tokens.size() == 2)
        {
            if (Shared_Ptr<CVar> c_var = _cvar_registry->get_cvar(tokens[0].c_str()))
            {
                c_var->set_from_string(tokens[1]);
            }
        }
    }
}

void Engine::_initialize_cvars()
{
    _cvar_registry = Shared_Ptr<CVar_Registry>::create();

    _cvar_net_role = _cvar_registry->register_cvar<uint8>(
        "cs_net_role", Net_Role::Offline, "Which net role will the instance be.");
    _cvar_window_width = _cvar_registry->register_cvar<uint32>(
        "cs_window_width", 1280, "Width of the instance window");
    _cvar_window_height = _cvar_registry->register_cvar<uint32>(
        "cs_window_height", 720, "Height of the instance window");
    _cvar_window_title = _cvar_registry->register_cvar<std::string>(
        "cs_window_title", "CS Engine app", "Title of the instance window");
    _cvar_renderer_api = _cvar_registry->register_cvar<uint8>(
        "cs_renderer_api", Renderer_API::DirectX11, "Which API are we using for rendering");
}

Shared_Ptr<Window> Engine::_create_window()
{
    Shared_Ptr<Window> window = Shared_Ptr<GLFW_Window>::create();
    window->initialize(_cvar_window_width->get(), _cvar_window_height->get(), _cvar_window_title->get().c_str());
 
    return window;
}

Shared_Ptr<Renderer_Backend> Engine::_create_renderer_backend(Renderer_API::Type api, const Shared_Ptr<Window>& window)
{
    Shared_Ptr<Renderer_Backend> backend;
    #ifdef CS_PLATFORM_WINDOWS
        backend = Shared_Ptr<DirectX_Renderer_Backend>::create();
    #else
        //TODO: Warn
        backend = Shared_Ptr<OpenGL_Renderer_Backend>::create();
    #endif //CS_PLATFORM_WINDOWS

    backend->initialize(window);

    _renderer->initialize(window, backend);

    return backend;
}
