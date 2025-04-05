#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>
#include <memory>
#include "Resources/Model.h"
#include "Camera/Camera.h"
#include "Renderer/Renderer.h"

namespace SockEngine {

class Scene {
public:
    Scene(const std::string& name);
    ~Scene();

    void OnUpdate(float deltaTime);
    void Render(Renderer& renderer);

    void SetSkybox(const std::vector<std::string>& skyboxFaces);
    
    // Camera access
    Camera& GetCamera() { return m_Camera; }
    
    // Scene manipulation
    void AddModel(const std::string& filepath, const glm::vec3& position = glm::vec3(0.0f), 
                 const glm::vec3& scale = glm::vec3(1.0f));

private:
    std::string m_Name;
    Camera m_Camera;
    
    struct SceneObject {
        std::unique_ptr<Model> model;
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;
        
        glm::mat4 GetTransform() const {
            glm::mat4 transform = glm::mat4(1.0f);
            transform = glm::translate(transform, position);
            // Apply rotation (would need to handle properly in a full implementation)
            transform = glm::scale(transform, scale);
            return transform;
        }
    };
    
    std::vector<SceneObject> m_Objects;
    std::vector<std::string> m_SkyboxFaces;
    bool m_HasSkybox = false;

    Shader* m_ShadowMapShader;
    Shader* m_LightingShader;
};

}

#endif