#include "cs/engine/renderer/directx/directx_renderer.hpp"
#include "cs/engine/renderer/camera.hpp"
#include "cs/engine/renderer/mesh.hpp"
#include "cs/memory/shared_ptr.hpp"
#include "cs/engine/window.hpp"
#include <GL/glew.h>

#include <memory>

#ifdef CS_PLATFORM_WINDOWS
#include <d3d11.h>       // D3D interface
#include <dxgi.h>        // DirectX driver interface
#include <d3dcompiler.h> // shader compiler
#include <DirectXMath.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")

void DirectX_Buffer::unbind() const
{
    device_context->PSSetConstantBuffers(0, 1, nullptr);
    device_context->VSSetConstantBuffers(0, 1, nullptr);
}

void DirectX_Uniform_Buffer::bind() const
{
    device_context->PSSetConstantBuffers(0, 1, buffer.GetAddressOf());
    device_context->VSSetConstantBuffers(0, 1, buffer.GetAddressOf());
}

void DirectX_Uniform_Buffer::set_data(const void* data, uint32 size, uint32 offset)
{
    D3D11_MAPPED_SUBRESOURCE mapped_constant_buffer;
    HRESULT hr = device_context->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_constant_buffer);
    assert(SUCCEEDED(hr));
    CopyMemory(mapped_constant_buffer.pData, data, size);
    device_context->Unmap(buffer.Get(), 0);
}

void DirectX_Vertex_Buffer::bind() const
{
    // TODO: Get topology from class
    device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_context->IASetVertexBuffers(0, 1, buffer.GetAddressOf(), &stride, &offset);
}

void DirectX_Index_Buffer::bind() const
{
    device_context->IASetIndexBuffer(buffer.Get(), DXGI_FORMAT_R32_UINT, offset);
}

void DirectX_Shader::bind() const
{
    device_context->IASetInputLayout(vertex_layout.Get());
    device_context->VSSetShader(vertex_shader.Get(), nullptr, 0);
    device_context->PSSetShader(pixel_shader.Get(), nullptr, 0);
}

void DirectX_Shader::unbind() const
{
    device_context->IASetInputLayout(nullptr);
    device_context->VSSetShader(nullptr, nullptr, 0);
    device_context->PSSetShader(nullptr, nullptr, 0);
}

DirectX_Mesh::DirectX_Mesh(const Shared_Ptr<Mesh_Resource>& in_mesh_resource)
    : Mesh(in_mesh_resource)
{
}

void DirectX_Mesh::upload_data()
{
    for (int32 s = 0; s < submeshes.size(); ++s)
    {
        DirectX_Submesh& submesh = submeshes[s];
        const Submesh_Data& submesh_data = mesh_resource->submeshes[s];

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

DirectX_Renderer_Backend::~DirectX_Renderer_Backend()
{

}

struct
{
    mat4 world { mat4(1.0f) };
    mat4 world_inv_tran { mat4(1.0f) };
    mat4 view { mat4(1.0f) };
    mat4 projection { mat4(1.0f) };
} data;

void DirectX_Renderer_Backend::initialize(const Shared_Ptr<Window>& window)
{
    _window = window;
    _hwnd = static_cast<HWND>(window->native_handle());

    // Create the Direct3D device and swap chain
    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
    swap_chain_desc.BufferCount = 1;
    swap_chain_desc.BufferDesc.Width = (uint32) _viewport.Width;
    swap_chain_desc.BufferDesc.Height = (uint32) _viewport.Height;
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.OutputWindow = _hwnd;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.Windowed = TRUE;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION, &swap_chain_desc, &_swapchain, &_device, nullptr, &_device_context
    );
    assert(SUCCEEDED( hr ));

    _window->on_window_resize.bind([&](uint32 width, uint32 height) {
        _cleanup_render_stuff();
        _initialize_render_stuff();
    });
    
    _uniform_buffer = create_uniform_buffer(&data, sizeof(data));

    _initialize_render_stuff();
}

void DirectX_Renderer_Backend::set_camera(const Shared_Ptr<Camera>& camera)
{
    _camera = camera;
}

void DirectX_Renderer_Backend::begin_frame()
{
    constexpr float clearColor[] = { 0.1f, 0.3f, 0.1f, 1.0f };
    _device_context->ClearRenderTargetView(_render_target_view.Get(), clearColor);
    _device_context->ClearDepthStencilView(_depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    _device_context->RSSetViewports(1, &_viewport);
    _device_context->OMSetRenderTargets(1, _render_target_view.GetAddressOf(), _depth_stencil_view.Get());
}

void DirectX_Renderer_Backend::end_frame()
{
}

void DirectX_Renderer_Backend::render_frame()
{
    _swapchain->Present(1, 0);
}

void DirectX_Renderer_Backend::shutdown()
{
    if (_swapchain) _swapchain->Release();
    if (_device) _device->Release();
    if (_device_context) _device_context->Release();
}

UINT vertex_stride              = sizeof( Vertex_Data );
UINT vertex_offset              = 0;
float angle = 0;
void DirectX_Renderer_Backend::draw_mesh(const Shared_Ptr<Mesh>& mesh, const mat4& world_transform)
{
    Shared_Ptr<DirectX_Mesh> dx_mesh = mesh;
    if (!dx_mesh)
    {
        return;
    }

    //TODO: Change only when needed and per shader that needs it
    if (_camera)
    {
        _camera->aspect_ratio = _viewport.Width / _viewport.Height;
        _camera->calculate_projection();
        _camera->calculate_view();

        data.projection = _camera->get_projection();
        data.view = _camera->get_view();
    }

    data.world = world_transform;
    //data.world = rotate(data.world, MATH_DEG_TO_RAD(angle), {0.0f, 0.0f, 1.0f});
    //data.world = rotate(data.world, MATH_DEG_TO_RAD(90.0), {1.0f, 0.0f, 0.0f});
    //angle += 1.0f;

    data.world_inv_tran = data.world.inverse();
    data.world.transpose();
    data.view.transpose();
    data.projection.transpose();

    for (const DirectX_Submesh& submesh : dx_mesh->submeshes)
    {
        // TODO: Change only when needed
        _uniform_buffer->set_data(&data, sizeof(data), 0);
        _uniform_buffer->bind();

        _device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        _device_context->IASetVertexBuffers(0, 1, submesh.vertex_buffer.GetAddressOf(), &vertex_stride, &vertex_offset);

        submesh.shader->bind();
        _device_context->Draw(submesh.vertices_count, 0);
        submesh.shader->unbind();
    }
}

Shared_Ptr<Buffer> DirectX_Renderer_Backend::create_vertex_buffer(void *data, uint32 size)
{
    //TODO: Won't work
    Shared_Ptr<DirectX_Buffer> buffer = Shared_Ptr<DirectX_Buffer>::create();
    buffer->device_context = _device_context;

    D3D11_BUFFER_DESC buffer_desc{0};
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;// | D3D11_CPU_ACCESS_READ;
    buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
    buffer_desc.ByteWidth = (uint32) size;

    D3D11_SUBRESOURCE_DATA subresource_data{0};
    subresource_data.pSysMem = data;

    HRESULT hr = _device->CreateBuffer(&buffer_desc, &subresource_data, &buffer->buffer);
    assert(SUCCEEDED(hr));

    return buffer;
}

Shared_Ptr<Buffer> DirectX_Renderer_Backend::create_index_buffer(void *data, uint32 size)
{
    Shared_Ptr<DirectX_Index_Buffer> buffer = Shared_Ptr<DirectX_Index_Buffer>::create();
    D3D11_BUFFER_DESC buffer_desc{0};
    buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;// | D3D11_CPU_ACCESS_READ;
    buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
    buffer_desc.ByteWidth = size;

    D3D11_SUBRESOURCE_DATA subresource_data{0};
    subresource_data.pSysMem = data;

    HRESULT hr = _device->CreateBuffer(&buffer_desc, &subresource_data, &buffer->buffer);
    assert(SUCCEEDED(hr));
    
    return buffer;
}

Shared_Ptr<Buffer> DirectX_Renderer_Backend::create_uniform_buffer(void *data, uint32 size)
{
    Shared_Ptr<DirectX_Uniform_Buffer> buffer = Shared_Ptr<DirectX_Uniform_Buffer>::create();
    buffer->device_context = _device_context;

    D3D11_BUFFER_DESC buffer_desc = { 0 };
    buffer_desc.ByteWidth = size;
    buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    buffer_desc.MiscFlags = 0;
    buffer_desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA resource_data = { 0 };
    resource_data.pSysMem = data;

    HRESULT hr = _device->CreateBuffer(&buffer_desc, &resource_data, &buffer->buffer);
    assert(SUCCEEDED(hr));

    buffer->bind();

    return buffer;
}

Shared_Ptr<Shader> DirectX_Renderer_Backend::create_shader(const Shared_Ptr<Shader_Resource>& shader_resource)
{
    Shared_Ptr<DirectX_Shader> shader = Shared_Ptr<DirectX_Shader>::create();

    shader->device_context = _device_context;

    ComPtr<ID3DBlob> vertex_shader_blob = nullptr;
    shader->vertex_shader = _create_vertex_shader(shader_resource->vertex_filepath, vertex_shader_blob);
    shader->pixel_shader = _create_pixel_shader(shader_resource->pixel_filepath);

    // TODO: From Shader definition
    constexpr D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
    {
        { "POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    assert(SUCCEEDED(_device->CreateInputLayout(input_element_desc, ARRAYSIZE(input_element_desc),
        vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(), &shader->vertex_layout)));

    return shader;
}

Shared_Ptr<Mesh> DirectX_Renderer_Backend::create_mesh(const Shared_Ptr<Mesh_Resource>& mesh_resource)
{
    Shared_Ptr<DirectX_Mesh> dx_mesh = Shared_Ptr<DirectX_Mesh>::create(mesh_resource);
    dx_mesh->device = _device;

    for (const Submesh_Data& submesh : mesh_resource->submeshes)
    {
        DirectX_Submesh dx_submesh;

        // TODO: map with materials and existing shaders so we don't duplicate
        dx_submesh.shader = create_shader(submesh.material_resource->shader_resource);
        dx_mesh->submeshes.add(dx_submesh);
    }

    return dx_mesh;
}

#include <iostream>
bool DirectX_Renderer_Backend::_compile_shader(const char* filename, const char* entry_point, const char* profile, ComPtr<ID3DBlob>& shader_blob)
{
    constexpr UINT compile_flags = D3DCOMPILE_ENABLE_STRICTNESS;

    ComPtr<ID3DBlob> temp_shader_blob = nullptr;
    ComPtr<ID3DBlob> error_blob = nullptr;

    size_t num_chars;
    std::wstring w_filename( strlen(filename), L'#' );
    mbstowcs_s(&num_chars, &w_filename[0], w_filename.capacity(), filename, strlen(filename));
    
    HRESULT hr = D3DCompileFromFile( w_filename.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry_point, profile, 
        compile_flags, 0, &temp_shader_blob, &error_blob);

    if (FAILED(hr))
    {
        std::string e((char*)error_blob->GetBufferPointer());
        assert(false);
    }

    shader_blob = std::move(temp_shader_blob);
    return true;
}

ComPtr<ID3D11VertexShader> DirectX_Renderer_Backend::_create_vertex_shader(const char* filename, ComPtr<ID3DBlob>& vertex_shader_blob)
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

ComPtr<ID3D11PixelShader> DirectX_Renderer_Backend::_create_pixel_shader(const char* filename)
{
    ComPtr<ID3DBlob> pixel_shader_blob = nullptr;
    if (!_compile_shader(filename, "main", "ps_5_0", pixel_shader_blob))
    {
        return nullptr;
    }

    ComPtr<ID3D11PixelShader> pixel_shader;
    assert (SUCCEEDED(_device->CreatePixelShader(
        pixel_shader_blob->GetBufferPointer(),
        pixel_shader_blob->GetBufferSize(),
        nullptr, &pixel_shader)));

    return pixel_shader;
}

void DirectX_Renderer_Backend::_initialize_render_stuff()
{
    RECT win_rect;
    GetClientRect( _hwnd, &win_rect );
    _viewport = { 0.0f, 0.0f, ( FLOAT )( win_rect.right - win_rect.left ), ( FLOAT )( win_rect.bottom - win_rect.top ), 0.0f, 1.0f };

    _device_context->Flush();

    ComPtr<ID3D11Texture2D> framebuffer_texture;
    HRESULT hr = _swapchain->GetBuffer(0, __uuidof( ID3D11Texture2D ), (void**)framebuffer_texture.GetAddressOf());
    assert(SUCCEEDED( hr ));

    hr = _device->CreateRenderTargetView(framebuffer_texture.Get(), 0, &_render_target_view );
    assert(SUCCEEDED( hr ));
    framebuffer_texture->Release();

    // TODO: Update on resize
    D3D11_TEXTURE2D_DESC depth_texture_desc;
    ZeroMemory(&depth_texture_desc, sizeof(depth_texture_desc));
    depth_texture_desc.Width = (uint32) _viewport.Width;
    depth_texture_desc.Height = (uint32) _viewport.Height;
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

    hr = _device->CreateDepthStencilView(depth_stencil_texture, &depth_stencil_view_desc, &_depth_stencil_view);
    assert(SUCCEEDED(hr));
    depth_stencil_texture->Release();
}

void DirectX_Renderer_Backend::_cleanup_render_stuff()
{
    _render_target_view.Reset();
    _depth_stencil_view.Reset();
}

#endif //CS_PLATFORM_WINDOWS
