#ifndef APPLICATION_H
#define APPLICATION_H

#include "Window.h"
#include "Input/Input.h"
#include <memory>

namespace SockEngine {

class Application {
public:
    Application(const std::string& name);
    virtual ~Application();

    void Run();
    virtual void OnUpdate(float deltaTime) = 0;
    virtual void OnRender() = 0;
    virtual void OnImGuiRender() = 0;

    Window& GetWindow() { return *m_Window; }
    bool IsRunning() const { return m_Running; }
    void Close() { m_Running = false; }

    static Application& Get() { return *s_Instance; }

protected:
    std::unique_ptr<Window> m_Window;
    bool m_Running = true;
    float m_LastFrameTime = 0.0f;
    Input m_Input;

    // Performance tracking
    float m_FPS = 0.0f;
    float m_FrameTime = 0.0f;

private:
    static Application* s_Instance;
};

}

#endif