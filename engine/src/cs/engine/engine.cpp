#include "cs/engine/engine.hpp"
#include "cs/engine/cvar.hpp"
#include "cs/engine/input.hpp"
#include "cs/engine/net/net_instance.hpp"
#include "cs/engine/window/glfw/glfw_window.hpp"

#include "cs/engine/renderer/directx/directx11_renderer.hpp"
#include "cs/engine/renderer/opengl/opengl_renderer.hpp"

#include "cs/engine/game/game_instance.hpp"
#include "cs/engine/vr/vr_system.hpp"

void Engine::initialize(const Dynamic_Array<std::string>& args)
{
    _initialize_cvars();
    _parse_args(args);

    _input_system = Shared_Ptr<Input_System>::create();

    _vr_system = Shared_Ptr<VR_System>::create();
    if (_cvar_vr_support->get())
    {
        _vr_system->initialize();
    }

    _window = _create_window();

    _renderer = Shared_Ptr<Renderer>::create();
    _create_renderer_backend((Renderer_API::Type)_cvar_renderer_api->get(), _window);

    _net_instance = Shared_Ptr<Net_Instance>::create((Net_Role::Type)_cvar_net_role->get());

    _initialize_defaults();
}

void Engine::shutdown()
{
    _vr_system->shutdown();
}

void Engine::run()
{
    VR_System& vr_system = VR_System::get();

    _renderer->window->on_window_should_close.bind([&](){
        _should_close = true;
    });

    if (game_instance)
    {
        game_instance->init();
    }

    const float dt = 1/60.0f;
    while(!_should_close)
    {
        _net_instance->update(dt);

        if (vr_system.is_valid())
        {
            vr_system.poll_events();
            vr_system.update(dt);
        }

        if (game_instance)
        {
            game_instance->update(dt);
        }

        if (_renderer)
        {
            _renderer->window->poll_events();
            
            // Render normal view
            _renderer->backend->begin_frame(VR_Eye::None);
            if (game_instance)
            {
                game_instance->render(_renderer, VR_Eye::None);
            }
            _renderer->backend->end_frame(VR_Eye::None);

            if (vr_system.is_valid())
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
            backend = Shared_Ptr<DirectX11_Renderer_Backend>::create();
            break;
        }
    #endif //CS_PLATFORM_WINDOWS
        default:
        {
            backend = Shared_Ptr<OpenGL_Renderer_Backend>::create();
        } 
    }

    _renderer->initialize(window, backend);

    Shared_Ptr<Perspective_Camera> default_camera = Shared_Ptr<Perspective_Camera>::create();
    default_camera->FOV_deg = 45.0f;
    default_camera->near_d = 0.0f;
    default_camera->far_d = 1000.0f;
    default_camera->position = {0.0f, -5.0f, 2.0f};
    default_camera->target = {0.0f, 0.0f, 1.0f};

    _renderer->set_active_camera(default_camera);

    return backend;
}

void Engine::_initialize_defaults()
{
    default_shader_texture = Shared_Ptr<Shader_Resource>::create();
    //TODO: fix for opengl
    default_shader_texture->source_paths[Renderer_API::DirectX11].vertex_filepath = "assets/shaders/default_texture.hlsl.vert";
    default_shader_texture->source_paths[Renderer_API::DirectX11].pixel_filepath = "assets/shaders/default_texture.hlsl.frag";
    default_shader_texture->source_paths[Renderer_API::DirectX12].vertex_filepath = "assets/shaders/default_texture.hlsl.vert";
    default_shader_texture->source_paths[Renderer_API::DirectX12].pixel_filepath = "assets/shaders/default_texture.hlsl.frag";

    //TODO: make this blend with white instead of multiple textures
    
    default_shader_color = Shared_Ptr<Shader_Resource>::create();
    //TODO: fix for opengl
    default_shader_color->source_paths[Renderer_API::DirectX11].vertex_filepath = "assets/shaders/default_color.hlsl.vert";
    default_shader_color->source_paths[Renderer_API::DirectX11].pixel_filepath = "assets/shaders/default_color.hlsl.frag";
    default_shader_color->source_paths[Renderer_API::DirectX12].vertex_filepath = "assets/shaders/default_color.hlsl.vert";
    default_shader_color->source_paths[Renderer_API::DirectX12].pixel_filepath = "assets/shaders/default_color.hlsl.frag";

}