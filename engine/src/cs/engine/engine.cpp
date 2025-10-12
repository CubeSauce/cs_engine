#include "cs/engine/engine.hpp"
#include "cs/engine/cvar.hpp"
#include "cs/engine/input.hpp"
#include "cs/engine/thread_pool.hpp"
#include "cs/engine/net/net_connection.hpp"
#include "cs/engine/window/glfw/glfw_window.hpp"

#include "cs/engine/renderer/directx/directx11_renderer.hpp"
#include "cs/engine/renderer/opengl/opengl_renderer.hpp"

#include "cs/engine/physics/physics_system.hpp"

#include "cs/engine/vr/vr_system.hpp"

template<> 
Engine* Singleton<Engine>::_singleton { nullptr };

void Engine::initialize(const Dynamic_Array<std::string>& args)
{
    _profiler = Shared_Ptr<Profiler>::create();

    PROFILE_FUNCTION()

    _initialize_cvars();
    _parse_args(args);

    _thread_pool = Shared_Ptr<Thread_Pool>::create(_cvar_num_threads->get());

    if (!_cvar_headless->get())
    {
        _input_system = Shared_Ptr<Input_System>::create();
        _vr_system = Shared_Ptr<VR_System>::create();
        if (_cvar_vr_support->get())
        {
            _vr_system->initialize();
        }

        _window = _create_window();

        _renderer = Shared_Ptr<Renderer>::create();
        _create_renderer_backend((Renderer_API::Type)_cvar_renderer_api->get(), _window);
    }

    _physics_system = Shared_Ptr<Physics_System>::create();
    _physics_system->initialize();

    _net_connection = Shared_Ptr<Net_Connection>::create((Net_Type::Type)_cvar_net_role->get());

    _initialize_defaults();
}

void Engine::shutdown()
{
    PROFILE_FUNCTION()

    _vr_system->shutdown();
}

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Duration = std::chrono::duration<float>;

void Engine::run(Entry_Point& entry_point)
{
    PROFILE_FUNCTION()

    _renderer->window->on_window_should_close.bind([&](){
        _should_close = true;
    });

    entry_point.initialize();

    const float dt_static = 1/60.0f;

    TimePoint previousTime = Clock::now();
    double accumulator = 0.0;

    while(!_should_close)
    {
        Scoped_Profiler frame_scope("frame");

        // Get current time and calculate elapsed time
        TimePoint currentTime = Clock::now();
        Duration frameTime = currentTime - previousTime;
        previousTime = currentTime;

        // Clamp `deltaTime` to avoid spiral of death if the game lags
        double dt = frameTime.count();
        if (dt > 0.25) dt = 0.25;

        _poll_inputs();

        entry_point.update(dt);

        // Accumulate time
        accumulator += dt;

        // @SYSTEM: UPDATE(dt)
        // Fixed time-step physics update
        while (accumulator >= dt_static)
        {
            Scoped_Profiler accumulator_scope("accumulator_frame");

            _net_connection->update(dt_static);

            if (_vr_system.is_valid())
            {
                _vr_system->update(dt_static);
            }

            _physics_system->update(dt_static);

            accumulator -= dt_static;
        }

        //TODO forward to rendering
        double alpha = accumulator / dt_static;
        if (_renderer)
        {
            Scoped_Profiler frame_scope("render");

            // Render normal view
            _renderer->backend->begin_frame();
            entry_point.render(VR_Eye::None);
            _renderer->backend->end_frame();

            if (_vr_system.is_valid())
            {
                _renderer->backend->begin_frame(VR_Eye::Left);
                entry_point.render(VR_Eye::Left);
                _renderer->backend->end_frame(VR_Eye::Left);
                
                _renderer->backend->begin_frame(VR_Eye::Right);
                entry_point.render(VR_Eye::Right);
                _renderer->backend->end_frame(VR_Eye::Right);
            }
            
            _renderer->render_frame();
        }

        _should_close = _should_close || entry_point.should_shutdown();
    }

    entry_point.shutdown();
}

Dynamic_Array<std::string> split(const std::string& str, char delim)
{
    Dynamic_Array<std::string> tokens;
    
    size_t last_token = 0;
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == delim)
        {
            tokens.push_back(str.substr(last_token, i));
            last_token = i + 1;
        }
    }

    tokens.push_back(str.substr(last_token, str.size()));

    return tokens;
}

void Engine::_parse_args(const Dynamic_Array<std::string>& args)
{
    for (const std::string& arg : args)
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

    _cvar_headless = _cvar_registry->register_cvar<bool>("cs_headless", false,
        "Whether the app instance is headless. (no window and rendering)");
    _cvar_num_threads = _cvar_registry->register_cvar<uint32>("cs_num_threads", 1,
        "Number of threads that the engine can use (capped by actual processor count)");
    _cvar_net_role = _cvar_registry->register_cvar<uint8>("cs_net_role", Net_Type::Offline,
        "Which net role will the instance be.");
    _cvar_window_width = _cvar_registry->register_cvar<uint32>("cs_window_width", 1280,
        "Width of the instance window");
    _cvar_window_height = _cvar_registry->register_cvar<uint32>("cs_window_height", 720,
        "Height of the instance window");
    _cvar_window_title = _cvar_registry->register_cvar<std::string>("cs_window_title",
        "CS Engine app", "Title of the instance window");
    _cvar_renderer_api = _cvar_registry->register_cvar<uint8>("cs_renderer_api",
        Renderer_API::DirectX11, "Which API are we using for rendering");
    _cvar_vr_support = _cvar_registry->register_cvar<bool>("cs_vr_support", true,
    "Turn VR support on/off");

    _cvar_exit = _cvar_registry->register_cvar<bool>("cs_exit", false,
    "Exit the application and shutdown the engine.");
    _cvar_fixed_timestep = _cvar_registry->register_cvar<float>("cs_fdt", 1.0f / 60.0f,
        "Fixed timestep");
}

void Engine::_poll_inputs()
{
    if (_vr_system.is_valid())
    {
        _vr_system->poll_events();
    }

    if (_renderer)
    {
        _renderer->window->poll_events();
    }

    Input_System::get().update();
}

Shared_Ptr<Window> Engine::_create_window()
{
    PROFILE_FUNCTION()

    Shared_Ptr<Window> window = Shared_Ptr<GLFW_Window>::create();
    window->initialize(_cvar_window_width->get(), _cvar_window_height->get(), _cvar_window_title->get().c_str());
 
    return window;
}

Shared_Ptr<Renderer_Backend> Engine::_create_renderer_backend(Renderer_API::Type api, const Shared_Ptr<Window>& window)
{
    PROFILE_FUNCTION()

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
    default_camera->FOV = 45_deg;
    default_camera->near_d = 0.0f;
    default_camera->far_d = 1000.0f;
    default_camera->position = {0.0f, -5.0f, 2.0f};
    default_camera->orientation = quat::from_direction(vec3::forward_vector);
    
    _renderer->set_active_camera(default_camera);

    return backend;
}

void Engine::_initialize_defaults()
{
    PROFILE_FUNCTION()
    
    default_texture_shader_resource = Shared_Ptr<Shader_Resource>::create();
    //TODO: fix for opengl
    default_texture_shader_resource->source_paths[Renderer_API::DirectX11].vertex_filepath = "assets/shaders/default_texture.hlsl.vert";
    default_texture_shader_resource->source_paths[Renderer_API::DirectX11].pixel_filepath = "assets/shaders/default_texture.hlsl.frag";
    default_texture_shader_resource->source_paths[Renderer_API::DirectX12].vertex_filepath = "assets/shaders/default_texture.hlsl.vert";
    default_texture_shader_resource->source_paths[Renderer_API::DirectX12].pixel_filepath = "assets/shaders/default_texture.hlsl.frag";
    default_texture_shader_resource->source_paths[Renderer_API::OpenGL].vertex_filepath = "assets/shaders/gen/glsl/default_texture_vert.glsl";
    default_texture_shader_resource->source_paths[Renderer_API::OpenGL].fragment_filepath = "assets/shaders/gen/glsl/default_texture_frag.glsl";

    //TODO: make this blend with white instead of multiple textures
    
    default_color_shader_resource = Shared_Ptr<Shader_Resource>::create();
    //TODO: fix for opengl
    default_color_shader_resource->source_paths[Renderer_API::DirectX11].vertex_filepath = "assets/shaders/default_color.hlsl.vert";
    default_color_shader_resource->source_paths[Renderer_API::DirectX11].pixel_filepath = "assets/shaders/default_color.hlsl.frag";
    default_color_shader_resource->source_paths[Renderer_API::DirectX12].vertex_filepath = "assets/shaders/default_color.hlsl.vert";
    default_color_shader_resource->source_paths[Renderer_API::DirectX12].pixel_filepath = "assets/shaders/default_color.hlsl.frag";
    default_color_shader_resource->source_paths[Renderer_API::OpenGL].vertex_filepath = "assets/shaders/gen/glsl/default_color_vert.glsl";
    default_color_shader_resource->source_paths[Renderer_API::OpenGL].fragment_filepath = "assets/shaders/gen/glsl/default_color_frag.glsl";

    default_texture_resource = Shared_Ptr<Texture_Resource>::create();
    default_texture_resource->initialize_from_file("assets/textures/default_gray.png");

}
