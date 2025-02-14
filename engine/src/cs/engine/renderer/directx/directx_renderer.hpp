#pragma once

#include "cs/engine/renderer/renderer.hpp"
#include "cs/engine/renderer/shader.hpp"
#include "cs/engine/renderer/mesh.hpp"

#ifdef CS_PLATFORM_WINDOWS
#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class DirectX_Buffer : public Buffer
{
public:
    ComPtr<ID3D11DeviceContext> device_context;
    ComPtr<ID3D11Buffer> buffer;

    virtual ~DirectX_Buffer() = default;
    virtual void bind() const override {}
    virtual void unbind() const override;
    virtual void set_data(const void *data, uint32 size, uint32 offset = 0) override {}
};

class DirectX_Uniform_Buffer : public DirectX_Buffer
{
public:
    virtual void bind() const override;
    virtual void set_data(const void *data, uint32 size, uint32 offset = 0) override;
};

class DirectX_Vertex_Buffer : public DirectX_Buffer
{
public:
    uint32 stride{sizeof(Vertex_Data)};
    uint32 offset{0};

public:
    virtual void bind() const override;
};

class DirectX_Index_Buffer : public DirectX_Buffer
{
public:
    DXGI_FORMAT format{DXGI_FORMAT_R32_UINT};
    uint32 offset{0};

public:
    virtual void bind() const override;
};

class DirectX_Shader : public Shader
{
public:
    ComPtr<ID3D11DeviceContext> device_context;
    ComPtr<ID3D11VertexShader> vertex_shader;
    ComPtr<ID3D11PixelShader> pixel_shader;
    ComPtr<ID3D11InputLayout> vertex_layout;

public:
    virtual ~DirectX_Shader() override {}
    DirectX_Shader() = default;

    virtual void bind() const override;
    virtual void unbind() const override;
};

struct DirectX_Submesh
{
    Shared_Ptr<DirectX_Shader> shader;
    ComPtr<ID3D11Buffer> vertex_buffer;
    int32 vertices_count{0};
};

class DirectX_Mesh : public Mesh
{
public:
    ComPtr<ID3D11Device> device;
    Dynamic_Array<DirectX_Submesh> submeshes;

public:
    DirectX_Mesh() = default;
    DirectX_Mesh(const Shared_Ptr<Mesh_Resource> &in_mesh_resource);

    virtual ~DirectX_Mesh() override {}
    virtual void upload_data() override;
};

class Camera;
class DirectX_Renderer_Backend : public Renderer_Backend
{
public:
    virtual ~DirectX_Renderer_Backend() override;
    virtual void initialize(const Shared_Ptr<Window> &window, const Shared_Ptr<VR_System>& vr_system) override;
    virtual void set_camera(const Shared_Ptr<Camera> &camera) override;

    virtual void begin_frame(VR_Eye::Type eye = VR_Eye::None) override;
    virtual void end_frame(VR_Eye::Type eye = VR_Eye::None) override;
    virtual void render_frame() override;
    virtual void shutdown() override;
    virtual void draw_mesh(const Shared_Ptr<Mesh> &mesh, const mat4 &world_transform, VR_Eye::Type eye = VR_Eye::None) override;
    // virtual void draw_mesh(const Shared_Ptr<Mesh_Resource> &mesh) override;

    virtual Shared_Ptr<Buffer> create_vertex_buffer(void *data, uint32 size);
    virtual Shared_Ptr<Buffer> create_index_buffer(void *data, uint32 size);
    virtual Shared_Ptr<Buffer> create_uniform_buffer(void *data, uint32 size);

    virtual Shared_Ptr<Shader> create_shader(const Shared_Ptr<Shader_Resource> &shader_resource) override;
    virtual Shared_Ptr<Mesh> create_mesh(const Shared_Ptr<Mesh_Resource> &mesh) override;
    virtual Shared_Ptr<Material> create_material() override { return Shared_Ptr<Material>(); }

private:
    Shared_Ptr<Window> _window;
    Shared_Ptr<VR_System> _vr_system;

    ComPtr<ID3D11Device> _device;
    ComPtr<ID3D11DeviceContext> _device_context;
    ComPtr<IDXGISwapChain> _swapchain;

    struct Direct_X_Framebuffer
    {
        ComPtr<ID3D11Texture2D> texture;
        ComPtr<ID3D11RenderTargetView> render_target_view;
        ComPtr<ID3D11DepthStencilView> depth_stencil_view;
    };

    Shared_Ptr<Direct_X_Framebuffer> _left_eye, _right_eye, _basic;

    D3D11_VIEWPORT _viewport{};

    Shared_Ptr<Camera> _camera;
    Shared_Ptr<DirectX_Buffer> _uniform_buffer;

private:
    HWND _hwnd;

    bool _compile_shader(const char *filename, const char *entry_point, const char *profile, ComPtr<ID3DBlob> &shader_blob);
    ComPtr<ID3D11VertexShader> _create_vertex_shader(const char *filename, ComPtr<ID3DBlob> &vertex_shader_blob);
    ComPtr<ID3D11PixelShader> _create_pixel_shader(const char *filename);

    void _initialize_framebuffer(Direct_X_Framebuffer &framebuffer);
    void _initialize_render_stuff();
    void _cleanup_render_stuff();
};

#endif // CS_PLATFORM_WINDOWS
