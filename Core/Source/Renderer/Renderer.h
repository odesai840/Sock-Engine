#ifndef RENDERER_H
#define RENDERER_H

#include "Resources/Shader.h"
#include "Resources/Model.h"
#include "Camera/Camera.h"
#include "Scene/Scene.h"
#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>

namespace SockEngine {

class Renderer {
public:
    Renderer();
    ~Renderer();

    void Initialize(uint32_t viewportWidth, uint32_t viewportHeight);
    void Shutdown();
    
    // Main rendering method
    void RenderScene(Scene& scene, Camera& camera);
    
    // Individual model rendering (for custom render passes)
    void RenderModel(Model& model, const glm::mat4& transform, Shader& shader);

    void SetViewportSize(uint32_t width, uint32_t height);
    void EnableDebugNormals(bool enable) { m_DebugNormals = enable; }
    void EnableDebugSpecular(bool enable) { m_DebugSpecular = enable; }

    // Skybox
    void LoadSkybox(const std::vector<std::string>& skyboxFaces);
    void EnableSkybox(bool enable) { m_EnableSkybox = enable; }
    bool IsSkyboxEnabled() const { return m_EnableSkybox; }
    static unsigned int LoadCubemap(std::vector<std::string> faces);
    
    // Shadow mapping
    void SetupShadowMap(unsigned int width, unsigned int height);
    unsigned int GetShadowMap() const { return m_DepthMap; }
    glm::mat4 GetLightSpaceMatrix() const { return m_LightSpaceMatrix; }

    // Framebuffer
    void CreateFramebuffer();
    void BindFramebuffer();
    void UnbindFramebuffer();
    void RescaleFramebuffer(uint32_t width, uint32_t height);
    unsigned int GetFramebufferTexture() const { return m_TextureID; }

    // Lighting settings
    void SetDirectionalLight(const glm::vec3& direction) { m_DirectionalLightDir = direction; }
    glm::vec3 GetDirectionalLight() const { return m_DirectionalLightDir; }

private:
    uint32_t m_ViewportWidth, m_ViewportHeight;
    bool m_DebugNormals = false;
    bool m_DebugSpecular = false;
    
    // Camera data
    glm::mat4 m_ViewMatrix;
    glm::mat4 m_ProjectionMatrix;
    
    // Lighting data
    glm::vec3 m_DirectionalLightDir;
    glm::mat4 m_LightSpaceMatrix;
    
    // Shadow mapping
    unsigned int m_ShadowWidth = 8192, m_ShadowHeight = 8192;
    unsigned int m_DepthMapFBO;
    unsigned int m_DepthMap;
    float m_ShadowBias = 0.00011f;
    float m_NearPlane = 0.1f;
    float m_FarPlane = 100000.0f;
    float m_OrthoSize = 3000.0f;
    
    // Skybox
    unsigned int m_SkyboxVAO, m_SkyboxVBO;
    unsigned int m_SkyboxTexture;
    std::unique_ptr<Shader> m_SkyboxShader;
    bool m_EnableSkybox = true;
    
    // Framebuffer
    unsigned int m_ViewportFBO; 
    unsigned int m_ViewportRBO;
    unsigned int m_TextureID;

    // Shaders
    std::unique_ptr<Shader> m_ShadowMapShader;
    std::unique_ptr<Shader> m_LightingShader;

    // Internal rendering methods
    void BeginScene(Camera& camera);
    void EndScene();
    void BeginShadowPass(const glm::vec3& lightDir, float lightDistance);
    void EndShadowPass();
    void SetupSkybox();
    
    // Scene data collection
    std::vector<Entity> CollectRenderableEntities(Scene& scene);
    void RenderShadowPass(const std::vector<Entity>& entities, Scene& scene);
    void RenderMainPass(const std::vector<Entity>& entities, Scene& scene, Camera& camera);
    void RenderSkybox();
};

}

#endif