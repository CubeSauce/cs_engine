#include "cs/engine/renderer/opengl/opengl_renderer.hpp"

#include "GL/glew.h"

void OpenGL_Shader::bind() const
{

}

void OpenGL_Shader::unbind() const
{

}

void OpenGL_Mesh::upload_data()
{

}

OpenGL_Renderer_Backend::~OpenGL_Renderer_Backend()
{

}

void OpenGL_Renderer_Backend::initialize(const Shared_Ptr<Window> &window)
{
    assert(glewInit() == GLEW_OK);



    glEnable(GL_DEPTH_TEST);
}

void OpenGL_Renderer_Backend::set_camera(const Shared_Ptr<Camera> &camera)
{

}

void OpenGL_Renderer_Backend::render_frame()
{

}

struct
{
    mat4 world { mat4(1.0f) };
    mat4 world_inv_tran { mat4(1.0f) };
    mat4 view { mat4(1.0f) };
    mat4 projection { mat4(1.0f) };
} data;

void OpenGL_Renderer_Backend::begin_frame()
{
    glClearColor(0.05f, 0.05f, 0.75f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (_camera)
    {
        //TODO: _camera->aspect_ratio = _viewport.Width / _viewport.Height;
        _camera->aspect_ratio = 16.0/9.0f;
        _camera->calculate_projection();
        _camera->calculate_view();

        data.projection = _camera->get_projection();
        data.view = _camera->get_view();
    }
}

void OpenGL_Renderer_Backend::end_frame()
{

}

void OpenGL_Renderer_Backend::shutdown()
{

}

void OpenGL_Renderer_Backend::draw_mesh(const Shared_Ptr<Mesh>& mesh, const mat4& world_transform)
{
    Shared_Ptr<OpenGL_Mesh> gl_mesh = mesh;
    if (!gl_mesh)
    {
        return;
    }

    data.world = world_transform;
    data.world_inv_tran = data.world.inverse();
    //data.world.transpose();
    //data.view.transpose();
    //data.projection.transpose();
    
    for (const OpenGL_Submesh& submesh : gl_mesh->submeshes)
    {
        // TODO: Change only when needed
        _uniform_buffer->set_data(&data, sizeof(data), 0);
        _uniform_buffer->bind();

        glBindVertexArray(submesh.vertex_array);
        submesh.shader->bind();
        glDrawArrays(GL_TRIANGLES, 0, submesh.vertices_count);
        submesh.shader->unbind();
        glBindVertexArray(0);
    }

}

Shared_Ptr<Buffer> OpenGL_Renderer_Backend::create_vertex_buffer(void *data, uint32 size)
{
    return Shared_Ptr<OpenGL_Buffer>::create();
}

Shared_Ptr<Buffer> OpenGL_Renderer_Backend::create_uniform_buffer(void *data, uint32 size)
{
    return Shared_Ptr<OpenGL_Buffer>::create();
}

Shared_Ptr<Buffer> OpenGL_Renderer_Backend::create_index_buffer(void *data, uint32 size)
{
    return Shared_Ptr<OpenGL_Buffer>::create();
}

Shared_Ptr<Shader> OpenGL_Renderer_Backend::create_shader(const Shared_Ptr<Shader_Resource> &shader_resource)
{
    return Shared_Ptr<OpenGL_Shader>::create();
}

Shared_Ptr<Mesh> OpenGL_Renderer_Backend::create_mesh(const Shared_Ptr<Mesh_Resource> &mesh)
{
    Shared_Ptr<OpenGL_Mesh> gl_mesh = Shared_Ptr<OpenGL_Mesh>::create();

    // create buffers/arrays
    // TODO: glGenBuffers for all submeshes at once
    for (const Submesh_Data& submesh : mesh->submeshes)
    {
        OpenGL_Submesh gl_submesh;

        glGenVertexArrays(1, &gl_submesh.vertex_array);
        glBindVertexArray(gl_submesh.vertex_array);

        glGenBuffers(1, &gl_submesh.vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, gl_submesh.vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, submesh.vertices.size_in_bytes(), submesh.vertices.begin(), GL_STATIC_DRAW); 

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_Data), (void*)offsetof(Vertex_Data, vertex_location));
        // vertex normals
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_Data), (void*)offsetof(Vertex_Data, vertex_normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_Data), (void*)offsetof(Vertex_Data, vertex_texture_coordinate));

        //index/elements buffer
        gl_submesh.vertices_count = submesh.vertices.size(); 
        gl_mesh->submeshes.add(gl_submesh);
    }
        
    return gl_mesh;
}

void OpenGL_Renderer_Backend::_initialize_render_stuff()
{
    
}

void OpenGL_Renderer_Backend::_cleanup_render_stuff()
{
    
}
