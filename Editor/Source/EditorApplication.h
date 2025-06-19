#ifndef EDITOR_APPLICATION_H
#define EDITOR_APPLICATION_H

#include "Application/Application.h"
#include "Renderer/Renderer.h"
#include "Scene/Scene.h"

namespace SockEngine {

class EditorApplication : public Application {
public:
    EditorApplication();
    ~EditorApplication() override;

    void OnUpdate(float deltaTime) override;
    void OnRender() override;
    void OnImGuiRender() override;

private:
    std::unique_ptr<Renderer> m_Renderer;
    std::unique_ptr<Scene> m_ActiveScene;

    // Editor state
    glm::vec2 m_ViewportMin = {0.0f, 0.0f};
    glm::vec2 m_ViewportMax = {0.0f, 0.0f};
    bool m_ViewportBoundsValid = false;
    bool m_ViewportFocused = false;
    bool m_ViewportHovered = false;

    // Editor debug options
    bool m_DebugNormals = false;
    bool m_DebugSpecular = false;
    bool m_SkyboxEnabled = false;

    // Entity scheduled for deletion
    Entity m_EntityToDelete;

    // Menu bar
    struct MenuItem {
        const char* name;
        const char* shortcut;
        std::function<void()> action;
    };

    struct Menu {
        const char* name;
        std::vector<MenuItem> items;
    };

    // Editor windows
    void ShowAboutWindow();
    void DrawAboutWindow();
    void DrawMenuBar();
    void DrawViewport();
    void DrawSceneHierarchy();
    void DrawEntityNode(Entity entity);
    void DrawComponents(Entity entity);
    void DrawTransformComponent(Entity entity);
    void DrawModelComponent(Entity entity);
    void DrawAddComponentPopup(Entity entity);
    void DrawInspector();
    void DrawDebugPanel();
    void DrawOutputLog();

    bool m_ShowAboutWindow = false;
};

}

#endif