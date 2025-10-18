#include "cs/engine/window/glfw/glfw_window.hpp"
#include "cs/engine/profiling/profiler.hpp"
#include "cs/engine/input.hpp"

#include <string>
#include <cstring>

#include <GLFW/glfw3.h>
#ifdef CS_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(CS_PLATFORM_APPLE)
#define GLFW_EXPOSE_NATIVE_COCOA
#endif //CS_PLATFORM_WINDOWS
#include <GLFW/glfw3native.h>

Name_Id glfw_key_name_to_name_id(int32 key)
{
    switch(key)
    {
    case GLFW_KEY_SPACE: return Name_Id("KEY_SPACE");
    case GLFW_KEY_APOSTROPHE: return Name_Id("KEY_APOSTROPHE");
    case GLFW_KEY_COMMA: return Name_Id("KEY_COMMA");
    case GLFW_KEY_MINUS: return Name_Id("KEY_MINUS");
    case GLFW_KEY_PERIOD: return Name_Id("KEY_PERIOD");
    case GLFW_KEY_SLASH: return Name_Id("KEY_SLASH");
    case GLFW_KEY_0: return Name_Id("KEY_0");
    case GLFW_KEY_1: return Name_Id("KEY_1");
    case GLFW_KEY_2: return Name_Id("KEY_2");
    case GLFW_KEY_3: return Name_Id("KEY_3");
    case GLFW_KEY_4: return Name_Id("KEY_4");
    case GLFW_KEY_5: return Name_Id("KEY_5");
    case GLFW_KEY_6: return Name_Id("KEY_6");
    case GLFW_KEY_7: return Name_Id("KEY_7");
    case GLFW_KEY_8: return Name_Id("KEY_8");
    case GLFW_KEY_9: return Name_Id("KEY_9");
    case GLFW_KEY_SEMICOLON: return Name_Id("KEY_SEMICOLON");
    case GLFW_KEY_EQUAL: return Name_Id("KEY_EQUAL");
    case GLFW_KEY_A: return Name_Id("KEY_A");
    case GLFW_KEY_B: return Name_Id("KEY_B");
    case GLFW_KEY_C: return Name_Id("KEY_C");
    case GLFW_KEY_D: return Name_Id("KEY_D");
    case GLFW_KEY_E: return Name_Id("KEY_E");
    case GLFW_KEY_F: return Name_Id("KEY_F");
    case GLFW_KEY_G: return Name_Id("KEY_G");
    case GLFW_KEY_H: return Name_Id("KEY_H");
    case GLFW_KEY_I: return Name_Id("KEY_I");
    case GLFW_KEY_J: return Name_Id("KEY_J");
    case GLFW_KEY_K: return Name_Id("KEY_K");
    case GLFW_KEY_L: return Name_Id("KEY_L");
    case GLFW_KEY_M: return Name_Id("KEY_M");
    case GLFW_KEY_N: return Name_Id("KEY_N");
    case GLFW_KEY_O: return Name_Id("KEY_O");
    case GLFW_KEY_P: return Name_Id("KEY_P");
    case GLFW_KEY_Q: return Name_Id("KEY_Q");
    case GLFW_KEY_R: return Name_Id("KEY_R");
    case GLFW_KEY_S: return Name_Id("KEY_S");
    case GLFW_KEY_T: return Name_Id("KEY_T");
    case GLFW_KEY_U: return Name_Id("KEY_U");
    case GLFW_KEY_V: return Name_Id("KEY_V");
    case GLFW_KEY_W: return Name_Id("KEY_W");
    case GLFW_KEY_X: return Name_Id("KEY_X");
    case GLFW_KEY_Y: return Name_Id("KEY_Y");
    case GLFW_KEY_Z: return Name_Id("KEY_Z");
    case GLFW_KEY_LEFT_BRACKET: return Name_Id("KEY_LEFT_BRACKET");
    case GLFW_KEY_BACKSLASH: return Name_Id("KEY_BACKSLASH");
    case GLFW_KEY_RIGHT_BRACKET: return Name_Id("KEY_RIGHT_BRACKET");
    case GLFW_KEY_GRAVE_ACCENT: return Name_Id("KEY_GRAVE_ACCENT");
    case GLFW_KEY_ESCAPE: return Name_Id("KEY_ESCAPE");
    case GLFW_KEY_ENTER: return Name_Id("KEY_ENTER");
    case GLFW_KEY_TAB: return Name_Id("KEY_TAB");
    case GLFW_KEY_BACKSPACE: return Name_Id("KEY_BACKSPACE");
    case GLFW_KEY_INSERT: return Name_Id("KEY_INSERT");
    case GLFW_KEY_DELETE: return Name_Id("KEY_DELETE");
    case GLFW_KEY_RIGHT: return Name_Id("KEY_RIGHT");
    case GLFW_KEY_LEFT: return Name_Id("KEY_LEFT");
    case GLFW_KEY_DOWN: return Name_Id("KEY_DOWN");
    case GLFW_KEY_UP: return Name_Id("KEY_UP");
    case GLFW_KEY_PAGE_UP: return Name_Id("KEY_PAGE_UP");
    case GLFW_KEY_PAGE_DOWN: return Name_Id("KEY_PAGE_DOWN");
    case GLFW_KEY_HOME: return Name_Id("KEY_HOME");
    case GLFW_KEY_END: return Name_Id("KEY_END");
    case GLFW_KEY_CAPS_LOCK: return Name_Id("KEY_CAPS_LOCK");
    case GLFW_KEY_SCROLL_LOCK: return Name_Id("KEY_SCROLL_LOCK");
    case GLFW_KEY_NUM_LOCK: return Name_Id("KEY_NUM_LOCK");
    case GLFW_KEY_PRINT_SCREEN: return Name_Id("KEY_PRINT_SCREEN");
    case GLFW_KEY_PAUSE: return Name_Id("KEY_PAUSE");
    case GLFW_KEY_F1: return Name_Id("KEY_F1");
    case GLFW_KEY_F2: return Name_Id("KEY_F2");
    case GLFW_KEY_F3: return Name_Id("KEY_F3");
    case GLFW_KEY_F4: return Name_Id("KEY_F4");
    case GLFW_KEY_F5: return Name_Id("KEY_F5");
    case GLFW_KEY_F6: return Name_Id("KEY_F6");
    case GLFW_KEY_F7: return Name_Id("KEY_F7");
    case GLFW_KEY_F8: return Name_Id("KEY_F8");
    case GLFW_KEY_F9: return Name_Id("KEY_F9");
    case GLFW_KEY_F10: return Name_Id("KEY_F10");
    case GLFW_KEY_F11: return Name_Id("KEY_F11");
    case GLFW_KEY_F12: return Name_Id("KEY_F12");
    case GLFW_KEY_F13: return Name_Id("KEY_F13");
    case GLFW_KEY_F14: return Name_Id("KEY_F14");
    case GLFW_KEY_F15: return Name_Id("KEY_F15");
    case GLFW_KEY_F16: return Name_Id("KEY_F16");
    case GLFW_KEY_F17: return Name_Id("KEY_F17");
    case GLFW_KEY_F18: return Name_Id("KEY_F18");
    case GLFW_KEY_F19: return Name_Id("KEY_F19");
    case GLFW_KEY_F20: return Name_Id("KEY_F20");
    case GLFW_KEY_F21: return Name_Id("KEY_F21");
    case GLFW_KEY_F22: return Name_Id("KEY_F22");
    case GLFW_KEY_F23: return Name_Id("KEY_F23");
    case GLFW_KEY_F24: return Name_Id("KEY_F24");
    case GLFW_KEY_F25: return Name_Id("KEY_F25");
    case GLFW_KEY_KP_0: return Name_Id("KEY_KP_0");
    case GLFW_KEY_KP_1: return Name_Id("KEY_KP_1");
    case GLFW_KEY_KP_2: return Name_Id("KEY_KP_2");
    case GLFW_KEY_KP_3: return Name_Id("KEY_KP_3");
    case GLFW_KEY_KP_4: return Name_Id("KEY_KP_4");
    case GLFW_KEY_KP_5: return Name_Id("KEY_KP_5");
    case GLFW_KEY_KP_6: return Name_Id("KEY_KP_6");
    case GLFW_KEY_KP_7: return Name_Id("KEY_KP_7");
    case GLFW_KEY_KP_8: return Name_Id("KEY_KP_8");
    case GLFW_KEY_KP_9: return Name_Id("KEY_KP_9");
    case GLFW_KEY_KP_DECIMAL: return Name_Id("KEY_KP_DECIMAL");
    case GLFW_KEY_KP_DIVIDE: return Name_Id("KEY_KP_DIVIDE");
    case GLFW_KEY_KP_MULTIPLY: return Name_Id("KEY_KP_MULTIPLY");
    case GLFW_KEY_KP_SUBTRACT: return Name_Id("KEY_KP_SUBTRACT");
    case GLFW_KEY_KP_ADD: return Name_Id("KEY_KP_ADD");
    case GLFW_KEY_KP_ENTER: return Name_Id("KEY_KP_ENTER");
    case GLFW_KEY_KP_EQUAL: return Name_Id("KEY_KP_EQUAL");
    case GLFW_KEY_LEFT_SHIFT: return Name_Id("KEY_LEFT_SHIFT");
    case GLFW_KEY_LEFT_CONTROL: return Name_Id("KEY_LEFT_CONTROL");
    case GLFW_KEY_LEFT_ALT: return Name_Id("KEY_LEFT_ALT");
    case GLFW_KEY_LEFT_SUPER: return Name_Id("KEY_LEFT_SUPER");
    case GLFW_KEY_RIGHT_SHIFT: return Name_Id("KEY_RIGHT_SHIFT");
    case GLFW_KEY_RIGHT_CONTROL: return Name_Id("KEY_RIGHT_CONTROL");
    case GLFW_KEY_RIGHT_ALT: return Name_Id("KEY_RIGHT_ALT");
    case GLFW_KEY_RIGHT_SUPER: return Name_Id("KEY_RIGHT_SUPER");
    case GLFW_KEY_MENU: return Name_Id("KEY_MENU");
    default: return Name_Id("KEY_NONE");
    }
}

Name_Id glfw_button_name_to_name_id(int32 button)
{
    switch(button)
    {
    case GLFW_MOUSE_BUTTON_LEFT: return Name_Id("MOUSE_BUTTON_LEFT");
    case GLFW_MOUSE_BUTTON_RIGHT: return Name_Id("MOUSE_BUTTON_RIGHT");
    case GLFW_MOUSE_BUTTON_MIDDLE: return Name_Id("MOUSE_BUTTON_MIDDLE");
    case GLFW_MOUSE_BUTTON_4: return Name_Id("MOUSE_BUTTON_4");
    case GLFW_MOUSE_BUTTON_5: return Name_Id("MOUSE_BUTTON_5");
    case GLFW_MOUSE_BUTTON_6: return Name_Id("MOUSE_BUTTON_6");
    case GLFW_MOUSE_BUTTON_7: return Name_Id("MOUSE_BUTTON_7");
    case GLFW_MOUSE_BUTTON_8: return Name_Id("MOUSE_BUTTON_8");
    default: return Name_Id("BUTTON_NONE");
    }
}

Name_Id glfw_gamepad_button_to_name_id(int32 button)
{
    switch(button)
    {
    case GLFW_GAMEPAD_BUTTON_A: return Name_Id("GAMEPAD_FACE_DOWN");
    case GLFW_GAMEPAD_BUTTON_B: return Name_Id("GAMEPAD_FACE_RIGHT");
    case GLFW_GAMEPAD_BUTTON_X: return Name_Id("GAMEPAD_FACE_LEFT");
    case GLFW_GAMEPAD_BUTTON_Y: return Name_Id("GAMEPAD_FACE_UP");
    case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER: return Name_Id("GAMEPAD_BUTTON_LEFT_BUMPER");
    case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER: return Name_Id("GAMEPAD_BUTTON_RIGHT_BUMPER");
    case GLFW_GAMEPAD_BUTTON_BACK: return Name_Id("GAMEPAD_BUTTON_BACK");
    case GLFW_GAMEPAD_BUTTON_START: return Name_Id("GAMEPAD_BUTTON_START");
    case GLFW_GAMEPAD_BUTTON_GUIDE: return Name_Id("GAMEPAD_BUTTON_GUIDE");
    case GLFW_GAMEPAD_BUTTON_LEFT_THUMB: return Name_Id("GAMEPAD_BUTTON_LEFT_THUMB");
    case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB: return Name_Id("GAMEPAD_BUTTON_RIGHT_THUMB");
    case GLFW_GAMEPAD_BUTTON_DPAD_UP: return Name_Id("GAMEPAD_BUTTON_DPAD_UP");
    case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT: return Name_Id("GAMEPAD_BUTTON_DPAD_RIGHT");
    case GLFW_GAMEPAD_BUTTON_DPAD_DOWN: return Name_Id("GAMEPAD_BUTTON_DPAD_DOWN");
    case GLFW_GAMEPAD_BUTTON_DPAD_LEFT: return Name_Id("GAMEPAD_BUTTON_DPAD_LEFT");
    default: return Name_Id("GAMEPAD_BUTTON_NONE");
    }
}

Name_Id glfw_gamepad_axis_to_name_id(int32 axis)
{
    switch(axis)
    {
    case GLFW_GAMEPAD_AXIS_LEFT_X: return Name_Id("GAMEPAD_AXIS_LEFT_X");
    case GLFW_GAMEPAD_AXIS_LEFT_Y: return Name_Id("GAMEPAD_AXIS_LEFT_Y");
    case GLFW_GAMEPAD_AXIS_RIGHT_X: return Name_Id("GAMEPAD_AXIS_RIGHT_X");
    case GLFW_GAMEPAD_AXIS_RIGHT_Y: return Name_Id("GAMEPAD_AXIS_RIGHT_Y");
    case GLFW_GAMEPAD_AXIS_LEFT_TRIGGER: return Name_Id("GAMEPAD_AXIS_LEFT_TRIGGER");
    case GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER: return Name_Id("GAMEPAD_AXIS_RIGHT_TRIGGER");
    default: return Name_Id("GAMEPAD_AXIS_NONE");
    }
}


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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_REPEAT)
    {
        return;
    }

    GLFW_Window* glfw_window = static_cast<GLFW_Window*>(glfwGetWindowUserPointer(window));
    glfw_window->input_source.broadcast(glfw_key_name_to_name_id(key),
        action == GLFW_PRESS ? 1.0f : 0.0f);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_REPEAT)
    {
        return;
    }
    
    GLFW_Window* glfw_window = static_cast<GLFW_Window*>(glfwGetWindowUserPointer(window));
    glfw_window->input_source.broadcast(glfw_button_name_to_name_id(button),
        action == GLFW_PRESS ? 1.0f : 0.0f);
}

Event<Name_Id, float> joystick_input_source;
Dynamic_Array<int32> connected_joysticks;
void joystick_callback(int32 jid, int32 event)
{
    if (event == GLFW_CONNECTED)
    {
        //glfwSetJoystickUserPointer(jid, joystick_input_source);
        connected_joysticks.push_back(jid);
    }
    else if (event == GLFW_DISCONNECTED)
    {
        // The joystick was disconnected
    }
}

bool GLFW_Window::initialize(int32 width, int32 height, const char* title) 
{
    assert(glfwInit() == GLFW_TRUE);

    // DEFAULT SUPPORT FOR OpenGL - TODO: custom context from renderer
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // OpenGL version 4.x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0); // OpenGL version 4.5
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    _window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    assert(_window);

    _width = width;
    _height = height;
    on_window_resize.bind([this](uint32 width, uint32 height) {
        _width = width;
        _height = height;
    });

    glfwSetWindowUserPointer(_window, this);
    _gamepad_state = new GLFWgamepadstate[GLFW_JOYSTICK_LAST];
    memset(_gamepad_state, 0, sizeof(GLFWgamepadstate) * GLFW_JOYSTICK_LAST);
    _previous_gamepad_state = new GLFWgamepadstate[GLFW_JOYSTICK_LAST];
    memset(_previous_gamepad_state, 0, sizeof(GLFWgamepadstate) * GLFW_JOYSTICK_LAST);

    for (int32 jid = GLFW_JOYSTICK_1; jid < GLFW_JOYSTICK_LAST; ++jid)
    {
        if (glfwJoystickPresent(jid))
        {
            connected_joysticks.push_back(jid);
            //glfwSetJoystickUserPointer(jid, joystick_input_source.get());
        }
    }

    glfwSetFramebufferSizeCallback(_window, framebuffer_size_callback);
    glfwSetWindowCloseCallback(_window, window_close_callback);
    glfwSetMouseButtonCallback(_window, mouse_button_callback);
    glfwSetKeyCallback(_window, key_callback);
    glfwSetJoystickCallback(joystick_callback);

    glfwMakeContextCurrent(_window);
    Input_System::get().register_input_source(input_source);
    Input_System::get().register_input_source(joystick_input_source);

    return true;
}

void GLFW_Window::poll_events() 
{
    PROFILE_FUNCTION()

    glfwPollEvents();
    _poll_analog_inputs();
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
    #elif defined(CS_PLATFORM_APPLE)
        return static_cast<void*>(glfwGetCocoaWindow(_window));
    #endif //CS_PLATFORM_WINDOWS
}

void GLFW_Window::get_window_size(uint32 &width, uint32 &height) const
{
    width = _width;
    height = _height;
}

constexpr float deadzone = 0.05f;

void GLFW_Window::_poll_analog_inputs()
{
    PROFILE_FUNCTION()

    for (int32 jid : connected_joysticks)
    {
        if (glfwJoystickIsGamepad(jid))
        {
            glfwGetGamepadState(jid, &_gamepad_state[jid]);

            for (int32 button = 0; button <= GLFW_GAMEPAD_BUTTON_LAST; ++button)
            {
                const uint8 state = _gamepad_state[jid].buttons[button];
                const uint8 previous_state = _previous_gamepad_state[jid].buttons[button];

                if (state != previous_state)
                {
                    joystick_input_source.broadcast(glfw_gamepad_button_to_name_id(button),
                        state == GLFW_PRESS ? 1.0f : 0.0f);
                }
            }

            for (int32 axis = 0; axis <= GLFW_GAMEPAD_AXIS_LAST; ++axis)
            {
                const float state = axis_deadzone(_gamepad_state[jid].axes[axis], deadzone);
                const float previous_state = axis_deadzone(_previous_gamepad_state[jid].axes[axis], deadzone);

                joystick_input_source.broadcast(glfw_gamepad_axis_to_name_id(axis), state);
            }

            _previous_gamepad_state[jid] = _gamepad_state[jid];
        }
    }
}