#include "Scene.h"
#include <glm/gtc/matrix_transform.hpp>

namespace SockEngine {

Scene::Scene(const std::string& name)
    : m_Name(name), m_Camera(glm::vec3(0.0f, 1.0f, 5.0f))
{
    m_ShadowMapShader = std::make_unique<Shader>("../Shaders/ShadowMap.vert", "../Shaders/ShadowMap.frag");
    m_LightingShader = std::make_unique<Shader>("../Shaders/Lighting.vert", "../Shaders/Lighting.frag");
}

Scene::~Scene() {
}

void Scene::OnUpdate(float deltaTime) {
    // Update scene objects, animations, etc.
}

void Scene::Render(Renderer& renderer) {
    // First render pass: shadow mapping
    renderer.BeginShadowPass(glm::vec3(-0.2f, -1.0f, -0.3f), 50000.0f);
    
    m_ShadowMapShader->use();
    m_ShadowMapShader->setMat4("lightSpaceMatrix", renderer.GetLightSpaceMatrix());
    
    for (const auto& object : m_Objects) {
        // Do a simplified render pass just for shadow mapping
        m_ShadowMapShader->setMat4("model", object.GetTransform());
        object.model->Draw(*m_ShadowMapShader);
    }
    
    renderer.EndShadowPass();
    
    // Second render pass: main scene rendering
    renderer.BeginScene(m_Camera);
    
    m_LightingShader->use();
    m_LightingShader->setVec3("viewPos", m_Camera.Position);
    m_LightingShader->setFloat("material.shininess", 32.0f);
    
    for (const auto& object : m_Objects) {
        renderer.RenderModel(*object.model, object.GetTransform(), *m_LightingShader);
    }
    
    renderer.EndScene();
}

void Scene::SetSkybox(const std::vector<std::string>& skyboxFaces) {
    m_SkyboxFaces = skyboxFaces;
    m_HasSkybox = true;
}

void Scene::AddModel(const std::string& filepath, const glm::vec3& position, const glm::vec3& scale) {
    SceneObject object;
    object.model = std::make_unique<Model>(filepath);
    object.position = position;
    object.scale = scale;
    m_Objects.push_back(std::move(object));
}

}