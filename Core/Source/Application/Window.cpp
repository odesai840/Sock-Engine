#include "Window.h"
#include "Events.h"
#include <iostream>
#include <SOIL2/SOIL2.h>

namespace SockEngine {

static bool s_GLFWInitialized = false;

static void GLFWErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

Window::Window(const std::string& title, uint32_t width, uint32_t height) {
    Init(title, width, height);
}

Window::~Window() {
    Shutdown();
}

void Window::Init(const std::string& title, uint32_t width, uint32_t height) {
    m_Data.Title = title;
    m_Data.Width = width;
    m_Data.Height = height;

    if (!s_GLFWInitialized) {
        // Initialize GLFW
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW!" << std::endl;
            return;
        }

        glfwSetErrorCallback(GLFWErrorCallback);
        s_GLFWInitialized = true;
    }

    // Set GLFW window hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef _DEBUG
    glfwWindowHint(GLFW_CONTEXT_DEBUG, true);
#endif

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Set monitor info
    //GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    //const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    //glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    //glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    //glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    //glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    // Create window
    m_Window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_Window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        return;
    }

    glfwMakeContextCurrent(m_Window);
    glfwMaximizeWindow(m_Window);
    SetVSync(true);

    // Set window icon
    GLFWimage images[1];
    images[0].pixels = SOIL_load_image("../Assets/Branding/sockenginelogo.png", &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(m_Window, 1, images);
    SOIL_free_image_data(images[0].pixels);

    // Set GLFW callbacks
    glfwSetWindowUserPointer(m_Window, &m_Data);

    // Window resize callback
    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int newWidth, int newHeight) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        data.Width = newWidth;
        data.Height = newHeight;

        WindowResizeEvent event(newWidth, newHeight);
        data.EventCallback(&event);
    });

    // Window close callback
    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        WindowCloseEvent event;
        data.EventCallback(&event);
    });

    // Key callback
    glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        
        switch (action) {
            case GLFW_PRESS:
            {
                KeyPressedEvent event(key);
                data.EventCallback(&event);
                break;
            }
            case GLFW_RELEASE:
            {
                KeyReleasedEvent event(key);
                data.EventCallback(&event);
                break;
            }
            case GLFW_REPEAT:
            {
                KeyRepeatEvent event(key);
                data.EventCallback(&event);
                break;
            }
        }
    });

    // Mouse button callback
    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

        switch (action) {
            case GLFW_PRESS:
            {
                MouseButtonPressedEvent event(button);
                data.EventCallback(&event);
                break;
            }
            case GLFW_RELEASE:
            {
                MouseButtonReleasedEvent event(button);
                data.EventCallback(&event);
                break;
            }
            case GLFW_REPEAT:
                MouseButtonRepeatEvent event(button);
                data.EventCallback(&event);
                break;
        }
    });

    // Mouse scroll callback
    glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        
        MouseScrolledEvent event((float)xOffset, (float)yOffset);
        data.EventCallback(&event);
    });

    // Cursor position callback
    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        
        MouseMovedEvent event((float)xpos, (float)ypos);
        data.EventCallback(&event);
    });

    // Initialize GLAD
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return;
    }
}

void Window::Shutdown() {
    glfwDestroyWindow(m_Window);
}

void Window::PollEvents() {
    glfwPollEvents();
}

void Window::SwapBuffers() {
    glfwSwapBuffers(m_Window);
}

void Window::SetVSync(bool enabled) {
    glfwSwapInterval(enabled ? 1 : 0);
    m_Data.VSync = enabled;
}

bool Window::IsVSync() const {
    return m_Data.VSync;
}

void Window::SetMouseCursorVisible(bool visible) {
    m_Data.CursorVisible = visible;
    glfwSetInputMode(m_Window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
}

void Window::SetMouseCursorLocked(bool locked) {
    m_Data.CursorLocked = locked;
    
    // Lock/unlock cursor
    glfwSetInputMode(m_Window, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

}