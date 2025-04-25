#ifndef SCENE_H
#define SCENE_H

#include "Registry.h"
#include "Entity.h"
#include "Camera/Camera.h"
#include "Renderer/Renderer.h"
#include <vector>
#include <string>
#include <memory>

namespace SockEngine {

class Scene {
public:
    Scene(const std::string& name);
    ~Scene();

    void OnUpdate(float deltaTime);
    void Render(Renderer& renderer);

    // Camera access
    Camera& GetCamera() { return m_EditorCamera; }

    // Entity management
    Entity CreateEntity(const std::string& name);
    Entity CreateEntity(const std::string& name, Entity parent);
    Entity DuplicateEntity(Entity entity);
    void DestroyEntity(Entity entity);

    // Model loading
    Entity LoadModel(const std::string& filepath, const glm::vec3& position = glm::vec3(0.0f), 
                     const glm::vec3& scale = glm::vec3(1.0f));

    // Entity queries
    Entity FindEntityByName(const std::string& name);
    std::vector<Entity> GetRootEntities();

    // Scene info
    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }
    Entity GetRootEntity() const { return m_RootEntity; }

    // Registry access
    Registry& GetRegistry() { return m_Registry; }

    // Selection support for editor
    void SetSelectedEntity(Entity entity) { m_SelectedEntity = entity; }
    Entity GetSelectedEntity() const { return m_SelectedEntity; }

    // Hierarchy management
    void UpdateRelationship(Entity child, Entity parent);

private:
    std::string m_Name;
    Camera m_EditorCamera;

    // Entity registry
    Registry m_Registry;

    // Root entity of the scene
    Entity m_RootEntity;

    // Shaders
    std::unique_ptr<Shader> m_ShadowMapShader;
    std::unique_ptr<Shader> m_LightingShader;

    // Editor state
    Entity m_SelectedEntity;

    // Hierarchy management
    Entity DuplicateEntityHierarchy(Entity entity, Entity parent);

    // Utility function to check if setting a new parent would create a cycle
    bool WouldCreateCycle(Entity child, Entity newParent);
};

}

#endif