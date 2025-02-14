#include "cs/engine/renderer/renderer.hpp"
#include "cs/engine/window.hpp"
#include "cs/engine/renderer/camera.hpp"

Renderer::Renderer()
{
}

void Renderer::initialize(const Shared_Ptr<Window>& in_window, const Shared_Ptr<Renderer_Backend>& in_backend, const Shared_Ptr<VR_System>& in_vr_system)
{
    window = in_window;
    backend = in_backend;
    backend->initialize(window, in_vr_system);
}

void Renderer::begin_frame()
{
    backend->begin_frame();
}

void Renderer::end_frame()
{
    backend->end_frame();
}

void Renderer::render_frame()
{
    backend->render_frame();
}

void Renderer::shutdown()
{
    backend->shutdown();
}

void Renderer::set_active_camera(const Shared_Ptr<Camera>& camera)
{
    _active_camera = camera;

    backend->set_camera(camera);
}

void Renderer::draw_mesh(const Shared_Ptr<Mesh_Resource>& mesh)
{
    //backend->draw_mesh(mesh);
}

