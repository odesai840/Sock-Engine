#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include <functional>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace SockEngine {

class Window {
public:
    using EventCallbackFn = std::function<void(void*)>;

    Window(const std::string& title, uint32_t width, uint32_t height);
    ~Window();
    
    void PollEvents();
    void SwapBuffers();

    uint32_t GetWidth() const { return m_Data.Width; }
    uint32_t GetHeight() const { return m_Data.Height; }
    GLFWwindow* GetNativeWindow() const { return m_Window; }

    void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }

    void SetVSync(bool enabled);
    bool IsVSync() const;

    // Mouse cursor control
    void SetMouseCursorVisible(bool visible);
    void SetMouseCursorLocked(bool locked);
    bool IsMouseCursorVisible() const { return m_Data.CursorVisible; }
    bool IsMouseCursorLocked() const { return m_Data.CursorLocked; }
    bool IsFocused() const { return m_Data.Focused; }

private:
    void Init(const std::string& title, uint32_t width, uint32_t height);
    void Shutdown();

    GLFWwindow* m_Window;

    struct WindowData {
        std::string Title;
        uint32_t Width, Height;
        bool VSync;
        bool CursorVisible = true;
        bool CursorLocked = false;
        bool Focused = true;
        EventCallbackFn EventCallback;
    };

    WindowData m_Data;
};

}

#endif