#include "EditorApplication.h"
#include <iostream>
#include <imgui/imgui.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL2/SOIL2.h>

namespace SockEngine {

EditorApplication::EditorApplication()
    : Application("Sock Engine")
{
    // Initialize renderer
    m_Renderer = std::make_unique<Renderer>();
    m_Renderer->Initialize(1920, 1080);
    
    // Create a default scene
    m_ActiveScene = std::make_unique<Scene>("New Scene");
    
    // Set skybox
    std::vector<std::string> skyboxFaces {
        "../Assets/Textures/SkyboxDay/right.bmp",
        "../Assets/Textures/SkyboxDay/left.bmp",
        "../Assets/Textures/SkyboxDay/top.bmp",
        "../Assets/Textures/SkyboxDay/bottom.bmp",
        "../Assets/Textures/SkyboxDay/front.bmp",
        "../Assets/Textures/SkyboxDay/back.bmp"
    };
    // Load skybox textures
    m_Renderer->LoadSkybox(skyboxFaces);
    // Set skybox texture paths for the scene
    m_ActiveScene->SetSkybox(skyboxFaces);
    m_SkyboxEnabled = m_Renderer->IsSkyboxEnabled();
    
    // Add a model to the scene
    m_ActiveScene->AddModel("../Assets/Models/sponza/sponza.obj");
}

EditorApplication::~EditorApplication() {
    if (m_Renderer) {
        m_Renderer->Shutdown();
    }
}

void EditorApplication::OnUpdate(float deltaTime) {
    // Handle right mouse button state
    static bool wasRightMouseButtonDown = false;
    bool isRightMouseButtonDown = m_Input.GetMouseButtonHeld(GLFW_MOUSE_BUTTON_RIGHT);
    
    // Handle input for camera when viewport is focused
    if (m_ViewportFocused) {
        Camera& camera = m_ActiveScene->GetCamera();
        
        // Right mouse button state changes
        if (isRightMouseButtonDown && !wasRightMouseButtonDown) {
            // Just pressed down - lock cursor
            GetWindow().SetMouseCursorLocked(true);
            // Reset any unexpected delta that might occur
            m_Input.ResetDeltas();
        }
        else if (!isRightMouseButtonDown && wasRightMouseButtonDown) {
            // Just released - unlock cursor
            GetWindow().SetMouseCursorLocked(false);
        }

        // Editor camera movement and rotation
        if (isRightMouseButtonDown) {
            // Keyboard input for movement
            if (m_Input.GetKeyHeld(GLFW_KEY_W)) {
                camera.ProcessKeyboard(FORWARD, deltaTime);
            }
            if (m_Input.GetKeyHeld(GLFW_KEY_A)) {
                camera.ProcessKeyboard(LEFT, deltaTime);
            }
            if (m_Input.GetKeyHeld(GLFW_KEY_S)) {
                camera.ProcessKeyboard(BACKWARD, deltaTime);
            }
            if (m_Input.GetKeyHeld(GLFW_KEY_D)) {
                camera.ProcessKeyboard(RIGHT, deltaTime);
            }

            // Camera rotation with mouse
            glm::vec2 mouseDelta = m_Input.GetMouseDelta();
            
            // Only process significant motion
            if (glm::length(mouseDelta) > 0.01f) {
                float sensitivity = 0.1f; // Adjust as needed
                camera.ProcessMouseMovement(mouseDelta.x * sensitivity, -mouseDelta.y * sensitivity);
            }
        }
        
        // Camera zoom with scroll wheel
        glm::vec2 scrollDelta = m_Input.GetMouseScroll();
        if (scrollDelta.y != 0.0f) {
            camera.ProcessMouseScroll(scrollDelta.y);
        }
    }
    
    // Update right mouse button state for next frame
    wasRightMouseButtonDown = isRightMouseButtonDown;
    
    // Update scene
    m_ActiveScene->OnUpdate(deltaTime);
}

void EditorApplication::OnRender() {
    // Apply debug settings to renderer
    m_Renderer->EnableDebugNormals(m_DebugNormals);
    m_Renderer->EnableDebugSpecular(m_DebugSpecular);
    
    // Render scene
    m_ActiveScene->Render(*m_Renderer);
}

void EditorApplication::OnImGuiRender() {
    // Set up the dockspace
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0, 0, 0, 0));
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;
    
    ImGui::Begin("Main Dockspace", nullptr, window_flags);
    ImGui::PopStyleColor(1);
    ImGuiID dockspace_id = ImGui::GetID("Main Dockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    DrawMenuBar();
    ImGui::End();

    // Draw the About window if the flag is set
    if (m_ShowAboutWindow) {
        DrawAboutWindow();
    }
    
    // Draw editor windows
    DrawViewport();
    DrawSceneHierarchy();
    DrawInspector();
    DrawDebugPanel();
    DrawOutputLog();
}

void EditorApplication::DrawMenuBar() {
    if (ImGui::BeginMenuBar()) {
        std::vector<Menu> menus = {
            { "File", {
                { "New Scene", "CTRL+N", []() {} },
                { "Open Scene", "CTRL+O", []() {} },
                { "Save Scene", "CTRL+S", []() {} },
                { "Save Scene As...", "CTRL+ALT+S", []() {} },
                { "Save All", "CTRL+SHIFT+S", []() {} },
                { "Exit", nullptr, [this]() { Close(); } }
            }},
            { "Edit", {
                { "Undo", "CTRL+Z", []() {} },
                { "Redo", "CTRL+Y", []() {} },
                { "Cut", "CTRL+X", []() {} },
                { "Copy", "CTRL+C", []() {} },
                { "Paste", "CTRL+V", []() {} },
                { "Settings", nullptr, []() {} }
            }},
            { "View", {
                { "Scene Hierarchy", nullptr, []() {} },
                { "Inspector", nullptr, []() {} },
                { "Viewport", nullptr, []() {} }
            }},
            { "Help", {
                { "About", nullptr, [this]() { ShowAboutWindow(); } },
            }}
        };

        // Render all menus
        for (const auto& menu : menus) {
            if (ImGui::BeginMenu(menu.name)) {
                for (const auto& item : menu.items) {
                    if (ImGui::MenuItem(item.name, item.shortcut)) {
                        item.action();
                    }
                }
                ImGui::EndMenu();
            }
        }
        
        ImGui::EndMenuBar();
    }
}

void EditorApplication::ShowAboutWindow() {
    // Set show_about_window flag to true
    m_ShowAboutWindow = true;
}

void EditorApplication::DrawAboutWindow() {
    // Calculate center position for the window
    ImVec2 center = ImVec2(ImGui::GetIO().DisplaySize.x / 2.0f, ImGui::GetIO().DisplaySize.y / 2.0f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

    // Load logo
    GLuint logo = 0;
    ImVec2 logoSize = ImVec2(0, 0);
    int width, height, channels;
    unsigned char* data = SOIL_load_image("../Assets/Branding/sockenginelogo.png", &width, &height, &channels, 0);
    if (data)
    {
        glGenTextures(1, &logo);
        glBindTexture(GL_TEXTURE_2D, logo);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        logoSize = ImVec2((float)width, (float)height);
        SOIL_free_image_data(data);
    }

    float displayHeight = 400.0f;
    float aspectRatio = logoSize.x / logoSize.y;
    ImVec2 displaySize(displayHeight * aspectRatio, displayHeight);
    
    // Create the window
    ImGui::Begin("##", &m_ShowAboutWindow, window_flags);
    ImGui::PopStyleVar(1);
    ImGui::Image((void*)(intptr_t)logo, displaySize);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Sock Engine");
    ImGui::Text("Version 0.1.0");
    ImGui::Separator();
    ImGui::Text("© 2025 Sock Games");
    ImGui::End();
    
}

void EditorApplication::DrawViewport() {
    ImGui::Begin("Viewport");
    
    // Store viewport focus/hover state
    m_ViewportFocused = ImGui::IsWindowFocused();
    m_ViewportHovered = ImGui::IsWindowHovered();
    
    // Get the size of the viewport
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    
    // Resize the renderer's viewport if needed
    m_Renderer->SetViewportSize(static_cast<uint32_t>(viewportSize.x), static_cast<uint32_t>(viewportSize.y));
    
    // Display the rendered scene in the viewport
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddImage(
        (void*)m_Renderer->GetFramebufferTexture(),
        ImVec2(pos.x, pos.y),
        ImVec2(pos.x + viewportSize.x, pos.y + viewportSize.y),
        ImVec2(0, 1),
        ImVec2(1, 0)
    );
    
    ImGui::End();
}

void EditorApplication::DrawSceneHierarchy() {
    ImGui::Begin("Scene Hierarchy");
    
    // In a full implementation, this would display a tree view of scene objects
    if (ImGui::TreeNode(m_ActiveScene->GetName().c_str())) {
        ImGui::TreePop();
    }
    
    ImGui::End();
}

void EditorApplication::DrawInspector() {
    ImGui::Begin("Inspector");
    
    // In a full implementation, this would display properties of the selected object
    ImGui::Text("No object selected");
    
    ImGui::End();
}

void EditorApplication::DrawDebugPanel() {
    ImGui::Begin("Debug");
    
    ImGui::Text("FPS: %.1f", m_FPS);
    ImGui::Text("Frame Time: %.3f ms", m_FrameTime);
    
    ImGui::Spacing();
    
    // VSync
    bool enabled = m_Window->IsVSync();
    if (ImGui::Checkbox("VSync", &enabled)) {
        m_Window->SetVSync(enabled);
    }
    
    ImGui::Separator();
    
    // Debug visualization options
    if (ImGui::CollapsingHeader("Debug Visualizations", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Debug Normals", &m_DebugNormals);
        if (m_DebugNormals) {
            m_DebugSpecular = false;
        }
    
        ImGui::Checkbox("Debug Specular", &m_DebugSpecular);
        if (m_DebugSpecular) {
            m_DebugNormals = false;
        }
        
        if (ImGui::Checkbox("Enable Skybox", &m_SkyboxEnabled)) {
            m_Renderer->EnableSkybox(m_SkyboxEnabled);
        }
    }
    
    ImGui::Separator();
    
    // Camera settings
    Camera& camera = m_ActiveScene->GetCamera();
    
    if (ImGui::CollapsingHeader("Camera Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Movement Speed", &camera.MovementSpeed, 100.0f, 8000.0f, "%.1f");
    }

    ImGui::Separator();
    
    // Input debug info
    if (ImGui::CollapsingHeader("Input Debug", ImGuiTreeNodeFlags_DefaultOpen)) {
        // Mouse position and delta
        glm::vec2 mousePos = m_Input.GetMousePosition();
        glm::vec2 mouseDelta = m_Input.GetMouseDelta();
        ImGui::Text("Mouse Position: %.1f, %.1f", mousePos.x, mousePos.y);
        ImGui::Text("Mouse Delta: %.1f, %.1f", mouseDelta.x, mouseDelta.y);
    
        // Mouse buttons state
        ImGui::Text("Right Mouse Button: %s", 
                    m_Input.GetMouseButtonHeld(GLFW_MOUSE_BUTTON_RIGHT) ? "Down" : "Up");
    
        // Cursor lock state
        ImGui::Text("Cursor Locked: %s", GetWindow().IsMouseCursorLocked() ? "Yes" : "No");
    }
    
    ImGui::End();
}

void EditorApplication::DrawOutputLog() {
    ImGui::Begin("Output Log");
    // In a full implementation, this would display engine logs
    ImGui::End();
}

}
