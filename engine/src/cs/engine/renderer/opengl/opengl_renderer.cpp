#include "cs/engine/renderer/opengl/opengl_renderer.hpp"
#include "cs/engine/window.hpp"
#include "GL/glew.h"

#include <fstream>
#include <sstream>


void OpenGL_Uniform_Buffer::bind() const
{
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
}

void OpenGL_Uniform_Buffer::unbind() const
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);  
}

void OpenGL_Uniform_Buffer::set_data(const void *data, uint32 size, uint32 offset)
{
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}

void OpenGL_Shader::bind() const
{
    glUseProgram(shader);
}

void OpenGL_Shader::unbind() const
{

}

void OpenGL_Mesh::upload_data()
{

}

//TODO: Merge between all renderers
struct
{
    mat4 world { mat4(1.0f) };
    mat4 world_inv_tran { mat4(1.0f) };
    mat4 view { mat4(1.0f) };
    mat4 projection { mat4(1.0f) };
} data;

void OpenGL_Renderer_Backend::initialize(const Shared_Ptr<Window> &window, const Shared_Ptr<VR_System>& vr_system)
{
    assert(window);
    _window = window;
    assert(glewInit() == GLEW_OK);
    glEnable(GL_DEPTH_TEST);
    
    _uniform_buffer = create_uniform_buffer(&data, sizeof(data));
}

void OpenGL_Renderer_Backend::set_camera(const Shared_Ptr<Camera> &camera)
{

}

void OpenGL_Renderer_Backend::render_frame()
{

}

void OpenGL_Renderer_Backend::begin_frame(VR_Eye::Type eye)
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

void OpenGL_Renderer_Backend::end_frame(VR_Eye::Type eye)
{
    _window->swap_buffers();
}

void OpenGL_Renderer_Backend::shutdown()
{

}

void OpenGL_Renderer_Backend::draw_mesh(const Shared_Ptr<Mesh>& mesh, const mat4& world_transform, VR_Eye::Type eye)
{
    Shared_Ptr<OpenGL_Mesh> gl_mesh = mesh;
    if (!gl_mesh)
    {
        return;
    }

    data.world = world_transform;
    data.world_inv_tran = data.world.inverse();
    // data.world.transpose();
    // data.view.transpose();
    // data.projection.transpose();
    
    for (const OpenGL_Submesh& submesh : gl_mesh->submeshes)
    {
        glBindVertexArray(submesh.vertex_array);
        submesh.shader->bind();
        
        // TODO: Change only when needed
        _uniform_buffer->bind();
        _uniform_buffer->set_data(&data, sizeof(data), 0);

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
    Shared_Ptr<OpenGL_Uniform_Buffer> gl_buffer = Shared_Ptr<OpenGL_Uniform_Buffer>::create();

    glGenBuffers(1, &gl_buffer->buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, gl_buffer->buffer);
    glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, gl_buffer->buffer, 0, size);

    glBindBuffer(GL_UNIFORM_BUFFER, gl_buffer->buffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    return gl_buffer;
}

Shared_Ptr<Buffer> OpenGL_Renderer_Backend::create_index_buffer(void *data, uint32 size)
{
    return Shared_Ptr<OpenGL_Buffer>::create();
}

void checkCompileErrors(GLuint shader, std::string type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            printf("ERROR::SHADER_COMPILATION_ERROR of type: %s\n\t%s", type, infoLog);
            printf("---------------------------------------------------\n");
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            printf("ERROR::PROGRAM_LINKING_ERROR of type: %s\n\t%s", type, infoLog);
            printf("---------------------------------------------------\n");
        }
    }
}

Shared_Ptr<Shader> OpenGL_Renderer_Backend::create_shader(const Shared_Ptr<Shader_Resource> &shader_resource)
{
    Shared_Ptr<OpenGL_Shader> gl_shader = Shared_Ptr<OpenGL_Shader>::create();

    shader_resource->vertex_filepath;
    shader_resource->pixel_filepath;

    // 1. retrieve the vertex/fragment source code from filePath
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::string vertexCode;
    std::string fragmentCode;

    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

    // open files
    vShaderFile.open(shader_resource->vertex_filepath);
    fShaderFile.open(shader_resource->pixel_filepath);
    std::stringstream vShaderStream, fShaderStream;
    // read file's buffer contents into streams
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();		
    // close file handlers
    vShaderFile.close();
    fShaderFile.close();
    // convert stream into string
    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();			

    const char* vShaderCode = vertexCode.c_str();
    const char * fShaderCode = fragmentCode.c_str();
    // 2. compile shaders
    unsigned int vertex, fragment;
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // shader Program
    gl_shader->shader = glCreateProgram();
    glAttachShader(gl_shader->shader, vertex);
    glAttachShader(gl_shader->shader, fragment);
    glLinkProgram(gl_shader->shader);
    checkCompileErrors(gl_shader->shader, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return gl_shader;
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
        gl_submesh.shader = create_shader(submesh.material_resource->shader_resource);
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
