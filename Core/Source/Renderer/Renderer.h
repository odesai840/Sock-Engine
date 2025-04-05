#ifndef RENDERER_H
#define RENDERER_H

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Resources/Shader.h"
#include "Resources/Model.h"
#include "Camera/Camera.h"

namespace SockEngine {

class Renderer {
public:
    Renderer();
    ~Renderer();

    void Initialize(uint32_t viewportWidth, uint32_t viewportHeight);
    void Shutdown();
    
    void BeginScene(Camera& camera);
    void EndScene();
    
    void RenderModel(Model& model, const glm::mat4& transform, Shader& shader);
    void RenderSkybox(const std::vector<std::string>& skyboxFaces);

    void SetViewportSize(uint32_t width, uint32_t height);
    void EnableDebugNormals(bool enable) { m_DebugNormals = enable; }
    void EnableDebugSpecular(bool enable) { m_DebugSpecular = enable; }
    
    // Shadow mapping
    void SetupShadowMap(unsigned int width, unsigned int height);
    void BeginShadowPass(const glm::vec3& lightDir, float lightDistance);
    void EndShadowPass();
    unsigned int GetShadowMap() const { return m_DepthMap; }
    glm::mat4 GetLightSpaceMatrix() const { return m_LightSpaceMatrix; }

    // Framebuffer
    void CreateFramebuffer();
    void BindFramebuffer();
    void UnbindFramebuffer();
    void RescaleFramebuffer(uint32_t width, uint32_t height);
    unsigned int GetFramebufferTexture() const { return m_TextureID; }

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
    unsigned int m_ShadowWidth = 4096, m_ShadowHeight = 4096;
    unsigned int m_DepthMapFBO;
    unsigned int m_DepthMap;
    float m_ShadowBias = 0.00011f;
    float m_NearPlane = 0.1f;
    float m_FarPlane = 100000.0f;
    float m_OrthoSize = 3000.0f;
    
    // Skybox
    unsigned int m_SkyboxVAO, m_SkyboxVBO;
    unsigned int m_CubemapTexture;
    Shader* m_SkyboxShader;
    
    // Framebuffer
    unsigned int m_ViewportFBO; 
    unsigned int m_ViewportRBO;
    unsigned int m_TextureID;

    void SetupSkybox();
};

}

#endif