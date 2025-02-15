#include "cs/engine/engine.hpp"
#include "cs/engine/cvar.hpp"
#include "cs/engine/net/net_instance.hpp"
#include "cs/engine/window/glfw/glfw_window.hpp"

#include "cs/engine/renderer/directx/directx_renderer.hpp"
#include "cs/engine/renderer/opengl/opengl_renderer.hpp"

#include "cs/engine/game/game_instance.hpp"
#include "cs/engine/vr/vr_system.hpp"

void Engine::initialize(const Dynamic_Array<std::string>& args)
{
    _initialize_cvars();
    _parse_args(args);

    _net_instance = Shared_Ptr<Net_Instance>::create((Net_Role::Type)_cvar_net_role->get());

    if (_net_instance->is_local())
    {
        if (_cvar_vr_support->get() && !_vr_system)
        {
            _vr_system = Shared_Ptr<VR_System>::create();
            _vr_system->initialize();
        }

        _renderer = Shared_Ptr<Renderer>::create();
        _create_renderer_backend((Renderer_API::Type)_cvar_renderer_api->get(), _create_window());

        Shared_Ptr<Perspective_Camera> default_camera = Shared_Ptr<Perspective_Camera>::create();
        default_camera->FOV_deg = 45.0f;
        default_camera->near_d = 0.0f;
        default_camera->far_d = 1000.0f;
        default_camera->position = {0.0f, 0.0f, -3.0f};
        default_camera->target = {0.0f, 1.0f, 0.0f};

        _renderer->set_active_camera(default_camera);

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

        if (_vr_system)
        {
            _vr_system->poll_events();
            _vr_system->update(dt);
        }

        if (_renderer)
        {
            _renderer->window->poll_events();
            
            _renderer->backend->begin_frame(VR_Eye::None);
            
            if (game_instance)
            {
                game_instance->render(_renderer, VR_Eye::None);
            }

            _renderer->backend->end_frame(VR_Eye::None);

            if (_vr_system)
            {
                _renderer->backend->begin_frame(VR_Eye::Left);
            
                if (game_instance)
                {
                    game_instance->render(_renderer, VR_Eye::Left);
                }
    
                _renderer->backend->end_frame(VR_Eye::Left);
                
                _renderer->backend->begin_frame(VR_Eye::Right);
            
                if (game_instance)
                {
                    game_instance->render(_renderer, VR_Eye::Right);
                }
    
                _renderer->backend->end_frame(VR_Eye::Right);
            }
            
            _renderer->render_frame();
            //_renderer->window->swap_buffers();
        }

        if (game_instance)
        {
            game_instance->update(dt);
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
        "cs_renderer_api", Renderer_API::OpenGL, "Which API are we using for rendering");
    _cvar_vr_support = _cvar_registry->register_cvar<bool>(
        "cs_vr_support", true, "Turn VR support on/off");
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
    switch(api)
    {
    #ifdef CS_PLATFORM_WINDOWS
        case Renderer_API::DirectX12:
        case Renderer_API::DirectX11:
        {
            backend = Shared_Ptr<DirectX_Renderer_Backend>::create();
            break;
        }
    #endif //CS_PLATFORM_WINDOWS
        default:
        {
            backend = Shared_Ptr<OpenGL_Renderer_Backend>::create();
        } 
    }

    _renderer->initialize(window, backend, _vr_system);

    return backend;
}
