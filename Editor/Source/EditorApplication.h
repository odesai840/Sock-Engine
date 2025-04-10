#ifndef EDITOR_APPLICATION_H
#define EDITOR_APPLICATION_H

#include <Application/Application.h>
#include <Renderer/Renderer.h>
#include <Scene/Scene.h>

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
    bool m_ViewportFocused = false;
    bool m_ViewportHovered = false;
    bool m_ShowAboutWindow = false;
    
    // Editor debug options
    bool m_DebugNormals = false;
    bool m_DebugSpecular = false;
    bool m_SkyboxEnabled = false;

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
    void DrawInspector();
    void DrawDebugPanel();
    void DrawOutputLog();
};

}

#endif