#pragma once

#include "cs/engine/renderer/renderer.hpp"
#include "cs/engine/renderer/shader.hpp"
#include "cs/engine/renderer/mesh.hpp"

#include "GL/glew.h"

class OpenGL_Buffer : public Buffer
{
public:
    virtual ~OpenGL_Buffer() = default;
    virtual void bind() const override {}
    virtual void unbind() const override {}
    virtual void set_data(const void *data, uint32 size, uint32 offset = 0) override {}
};

class OpenGL_Uniform_Buffer : public OpenGL_Buffer
{
public:
    GLuint buffer;

public:
    virtual ~OpenGL_Uniform_Buffer() = default;
    virtual void bind() const override;
    virtual void unbind() const override;
    virtual void set_data(const void *data, uint32 size, uint32 offset = 0) override;
};

class OpenGL_Shader : public Shader
{
public:
    GLuint shader;

public:
    virtual ~OpenGL_Shader() override {}
    OpenGL_Shader() = default;

    virtual void bind() const override;
    virtual void unbind() const override;
};

struct OpenGL_Submesh
{
    Shared_Ptr<OpenGL_Shader> shader;
    int32 vertices_count{0};

    GLuint vertex_array;
    GLuint vertex_buffer;
};

class OpenGL_Mesh : public Mesh
{
public:
    Dynamic_Array<OpenGL_Submesh> submeshes;
    
public:
    OpenGL_Mesh() = default;
    OpenGL_Mesh(const Shared_Ptr<Mesh_Resource> &in_mesh_resource);

    virtual ~OpenGL_Mesh() override {}
    virtual void upload_data() override;
};

class Camera;
class OpenGL_Renderer_Backend : public Renderer_Backend
{
public:
    virtual ~OpenGL_Renderer_Backend() override {}
    virtual void initialize(const Shared_Ptr<Window> &window, const Shared_Ptr<VR_System>& vr_system) override;
    virtual void set_camera(const Shared_Ptr<Camera> &camera) override;

    virtual void render_frame() override;
    virtual void begin_frame(VR_Eye::Type eye = VR_Eye::None) override;
    virtual void end_frame(VR_Eye::Type eye = VR_Eye::None) override;
    virtual void shutdown() override;
    virtual void draw_mesh(const Shared_Ptr<Mesh>& mesh, const mat4& world_transform, VR_Eye::Type eye = VR_Eye::None) override;
    //virtual void draw_mesh(const Shared_Ptr<Mesh_Resource> &mesh) override;

    virtual Shared_Ptr<Buffer> create_vertex_buffer(void *data, uint32 size);
    virtual Shared_Ptr<Buffer> create_index_buffer(void *data, uint32 size);
    virtual Shared_Ptr<Buffer> create_uniform_buffer(void *data, uint32 size);

    virtual Shared_Ptr<Shader> create_shader(const Shared_Ptr<Shader_Resource> &shader_resource) override;
    virtual Shared_Ptr<Mesh> create_mesh(const Shared_Ptr<Mesh_Resource> &mesh) override;
    virtual Shared_Ptr<Material> create_material() override { return Shared_Ptr<Material>(); }

private:
    Shared_Ptr<Window> _window;

    Shared_Ptr<Camera> _camera;
    Shared_Ptr<OpenGL_Buffer> _uniform_buffer;

private:
    //bool _compile_shader(const char *filename, const char *entry_point, const char *profile, ComPtr<ID3DBlob> &shader_blob);
    //ComPtr<ID3D11PixelShader> _create_pixel_shader(const char *filename);
    //ComPtr<ID3D11VertexShader> _create_vertex_shader(const char *filename, ComPtr<ID3DBlob> &vertex_shader_blob);

    void _initialize_render_stuff();
    void _cleanup_render_stuff();
};
