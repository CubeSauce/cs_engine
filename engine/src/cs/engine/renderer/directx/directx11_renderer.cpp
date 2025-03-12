#include "cs/engine/renderer/directx/directx11_renderer.hpp"
#include "cs/engine/renderer/camera.hpp"
#include "cs/engine/renderer/mesh.hpp"
#include "cs/engine/vr/vr_system.hpp"
#include "cs/memory/shared_ptr.hpp"
#include "cs/engine/window.hpp"
#include "cs/engine/profiling/profiler.hpp"

#include <GL/glew.h>

#include <memory>

#include "stb/stb_image.h"

#ifdef CS_PLATFORM_WINDOWS
#include <d3d11.h>       // D3D interface
#include <dxgi.h>        // DirectX11 driver interface
#include <d3dcompiler.h> // shader compiler
#include <DirectXMath.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")

void DirectX11_Buffer::unbind() const
{
    device_context->PSSetConstantBuffers(0, 1, nullptr);
    device_context->VSSetConstantBuffers(0, 1, nullptr);
}

void DirectX11_Uniform_Buffer::bind() const
{
    device_context->PSSetConstantBuffers(0, 1, buffer.GetAddressOf());
    device_context->VSSetConstantBuffers(0, 1, buffer.GetAddressOf());
}

void DirectX11_Uniform_Buffer::set_data(const void *data, uint32 size, uint32 offset)
{
    D3D11_MAPPED_SUBRESOURCE mapped_constant_buffer;
    HRESULT hr = device_context->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_constant_buffer);
    assert(SUCCEEDED(hr));
    CopyMemory(mapped_constant_buffer.pData, data, size);
    device_context->Unmap(buffer.Get(), 0);
}

void DirectX11_Vertex_Buffer::bind() const
{
    // TODO: Get topology from class
    device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_context->IASetVertexBuffers(0, 1, buffer.GetAddressOf(), &stride, &offset);
}

void DirectX11_Index_Buffer::bind() const
{
    device_context->IASetIndexBuffer(buffer.Get(), DXGI_FORMAT_R32_UINT, offset);
}

void DirectX11_Shader::bind() const
{
    device_context->IASetInputLayout(vertex_layout.Get());
    device_context->VSSetShader(vertex_shader.Get(), nullptr, 0);
    device_context->PSSetShader(pixel_shader.Get(), nullptr, 0);

    device_context->PSSetSamplers(0, 1, sampler_state_0.GetAddressOf());
}

void DirectX11_Shader::unbind() const
{
    device_context->IASetInputLayout(nullptr);
    device_context->VSSetShader(nullptr, nullptr, 0);
    device_context->PSSetShader(nullptr, nullptr, 0);
}

DirectX11_Mesh::DirectX11_Mesh(const Shared_Ptr<Mesh_Resource> &in_mesh_resource)
    : Mesh(in_mesh_resource)
{
}

void DirectX11_Mesh::upload_data()
{
    for (int32 s = 0; s < submeshes.size(); ++s)
    {
        DirectX11_Submesh &submesh = submeshes[s];
        const Submesh_Data &submesh_data = mesh_resource->submeshes[s];

        submesh.vertices_count = submesh_data.vertices.size();

        D3D11_BUFFER_DESC buffer_desc = {};

        buffer_desc.ByteWidth = submesh_data.vertices.size_in_bytes();
        buffer_desc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
        buffer_desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA resource_data = {};
        resource_data.pSysMem = submesh_data.vertices.begin();
        assert(SUCCEEDED(device->CreateBuffer(&buffer_desc, &resource_data, &submesh.vertex_buffer)));
    }
}

DirectX11_Renderer_Backend::~DirectX11_Renderer_Backend()
{
}

struct
{
    mat4 world{mat4(1.0f)};
    mat4 world_inv_tran{mat4(1.0f)};
    mat4 view{mat4(1.0f)};
    mat4 projection{mat4(1.0f)};
} data;

void DirectX11_Renderer_Backend::initialize(const Shared_Ptr<Window> &window)
{
    PROFILE_FUNCTION()

    _window = window;
    _hwnd = static_cast<HWND>(window->native_handle());

    // Create the Direct3D device and swap chain
    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
    swap_chain_desc.BufferCount = 1;
    swap_chain_desc.BufferDesc.Width = (uint32)_viewport.Width;
    swap_chain_desc.BufferDesc.Height = (uint32)_viewport.Height;
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.OutputWindow = _hwnd;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.Windowed = TRUE;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                               nullptr, 0, D3D11_SDK_VERSION, &swap_chain_desc, &_swapchain, &_device, nullptr, &_device_context);
    assert(SUCCEEDED(hr));

    _window->on_window_resize.bind([&](uint32 width, uint32 height)
    {
        _cleanup_render_stuff();
        _initialize_render_stuff(); 
    });

    _uniform_buffer = create_uniform_buffer(&data, sizeof(data));

    _initialize_render_stuff();
}

void DirectX11_Renderer_Backend::set_camera(const Shared_Ptr<Camera> &camera)
{
    _camera = camera;
}

void DirectX11_Renderer_Backend::begin_frame(VR_Eye::Type eye)
{
    PROFILE_FUNCTION()

    constexpr float clearColor[] = {0.1f, 0.3f, 0.1f, 1.0f};
    _device_context->RSSetViewports(1, &_viewport);

    // TODO: Some error checking
    _device_context->ClearRenderTargetView(_framebuffers[eye].render_target_view.Get(), clearColor);
    _device_context->ClearDepthStencilView(_framebuffers[eye].depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    _device_context->OMSetRenderTargets(1, _framebuffers[eye].render_target_view.GetAddressOf(), _framebuffers[eye].depth_stencil_view.Get());
}

void DirectX11_Renderer_Backend::end_frame(VR_Eye::Type eye)
{
}

void DirectX11_Renderer_Backend::render_frame()
{
    PROFILE_FUNCTION()

    VR_System& vr_system = VR_System::get();
    if (vr_system.is_valid())
    {
#ifdef CS_WITH_VR_SUPPORT
        vr::VRTextureBounds_t bounds;
        bounds.uMin = 0.0f;
        bounds.uMax = 1.0f;
        bounds.vMin = 0.0f;
        bounds.vMax = 1.0f;
    
        vr::Texture_t leftEyeTexture = { ( void * ) _framebuffers[VR_Eye::Left].texture.Get(), vr::TextureType_DirectX, vr::ColorSpace_Gamma };
        vr::VRCompositor()->Submit( vr::Eye_Left, &leftEyeTexture, &bounds, vr::Submit_Default );
    
        vr::Texture_t rightEyeTexture = { ( void * ) _framebuffers[VR_Eye::Right].texture.Get(), vr::TextureType_DirectX, vr::ColorSpace_Gamma };
        vr::VRCompositor()->Submit( vr::Eye_Right, &rightEyeTexture, &bounds, vr::Submit_Default );
#endif //CS_WITH_VR_SUPPORT
    }

    _swapchain->Present(1, 0);
}

void DirectX11_Renderer_Backend::shutdown()
{
    PROFILE_FUNCTION()

    if (_swapchain)
        _swapchain->Release();
    if (_device)
        _device->Release();
    if (_device_context)
        _device_context->Release();
}

UINT vertex_stride = sizeof(Vertex_Data);
UINT vertex_offset = 0;
void DirectX11_Renderer_Backend::draw_mesh(const Shared_Ptr<Mesh> &mesh, const mat4 &world_transform, VR_Eye::Type eye)
{
    PROFILE_FUNCTION()
    
    Shared_Ptr<DirectX11_Mesh> dx_mesh = mesh;
    if (!dx_mesh)
    {
        return;
    }

    data = {};

    VR_System& vr_system = VR_System::get();
    if (vr_system.is_valid())
    {
        Shared_Ptr<Camera> camera = vr_system.get_camera(eye);
        
        data.view = camera->get_view();
        data.projection = camera->get_projection();
    }
    else if (_camera)
    {
        _camera->aspect_ratio = _viewport.Width / _viewport.Height;
        _camera->calculate_projection();
        _camera->calculate_view();

        data.view = _camera->get_view();
        data.projection = _camera->get_projection();
    }

    data.world = world_transform;

    data.world_inv_tran = data.world.inverse();
    data.world.transpose();
    data.view.transpose();
    data.projection.transpose();

    for (const DirectX11_Submesh &submesh : dx_mesh->submeshes)
    {
        // TODO: Change only when needed
        _uniform_buffer->set_data(&data, sizeof(data), 0);
        _uniform_buffer->bind();

        _device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        _device_context->IASetVertexBuffers(0, 1, submesh.vertex_buffer.GetAddressOf(), &vertex_stride, &vertex_offset);

        if (Shared_Ptr<DirectX11_Texture> dx11_texture = submesh.material.texture)
        {
            _device_context->PSSetShaderResources(0, 1, dx11_texture->resource_view.GetAddressOf());
        }

        submesh.material.shader->bind();
        _device_context->Draw(submesh.vertices_count, 0);
        submesh.material.shader->unbind();
    }
}

Shared_Ptr<Buffer> DirectX11_Renderer_Backend::create_vertex_buffer(void *data, uint32 size)
{
    // TODO: Won't work
    Shared_Ptr<DirectX11_Buffer> buffer = Shared_Ptr<DirectX11_Buffer>::create();
    buffer->device_context = _device_context;

    D3D11_BUFFER_DESC buffer_desc{0};
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // | D3D11_CPU_ACCESS_READ;
    buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
    buffer_desc.ByteWidth = (uint32)size;

    D3D11_SUBRESOURCE_DATA subresource_data{0};
    subresource_data.pSysMem = data;

    HRESULT hr = _device->CreateBuffer(&buffer_desc, &subresource_data, &buffer->buffer);
    assert(SUCCEEDED(hr));

    return buffer;
}

Shared_Ptr<Buffer> DirectX11_Renderer_Backend::create_index_buffer(void *data, uint32 size)
{
    Shared_Ptr<DirectX11_Index_Buffer> buffer = Shared_Ptr<DirectX11_Index_Buffer>::create();
    D3D11_BUFFER_DESC buffer_desc{0};
    buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // | D3D11_CPU_ACCESS_READ;
    buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
    buffer_desc.ByteWidth = size;

    D3D11_SUBRESOURCE_DATA subresource_data{0};
    subresource_data.pSysMem = data;

    HRESULT hr = _device->CreateBuffer(&buffer_desc, &subresource_data, &buffer->buffer);
    assert(SUCCEEDED(hr));

    return buffer;
}

Shared_Ptr<Buffer> DirectX11_Renderer_Backend::create_uniform_buffer(void *data, uint32 size)
{
    Shared_Ptr<DirectX11_Uniform_Buffer> buffer = Shared_Ptr<DirectX11_Uniform_Buffer>::create();
    buffer->device_context = _device_context;

    D3D11_BUFFER_DESC buffer_desc = {0};
    buffer_desc.ByteWidth = size;
    buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    buffer_desc.MiscFlags = 0;
    buffer_desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA resource_data = {0};
    resource_data.pSysMem = data;

    HRESULT hr = _device->CreateBuffer(&buffer_desc, &resource_data, &buffer->buffer);
    assert(SUCCEEDED(hr));

    buffer->bind();

    return buffer;
}

Shared_Ptr<Shader> DirectX11_Renderer_Backend::create_shader(const Shared_Ptr<Shader_Resource> &shader_resource)
{
    return _create_shader(shader_resource);
}

Shared_Ptr<Mesh> DirectX11_Renderer_Backend::create_mesh(const Shared_Ptr<Mesh_Resource> &mesh_resource)
{
    Shared_Ptr<DirectX11_Mesh> dx_mesh = Shared_Ptr<DirectX11_Mesh>::create(mesh_resource);
    dx_mesh->device = _device;

    for (const Submesh_Data &submesh : mesh_resource->submeshes)
    {
        DirectX11_Submesh dx_submesh;

        // TODO: map with materials and existing shaders so we don't duplicate
        dx_submesh.material = _create_material(submesh.material_resource);
        dx_submesh.vertices_count = submesh.vertices.size();

        dx_mesh->submeshes.add(dx_submesh);
    }

    return dx_mesh;
}

Shared_Ptr<Mesh> DirectX11_Renderer_Backend::get_mesh(const Shared_Ptr<Mesh_Resource> &mesh_resource)
{
    Shared_Ptr<DirectX11_Mesh>& dx_mesh = _meshes[mesh_resource->name];

    if (!dx_mesh)
    {
        dx_mesh = create_mesh(mesh_resource);
        dx_mesh->upload_data();
    }

    return dx_mesh;
}

// For now use same format for all
Shared_Ptr<Texture> DirectX11_Renderer_Backend::create_texture(const Shared_Ptr<Texture_Resource>& texture_resource)
{
    return _create_texture(texture_resource);
}

#include <iostream>
bool DirectX11_Renderer_Backend::_compile_shader(const char *filename, const char *entry_point, const char *profile, ComPtr<ID3DBlob> &shader_blob)
{
    constexpr UINT compile_flags = D3DCOMPILE_ENABLE_STRICTNESS;

    ComPtr<ID3DBlob> temp_shader_blob = nullptr;
    ComPtr<ID3DBlob> error_blob = nullptr;

    size_t num_chars;
    std::wstring w_filename(strlen(filename), L'#');
    mbstowcs_s(&num_chars, &w_filename[0], w_filename.capacity(), filename, strlen(filename));

    HRESULT hr = D3DCompileFromFile(w_filename.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry_point, profile,
                                    compile_flags, 0, &temp_shader_blob, &error_blob);

    if (FAILED(hr))
    {
        std::string e((char *)error_blob->GetBufferPointer());
        printf("%s", e.c_str());
        flushall();
        assert(false);
    }

    shader_blob = std::move(temp_shader_blob);
    return true;
}

ComPtr<ID3D11VertexShader> DirectX11_Renderer_Backend::_create_vertex_shader(const char *filename, ComPtr<ID3DBlob> &vertex_shader_blob)
{
    if (!_compile_shader(filename, "main", "vs_5_0", vertex_shader_blob))
    {
        return nullptr;
    }

    ComPtr<ID3D11VertexShader> vertex_shader;
    assert(SUCCEEDED(_device->CreateVertexShader(
        vertex_shader_blob->GetBufferPointer(),
        vertex_shader_blob->GetBufferSize(),
        nullptr, &vertex_shader)));

    return vertex_shader;
}

ComPtr<ID3D11PixelShader> DirectX11_Renderer_Backend::_create_pixel_shader(const char *filename)
{
    ComPtr<ID3DBlob> pixel_shader_blob = nullptr;
    if (!_compile_shader(filename, "main", "ps_5_0", pixel_shader_blob))
    {
        return nullptr;
    }

    ComPtr<ID3D11PixelShader> pixel_shader;
    assert(SUCCEEDED(_device->CreatePixelShader(
        pixel_shader_blob->GetBufferPointer(),
        pixel_shader_blob->GetBufferSize(),
        nullptr, &pixel_shader)));

    return pixel_shader;
}

DirectX11_Material DirectX11_Renderer_Backend::_create_material(const Shared_Ptr<Material_Resource>& material_resource)
{    
    PROFILE_FUNCTION()

    DirectX11_Material dx11_material;

    dx11_material.shader = _create_shader(material_resource->shader_resource);
    dx11_material.texture = _create_texture(material_resource->texture_resource);

    return dx11_material;
}

Shared_Ptr<DirectX11_Shader> DirectX11_Renderer_Backend::_create_shader(const Shared_Ptr<Shader_Resource> &shader_resource)
{
    PROFILE_FUNCTION()

    Shared_Ptr<DirectX11_Shader> shader = Shared_Ptr<DirectX11_Shader>::create();
    shader->shader_resource = shader_resource;
    
    shader->device_context = _device_context;

    ComPtr<ID3DBlob> vertex_shader_blob = nullptr;
    shader->vertex_shader = _create_vertex_shader(shader_resource->source_paths[Renderer_API::DirectX11].vertex_filepath, vertex_shader_blob);
    shader->pixel_shader = _create_pixel_shader(shader_resource->source_paths[Renderer_API::DirectX11].fragment_filepath);

    // TODO: From Shader definition
    constexpr D3D11_INPUT_ELEMENT_DESC input_element_desc[] = 
    {
        {"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    assert(SUCCEEDED(_device->CreateInputLayout(input_element_desc, ARRAYSIZE(input_element_desc),
                                                vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), &shader->vertex_layout)));

    D3D11_SAMPLER_DESC sampler_desc = {};
    sampler_desc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    sampler_desc.Filter = D3D11_FILTER::D3D11_FILTER_ANISOTROPIC;
    sampler_desc.MaxAnisotropy = 16;
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
    assert(SUCCEEDED(_device->CreateSamplerState(&sampler_desc, &shader->sampler_state_0)));

    return shader;
}

Shared_Ptr<DirectX11_Texture> DirectX11_Renderer_Backend::_create_texture(const Shared_Ptr<Texture_Resource>& texture_resource)
{
    PROFILE_FUNCTION()

    if (!texture_resource)
    {
        return Shared_Ptr<DirectX11_Texture>();
    }

    Shared_Ptr<DirectX11_Texture> dx11_texture = Shared_Ptr<DirectX11_Texture>::create();

    D3D11_TEXTURE2D_DESC textureDesc = {};
    D3D11_SUBRESOURCE_DATA initialData = {};

    DXGI_FORMAT textureFormat;
    switch (texture_resource->num_channels)
    {
    case 1:
        textureFormat = DXGI_FORMAT::DXGI_FORMAT_R8_UNORM;
        break;
    case 2:
        textureFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8_UNORM;
        break;
    case 3:
        {
            //TODO: Convert image
            assert(false);
        }
        break;
    case 4:
        textureFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
        break;
    default:
        {
            return Shared_Ptr<DirectX11_Texture>();
        }
        break;
    }
    textureDesc.Format = textureFormat;
    textureDesc.ArraySize = 1;
    textureDesc.MipLevels = 1;
    textureDesc.Width = texture_resource->width;
    textureDesc.Height = texture_resource->height;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    //populate initial data
    initialData.pSysMem = texture_resource->data;
    initialData.SysMemPitch = texture_resource->num_channels * texture_resource->width;

    if (FAILED(_device->CreateTexture2D(&textureDesc, &initialData, dx11_texture->texture.GetAddressOf())))
    {
        return Shared_Ptr<DirectX11_Texture>();
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC resource_view_desc = {};
    resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    resource_view_desc.Format = textureDesc.Format;
    resource_view_desc.Texture2D.MipLevels = textureDesc.MipLevels;

    if (FAILED(_device->CreateShaderResourceView(dx11_texture->texture.Get(), &resource_view_desc, &dx11_texture->resource_view)))
    {
        return Shared_Ptr<DirectX11_Texture>();
    }

    return dx11_texture;
}

DirectX11_Renderer_Backend::DirectX11_Framebuffer DirectX11_Renderer_Backend::_create_framebuffer()
{
    PROFILE_FUNCTION()

    DirectX11_Framebuffer framebuffer;

    HRESULT hr = _swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)framebuffer.texture.GetAddressOf());
    assert(SUCCEEDED(hr));

    hr = _device->CreateRenderTargetView(framebuffer.texture.Get(), 0, &framebuffer.render_target_view);
    assert(SUCCEEDED(hr));

    // TODO: Update on resize
    D3D11_TEXTURE2D_DESC depth_texture_desc;
    ZeroMemory(&depth_texture_desc, sizeof(depth_texture_desc));
    depth_texture_desc.Width = (uint32)_viewport.Width;
    depth_texture_desc.Height = (uint32)_viewport.Height;
    depth_texture_desc.MipLevels = 1;
    depth_texture_desc.ArraySize = 1;
    depth_texture_desc.SampleDesc.Count = 1;
    depth_texture_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D *depth_stencil_texture;
    hr = _device->CreateTexture2D(&depth_texture_desc, NULL, &depth_stencil_texture);

    assert(SUCCEEDED(hr));

    D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc;
    ZeroMemory(&depth_stencil_view_desc, sizeof(depth_stencil_view_desc));
    depth_stencil_view_desc.Format = depth_texture_desc.Format;
    depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

    hr = _device->CreateDepthStencilView(depth_stencil_texture, &depth_stencil_view_desc, &framebuffer.depth_stencil_view);
    assert(SUCCEEDED(hr));
    depth_stencil_texture->Release();

    return framebuffer;
}

void DirectX11_Renderer_Backend::_destroy_framebuffer(DirectX11_Framebuffer& framebuffer)
{
    framebuffer.depth_stencil_view.Reset();
    framebuffer.render_target_view.Reset();
}

void DirectX11_Renderer_Backend::_initialize_render_stuff()
{
    PROFILE_FUNCTION()

    RECT win_rect;
    GetClientRect(_hwnd, &win_rect);
    _viewport = {0.0f, 0.0f, (FLOAT)(win_rect.right - win_rect.left), (FLOAT)(win_rect.bottom - win_rect.top), 0.0f, 1.0f};

    _device_context->Flush();

    _framebuffers[VR_Eye::None] = _create_framebuffer();

    VR_System& vr_system = VR_System::get();
    if (vr_system.is_valid())
    {
        _framebuffers[VR_Eye::Left] = _create_framebuffer();
        _framebuffers[VR_Eye::Right] = _create_framebuffer();
    }
}

void DirectX11_Renderer_Backend::_cleanup_render_stuff()
{
    PROFILE_FUNCTION()

    _destroy_framebuffer(_framebuffers[VR_Eye::None]);

    VR_System& vr_system = VR_System::get();
    if (vr_system.is_valid())
    {
        _destroy_framebuffer(_framebuffers[VR_Eye::Left]);
        _destroy_framebuffer(_framebuffers[VR_Eye::Right]);
    }
}

#endif // CS_PLATFORM_WINDOWS
