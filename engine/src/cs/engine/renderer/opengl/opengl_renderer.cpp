#include "cs/engine/renderer/opengl/opengl_renderer.hpp"
#include "cs/engine/window.hpp"
#include "GL/glew.h"

#include <fstream>
#include <sstream>

#include <filesystem>

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

void OpenGL_Renderer_Backend::initialize(const Shared_Ptr<Window> &window)
{
    assert(window);
    _window = window;
    assert(glewInit() == GLEW_OK);
    glEnable(GL_DEPTH_TEST);
    
    _window->on_window_resize.bind([&](uint32 width, uint32 height)
    {
        _cleanup_render_stuff();
        _initialize_render_stuff(); 
    });

    _uniform_buffer = create_uniform_buffer(&data, sizeof(data));

    _initialize_render_stuff();
}

void OpenGL_Renderer_Backend::set_camera(const Shared_Ptr<Camera> &camera)
{
    _camera = camera;
}

void OpenGL_Renderer_Backend::render_frame()
{ 
#if WITH_VR_SUPPORT
    VR_System& vr_system = VR_System::get();
    if (vr_system.is_valid())
    {
        vr::VRTextureBounds_t bounds;
        bounds.uMin = 0.0f;
        bounds.uMax = 1.0f;
        bounds.vMin = 0.0f;
        bounds.vMax = 1.0f;
    
        vr::Texture_t leftEyeTexture = {(void*)(uintptr_t)_left_eye.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture );
        vr::Texture_t rightEyeTexture = {(void*)(uintptr_t)_right_eye.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture );
    }
#endif
    
    _window->swap_buffers();
}

void OpenGL_Renderer_Backend::begin_frame(VR_Eye::Type eye)
{

    switch(eye)
    {
        case VR_Eye::None:
        {
            glViewport(m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3]);
            //glBindFramebuffer( GL_FRAMEBUFFER, _basic.m_nRenderFramebufferId );
            break;
        }
        case VR_Eye::Left:
        {
            glViewport(0, 0, vr_render_viewport[0], vr_render_viewport[1]);
            glBindFramebuffer( GL_FRAMEBUFFER, _left_eye.m_nRenderFramebufferId );
            break;
        }
        case VR_Eye::Right:
        {
            glViewport(0, 0, vr_render_viewport[0], vr_render_viewport[1]);
            glBindFramebuffer( GL_FRAMEBUFFER, _right_eye.m_nRenderFramebufferId );
            break;
        }
    }

    glClearColor(0.05f, 0.05f, 0.75f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGL_Renderer_Backend::end_frame(VR_Eye::Type eye)
{
    switch(eye)
    {
        case VR_Eye::None:
        {
            //glBindFramebuffer( GL_FRAMEBUFFER, _basic.m_nRenderFramebufferId );
            break;
        }
        case VR_Eye::Left:
        {
            glDisable( GL_MULTISAMPLE );
                 
            glBindFramebuffer(GL_READ_FRAMEBUFFER, _left_eye.m_nRenderFramebufferId);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _left_eye.m_nResolveFramebufferId );
            
            glBlitFramebuffer( 0, 0, vr_render_viewport[0],  vr_render_viewport[1], 0, 0,  vr_render_viewport[0],  vr_render_viewport[1], 
                GL_COLOR_BUFFER_BIT,
                    GL_LINEAR );
            
                glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0 );	
            
            glEnable( GL_MULTISAMPLE );
            break;
        }
        case VR_Eye::Right:
        {
            glDisable( GL_MULTISAMPLE );
                 
            glBindFramebuffer(GL_READ_FRAMEBUFFER, _right_eye.m_nRenderFramebufferId);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _right_eye.m_nResolveFramebufferId );
            
            glBlitFramebuffer( 0, 0, vr_render_viewport[0],  vr_render_viewport[1], 0, 0,  vr_render_viewport[0],  vr_render_viewport[1], 
                GL_COLOR_BUFFER_BIT,
                    GL_LINEAR );
            
                glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0 );	
            
            glEnable( GL_MULTISAMPLE );
            break;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

#if WITH_VR_SUPPORT
    VR_System& vr_system = VR_System::get();
    if (vr_system.is_valid())
    {
        Shared_Ptr<Camera> camera = vr_system.get_camera(eye);

        data.view = camera->get_view();
        data.projection = camera->get_projection();
        static mat4 toZup = quat::from_euler_angles(vec3(MATH_DEG_TO_RAD(90.0f), 0.0f, 0.0f)).to_mat4();
        data.view = vr_system._get_eye_pose(eye) * vr_system._head_view_matrix * toZup;
        data.projection = vr_system._get_eye_projection(eye);
    }
    else 
#endif  // EUGH this is ugly >.>
    if (_camera)
    {
        _camera->aspect_ratio = 16.0f/9.0f;
        _camera->calculate_projection();
        _camera->calculate_view();

        data.view = _camera->get_view();
        data.projection = _camera->get_projection();
    }

    data.world = world_transform;
    data.world_inv_tran = data.world.inverse();

    for (const OpenGL_Submesh& submesh : gl_mesh->submeshes)
    {
        glBindVertexArray(submesh.vertex_array);
        submesh.shader->bind();
        
		glUniformMatrix4fv(submesh.shader->world_location, 1, GL_FALSE, (GLfloat*)&data.world);
		glUniformMatrix4fv(submesh.shader->world_inv_location, 1, GL_FALSE, (GLfloat*)&data.world_inv_tran);
		glUniformMatrix4fv(submesh.shader->view_location, 1, GL_FALSE, (GLfloat*)&data.view);
		glUniformMatrix4fv(submesh.shader->projection_location, 1, GL_FALSE, (GLfloat*)&data.projection);

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
            printf("ERROR::SHADER_COMPILATION_ERROR of type: %s\n\t%s", type.c_str(), infoLog);
            printf("---------------------------------------------------\n");
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            printf("ERROR::PROGRAM_LINKING_ERROR of type: %s\n\t%s", type.c_str(), infoLog);
            printf("---------------------------------------------------\n");
        }
    }
}

Shared_Ptr<Shader> OpenGL_Renderer_Backend::create_shader(const Shared_Ptr<Shader_Resource> &shader_resource)
{
    Shared_Ptr<OpenGL_Shader> gl_shader = Shared_Ptr<OpenGL_Shader>::create();
    gl_shader->shader_resource = shader_resource;

    // 1. retrieve the vertex/fragment source code from filePath
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::string vertexCode;
    std::string fragmentCode;

    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

    // open files
    std::string path = std::filesystem::current_path().string() + "/" + 
        shader_resource->source_paths[Renderer_API::OpenGL].vertex_filepath;
    vShaderFile.open(path);
    path = std::filesystem::current_path().string() + "/" + 
        shader_resource->source_paths[Renderer_API::OpenGL].fragment_filepath;
    fShaderFile.open(path);
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

    glUseProgram(gl_shader->shader);
	gl_shader->world_location = glGetUniformLocation(gl_shader->shader, "World");
	gl_shader->world_inv_location = glGetUniformLocation(gl_shader->shader, "WorldInverseTranspose");
	gl_shader->view_location = glGetUniformLocation(gl_shader->shader, "View");
	gl_shader->projection_location = glGetUniformLocation(gl_shader->shader, "Projection");
    glUseProgram(0);

    return gl_shader;
}

Shared_Ptr<Mesh> OpenGL_Renderer_Backend::create_mesh(const Shared_Ptr<Mesh_Resource> &mesh_resource)
{
    Shared_Ptr<OpenGL_Mesh> gl_mesh = Shared_Ptr<OpenGL_Mesh>::create();

    // create buffers/arrays
    // TODO: glGenBuffers for all submeshes at once
    for (const Submesh_Data& submesh : mesh_resource->submeshes)
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
    glGetIntegerv( GL_VIEWPORT, m_viewport );
    
    _basic = _create_framebuffer(m_viewport[2], m_viewport[3]);

#if WITH_VR_SUPPORT
    VR_System& vr_system = VR_System::get();
    if (vr_system.is_valid())
    {
        vr_system.get_viewport(vr_render_viewport[0], vr_render_viewport[1]);
        
        _left_eye = _create_framebuffer(vr_render_viewport[0], vr_render_viewport[1]);
        _right_eye = _create_framebuffer(vr_render_viewport[0], vr_render_viewport[1]);
    }
#endif
}

void OpenGL_Renderer_Backend::_cleanup_render_stuff()
{
    _destroy_framebuffer(_left_eye);
    _destroy_framebuffer(_right_eye);
    _destroy_framebuffer(_basic);
}

OpenGL_Renderer_Backend::OpenGL_Framebuffer OpenGL_Renderer_Backend::_create_framebuffer(int nWidth, int nHeight)
{
    OpenGL_Framebuffer gl_framebuffer;

	glGenFramebuffers(1, &gl_framebuffer.m_nRenderFramebufferId );
	glBindFramebuffer(GL_FRAMEBUFFER, gl_framebuffer.m_nRenderFramebufferId);

	glGenRenderbuffers(1, &gl_framebuffer.m_nDepthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, gl_framebuffer.m_nDepthBufferId);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, nWidth, nHeight );
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,	gl_framebuffer.m_nDepthBufferId );

	glGenTextures(1, &gl_framebuffer.m_nRenderTextureId );
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gl_framebuffer.m_nRenderTextureId );
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, nWidth, nHeight, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, gl_framebuffer.m_nRenderTextureId, 0);

	glGenFramebuffers(1, &gl_framebuffer.m_nResolveFramebufferId );
	glBindFramebuffer(GL_FRAMEBUFFER, gl_framebuffer.m_nResolveFramebufferId);

	glGenTextures(1, &gl_framebuffer.m_nResolveTextureId );
	glBindTexture(GL_TEXTURE_2D, gl_framebuffer.m_nResolveTextureId );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gl_framebuffer.m_nResolveTextureId, 0);

	// check FBO status
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	return gl_framebuffer;
}

void OpenGL_Renderer_Backend::_destroy_framebuffer(OpenGL_Renderer_Backend::OpenGL_Framebuffer& framebuffer)
{
    glDeleteRenderbuffers( 1, &framebuffer.m_nDepthBufferId );
    glDeleteTextures( 1, &framebuffer.m_nRenderTextureId );
    glDeleteFramebuffers( 1, &framebuffer.m_nRenderFramebufferId );
    glDeleteTextures( 1, &framebuffer.m_nResolveTextureId );
    glDeleteFramebuffers( 1, &framebuffer.m_nResolveFramebufferId );
}