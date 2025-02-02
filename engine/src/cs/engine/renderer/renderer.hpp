#pragma once

#include "cs/cs.hpp"
#include "cs/memory/shared_ptr.hpp"
#include "cs/engine/renderer/mesh.hpp"
#include "cs/engine/renderer/camera.hpp"
#include "cs/engine/renderer/shader.hpp"
#include "cs/engine/renderer/material.hpp"

class Buffer
{
public:
    virtual ~Buffer() = default;
    virtual void bind() const = 0;
    virtual void unbind() const = 0;
    virtual void set_data(const void* data, uint32 size, uint32 offset = 0) = 0;
};

class Window;
class Renderer_Backend
{
public:
    virtual ~Renderer_Backend() = default;
    virtual void initialize(const Shared_Ptr<Window>& window) = 0;
    virtual void set_camera(const Shared_Ptr<Camera>& camera) = 0;
    
    virtual void begin_frame() = 0;
    virtual void end_frame() = 0;
    virtual void render_frame() = 0;
    virtual void shutdown() = 0;
    virtual void draw_mesh(const Shared_Ptr<Mesh>& mesh, const mat4& world_transform) = 0;
    //virtual void draw_mesh(const Shared_Ptr<Mesh_Resource>& mesh) = 0;

    virtual Shared_Ptr<Shader> create_shader(const Shared_Ptr<Shader_Resource>& shader_resource) = 0;
    virtual Shared_Ptr<Mesh> create_mesh(const Shared_Ptr<Mesh_Resource>& mesh) = 0;
    virtual Shared_Ptr<Material> create_material() = 0;
};

class Renderer
{
public:
    Shared_Ptr<Window> window;
    Shared_Ptr<Renderer_Backend> backend;

public:
    Renderer();
    
    void initialize(const Shared_Ptr<Window>& window, const Shared_Ptr<Renderer_Backend>& backend);

    void begin_frame();
    void end_frame();
    void render_frame();
    void shutdown();

    void set_active_camera(const Shared_Ptr<Camera>& camera);

    void draw_mesh(const Shared_Ptr<Mesh_Resource>& mesh);

private:
    Shared_Ptr<Camera> _active_camera;
};
