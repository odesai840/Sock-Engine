#include "Application.h"
#include "Events.h"
#include <iostream>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

namespace SockEngine {

Application* Application::s_Instance = nullptr;

Application::Application(const std::string& name) {
    s_Instance = this;
    
    // Create window
    m_Window = std::make_unique<Window>(name, 1920, 1080);
    m_Window->SetEventCallback([this](void* event) {
        Event* e = static_cast<Event*>(event);
        
        // Handle different event types
        switch (e->GetType()) {
            case EventType::WindowResize:
            {
                WindowResizeEvent* resizeEvent = static_cast<WindowResizeEvent*>(e);
                glfwSetWindowSize(m_Window->GetNativeWindow(), static_cast<int>(resizeEvent->GetWidth()), static_cast<int>(resizeEvent->GetHeight()));
                break;
            }
            case EventType::WindowClose:
            {
                m_Running = false;
                break;
            }
            case EventType::KeyPressed:
            {
                KeyPressedEvent* keyEvent = static_cast<KeyPressedEvent*>(e);
                m_Input.UpdateKeyState(keyEvent->GetKeyCode(), GLFW_PRESS);
                break;
            }
            case EventType::KeyReleased:
            {
                KeyReleasedEvent* keyEvent = static_cast<KeyReleasedEvent*>(e);
                m_Input.UpdateKeyState(keyEvent->GetKeyCode(), GLFW_RELEASE);
                break;
            }
            case EventType::KeyRepeat:
            {
                KeyRepeatEvent* keyEvent = static_cast<KeyRepeatEvent*>(e);
                m_Input.UpdateKeyState(keyEvent->GetKeyCode(), GLFW_REPEAT);
                break;
            }
            case EventType::MouseButtonPressed:
            {
                MouseButtonPressedEvent* mouseEvent = static_cast<MouseButtonPressedEvent*>(e);
                m_Input.UpdateMouseButtonState(mouseEvent->GetMouseButton(), GLFW_PRESS);
                break;
            }
            case EventType::MouseButtonReleased:
            {
                MouseButtonReleasedEvent* mouseEvent = static_cast<MouseButtonReleasedEvent*>(e);
                m_Input.UpdateMouseButtonState(mouseEvent->GetMouseButton(), GLFW_RELEASE);
                break;
            }
            case EventType::MouseButtonRepeat:
            {
                MouseButtonRepeatEvent* mouseEvent = static_cast<MouseButtonRepeatEvent*>(e);
                m_Input.UpdateMouseButtonState(mouseEvent->GetMouseButton(), GLFW_REPEAT);
                break;
            }
            case EventType::MouseMoved:
            {
                MouseMovedEvent* mouseEvent = static_cast<MouseMovedEvent*>(e);
                m_Input.UpdateMousePosition(mouseEvent->GetX(), mouseEvent->GetY());
                break;
            }
            case EventType::MouseScrolled:
            {
                MouseScrolledEvent* mouseEvent = static_cast<MouseScrolledEvent*>(e);
                m_Input.UpdateMouseScroll(mouseEvent->GetXOffset(), mouseEvent->GetYOffset());
                break;
            }
        }
    });

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Set ImGui style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup ImGui backend
    ImGui_ImplGlfw_InitForOpenGL(m_Window->GetNativeWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

Application::~Application() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Application::Run() {
    while (m_Running) {
        // Calculate delta time
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - m_LastFrameTime;
        m_LastFrameTime = currentTime;

        m_FPS = 1.0f / deltaTime;
        m_FrameTime = deltaTime * 1000.0f;

        // Poll for and process events
        m_Window->PollEvents();

        // Check if window should close
        if (glfwWindowShouldClose(m_Window->GetNativeWindow())) {
            m_Running = false;
            continue; // Don't render this frame
        }

        // Update
        OnUpdate(deltaTime);

        // Render
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        OnRender();
        OnImGuiRender();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        // Reset input deltas at the end of each frame
        m_Input.ResetDeltas();

        // Swap buffers
        m_Window->SwapBuffers();
    }
}

}