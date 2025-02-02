#include "cs/engine/window/glfw/glfw_window.hpp"

#include <cstring>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

GLFW_Window::~GLFW_Window()
{
    if (_window) glfwDestroyWindow(_window);
    glfwTerminate();
}

void window_close_callback(GLFWwindow* window)
{
    GLFW_Window* glfw_window = static_cast<GLFW_Window*>(glfwGetWindowUserPointer(window));
    glfw_window->on_window_should_close.broadcast();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    GLFW_Window* glfw_window = static_cast<GLFW_Window*>(glfwGetWindowUserPointer(window));
    glfw_window->on_window_resize.broadcast(width, height);
}

bool GLFW_Window::initialize(int width, int height, const char* title) 
{
    if (!glfwInit()) 
    {
        //std::cerr << "Failed to initialize GLFW!" << std::endl;
        return false;
    }

    // DEFAULT SUPPORT FOR OpenGL - TODO: custom context from renderer
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // OpenGL version 4.x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5); // OpenGL version 4.5
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    _window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    if (!_window) 
    {
        //std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(_window, this);

    glfwSetFramebufferSizeCallback(_window, framebuffer_size_callback);
    glfwSetWindowCloseCallback(_window, window_close_callback);
    glfwMakeContextCurrent(_window);

    return true;
}

void GLFW_Window::poll_events() 
{
    glfwPollEvents();
}

bool GLFW_Window::should_close() const 
{
    return glfwWindowShouldClose(_window);
}

void GLFW_Window::swap_buffers()
{
    glfwSwapBuffers(_window);
}

void* GLFW_Window::native_handle() const 
{
    #ifdef CS_PLATFORM_WINDOWS
        return static_cast<void*>(glfwGetWin32Window(_window));
    #else
        //TODO: Warn
        //return static_cast<void*>(_window);
    #endif //CS_PLATFORM_WINDOWS
}
