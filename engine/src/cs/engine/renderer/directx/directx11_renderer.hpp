#pragma once

#include "cs/engine/renderer/renderer.hpp"
#include "cs/engine/renderer/shader.hpp"
#include "cs/engine/renderer/mesh.hpp"

#ifdef CS_PLATFORM_WINDOWS
#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class DirectX11_Buffer : public Buffer
{
public:
    ComPtr<ID3D11DeviceContext> device_context;
    ComPtr<ID3D11Buffer> buffer;

    virtual ~DirectX11_Buffer() = default;
    virtual void bind() const override {}
    virtual void unbind() const override;
    virtual void set_data(const void *data, uint32 size, uint32 offset = 0) override {}
};

class DirectX11_Uniform_Buffer : public DirectX11_Buffer
{
public:
    virtual void bind() const override;
    virtual void set_data(const void *data, uint32 size, uint32 offset = 0) override;
};

class DirectX11_Vertex_Buffer : public DirectX11_Buffer
{
public:
    uint32 stride{sizeof(Vertex_Data)};
    uint32 offset{0};

public:
    virtual void bind() const override;
};

class DirectX11_Index_Buffer : public DirectX11_Buffer
{
public:
    DXGI_FORMAT format{DXGI_FORMAT_R32_UINT};
    uint32 offset{0};

public:
    virtual void bind() const override;
};

class DirectX11_Texture : public Texture
{
public:
    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11ShaderResourceView> resource_view;
};

class DirectX11_Shader : public Shader
{
public:
    ComPtr<ID3D11DeviceContext> device_context;
    ComPtr<ID3D11VertexShader> vertex_shader;
    ComPtr<ID3D11PixelShader> pixel_shader;
    ComPtr<ID3D11InputLayout> vertex_layout; 
    ComPtr<ID3D11SamplerState> sampler_state_0;

public:
    virtual ~DirectX11_Shader() override {}
    DirectX11_Shader() = default;

    virtual void bind() const override;
    virtual void unbind() const override;
};

struct DirectX11_Material
{
    Shared_Ptr<DirectX11_Shader> shader;
    Shared_Ptr<DirectX11_Texture> texture;
};

struct DirectX11_Submesh
{
    DirectX11_Material material;
    ComPtr<ID3D11Buffer> vertex_buffer;
    int32 vertices_count{0};
};

class DirectX11_Mesh : public Mesh
{
public:
    ComPtr<ID3D11Device> device;
    Dynamic_Array<DirectX11_Submesh> submeshes;

public:
    DirectX11_Mesh() = default;
    DirectX11_Mesh(const Shared_Ptr<Mesh_Resource> &in_mesh_resource);

    virtual ~DirectX11_Mesh() override {}
    virtual void upload_data() override;
};

class Camera;
class DirectX11_Renderer_Backend : public Renderer_Backend
{
public:
    virtual ~DirectX11_Renderer_Backend() override;
    virtual void initialize(const Shared_Ptr<Window> &window) override;
    virtual void set_camera(const Shared_Ptr<Camera> &camera) override;

    virtual void begin_frame(VR_Eye::Type eye = VR_Eye::None) override;
    virtual void end_frame(VR_Eye::Type eye = VR_Eye::None) override;
    virtual void render_frame() override;
    virtual void shutdown() override;
    virtual void draw_mesh(const Shared_Ptr<Mesh> &mesh, const mat4 &world_transform, VR_Eye::Type eye = VR_Eye::None) override;
    // virtual void draw_mesh(const Shared_Ptr<Mesh_Resource> &mesh_resource) override;

    virtual Shared_Ptr<Buffer> create_vertex_buffer(void *data, uint32 size);
    virtual Shared_Ptr<Buffer> create_index_buffer(void *data, uint32 size);
    virtual Shared_Ptr<Buffer> create_uniform_buffer(void *data, uint32 size);

    virtual Shared_Ptr<Shader> create_shader(const Shared_Ptr<Shader_Resource> &shader_resource) override;
    virtual Shared_Ptr<Mesh> create_mesh(const Shared_Ptr<Mesh_Resource> &mesh_resource) override;
    virtual Shared_Ptr<Material> create_material() override { return Shared_Ptr<Material>(); }
    virtual Shared_Ptr<Texture> create_texture(const Shared_Ptr<Texture_Resource>& texture_resource) override;

private:
    Shared_Ptr<Window> _window;

    ComPtr<ID3D11Device> _device;
    ComPtr<ID3D11DeviceContext> _device_context;
    ComPtr<IDXGISwapChain> _swapchain;

    struct DirectX11_Framebuffer
    {
        ComPtr<ID3D11Texture2D> texture;
        ComPtr<ID3D11RenderTargetView> render_target_view;
        ComPtr<ID3D11DepthStencilView> depth_stencil_view;
    };

    DirectX11_Framebuffer _framebuffers[3];

    D3D11_VIEWPORT _viewport{};

    Shared_Ptr<Camera> _camera;
    Shared_Ptr<DirectX11_Buffer> _uniform_buffer;

private:
    HWND _hwnd;

    bool _compile_shader(const char *filename, const char *entry_point, const char *profile, ComPtr<ID3DBlob> &shader_blob);
    ComPtr<ID3D11VertexShader> _create_vertex_shader(const char *filename, ComPtr<ID3DBlob> &vertex_shader_blob);
    ComPtr<ID3D11PixelShader> _create_pixel_shader(const char *filename);

    DirectX11_Material _create_material(const Shared_Ptr<Material_Resource>& material_resource);
    Shared_Ptr<DirectX11_Shader> _create_shader(const Shared_Ptr<Shader_Resource> &shader_resource);
    Shared_Ptr<DirectX11_Texture> _create_texture(const Shared_Ptr<Texture_Resource>& texture_resource);

    DirectX11_Framebuffer _create_framebuffer();
    void _destroy_framebuffer(DirectX11_Framebuffer& framebuffer);
    void _initialize_render_stuff();
    void _cleanup_render_stuff();
};

#endif // CS_PLATFORM_WINDOWS
