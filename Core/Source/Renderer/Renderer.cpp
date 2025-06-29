#include "Renderer.h"
#include <iostream>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL2/SOIL2.h>

namespace SockEngine {

Renderer::Renderer()
    : m_DirectionalLightDir(glm::vec3(-0.2f, -1.0f, -0.3f))
{
    m_SkyboxShader = std::make_unique<Shader>("../Shaders/Skybox.vert", "../Shaders/Skybox.frag");
    m_ShadowMapShader = std::make_unique<Shader>("../Shaders/ShadowMap.vert", "../Shaders/ShadowMap.frag");
    m_LightingShader = std::make_unique<Shader>("../Shaders/Lighting.vert", "../Shaders/Lighting.frag");
}

Renderer::~Renderer() {
}

void Renderer::Initialize() {
    // Initialize viewport size to render size
    m_ViewportWidth = m_RenderWidth;
    m_ViewportHeight = m_RenderHeight;
    
    // Enable OpenGL states
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    // Initialize skybox
    SetupSkybox();
    
    // Setup framebuffer
    CreateFramebuffer();
    
    // Setup shadow mapping
    SetupShadowMap(m_ShadowWidth, m_ShadowHeight);
}

void Renderer::Shutdown() {
    glDeleteVertexArrays(1, &m_SkyboxVAO);
    glDeleteBuffers(1, &m_SkyboxVBO);
    
    // Delete framebuffers
    glDeleteFramebuffers(1, &m_ViewportFBO);
    glDeleteRenderbuffers(1, &m_ViewportRBO);
    glDeleteTextures(1, &m_TextureID);
    
    // Delete shadow mapping resources
    glDeleteFramebuffers(1, &m_DepthMapFBO);
    glDeleteTextures(1, &m_DepthMap);
}

void Renderer::RenderScene(Scene& scene, Camera& camera) {
    // Collect all renderable entities from the scene
    std::vector<Entity> renderableEntities = CollectRenderableEntities(scene);
    
    // First pass: Shadow mapping
    RenderShadowPass(renderableEntities, scene);
    
    // Second pass: Main rendering
    RenderMainPass(renderableEntities, scene, camera);
}

std::vector<Entity> Renderer::CollectRenderableEntities(Scene& scene) {
    std::vector<Entity> entities;
    
    auto& registry = scene.GetRegistry().GetRegistry();
    auto view = registry.view<TransformComponent, ModelComponent, ActiveComponent>();
    
    for (auto entityHandle : view) {
        auto& active = view.get<ActiveComponent>(entityHandle);
        auto& model = view.get<ModelComponent>(entityHandle);
        
        if (active.active && model.model) {
            Entity entity(entityHandle, &scene.GetRegistry());
            entities.push_back(entity);
        }
    }
    
    return entities;
}

void Renderer::RenderShadowPass(const std::vector<Entity>& entities, Scene& scene) {
    BeginShadowPass(m_DirectionalLightDir, 50000.0f);
    
    m_ShadowMapShader->Use();
    m_ShadowMapShader->SetMat4("lightSpaceMatrix", m_LightSpaceMatrix);
    
    auto& registry = scene.GetRegistry().GetRegistry();
    
    // Render all entities that cast shadows
    for (const auto& entity : entities) {
        if (entity.HasComponent<ModelComponent>()) {
            auto& modelComponent = entity.GetComponent<ModelComponent>();
            
            if (modelComponent.castShadows) {
                auto& transform = entity.GetComponent<TransformComponent>();
                glm::mat4 worldMatrix = transform.GetWorldModelMatrix(registry);
                
                m_ShadowMapShader->SetMat4("model", worldMatrix);
                modelComponent.model->Draw(*m_ShadowMapShader);
            }
        }
    }
    
    EndShadowPass();
}

void Renderer::RenderMainPass(const std::vector<Entity>& entities, Scene& scene, Camera& camera) {
    BeginScene(camera);
    
    m_LightingShader->Use();
    m_LightingShader->SetVec3("viewPos", camera.Position);
    
    // Set common uniforms
    m_LightingShader->SetBool("debugNormals", m_DebugNormals);
    m_LightingShader->SetBool("debugSpec", m_DebugSpecular);
    
    // Set lighting parameters
    m_LightingShader->SetVec3("dirLight.direction", m_DirectionalLightDir);
    m_LightingShader->SetVec3("dirLight.ambient", 0.1f, 0.1f, 0.1f);
    m_LightingShader->SetVec3("dirLight.diffuse", 1.0f, 1.0f, 1.0f);
    m_LightingShader->SetVec3("dirLight.specular", 0.3f, 0.3f, 0.3f);
    
    // Set shadow mapping uniforms
    m_LightingShader->SetMat4("lightSpaceMatrix", m_LightSpaceMatrix);
    m_LightingShader->SetFloat("shadowBias", m_ShadowBias);
    
    // Bind shadow map
    glActiveTexture(GL_TEXTURE0 + 5);
    glBindTexture(GL_TEXTURE_2D, m_DepthMap);
    m_LightingShader->SetInt("shadowMap", 5);
    
    // Set view/projection matrices
    m_LightingShader->SetMat4("projection", m_ProjectionMatrix);
    m_LightingShader->SetMat4("view", m_ViewMatrix);
    
    auto& registry = scene.GetRegistry().GetRegistry();
    
    // Render all entities
    for (const auto& entity : entities) {
        if (entity.HasComponent<ModelComponent>() && entity.HasComponent<TransformComponent>()) {
            auto& modelComponent = entity.GetComponent<ModelComponent>();
            auto& transform = entity.GetComponent<TransformComponent>();
            
            // Set material properties
            m_LightingShader->SetFloat("material.shininess", modelComponent.shininess);
            
            // Set model transform
            glm::mat4 worldMatrix = transform.GetWorldModelMatrix(registry);
            m_LightingShader->SetMat4("model", worldMatrix);
            
            // Draw the model
            modelComponent.model->Draw(*m_LightingShader);
        }
    }
    
    // Render skybox if enabled
    if (!m_DebugNormals && !m_DebugSpecular && m_EnableSkybox) {
        RenderSkybox();
    }
    
    EndScene();
}

void Renderer::RenderSkybox() {
    // Disable culling
    glDisable(GL_CULL_FACE);
    
    // Draw skybox
    glDepthFunc(GL_LEQUAL); // Change depth function so depth test passes when values are equal to depth buffer's content
    m_SkyboxShader->Use();
    glm::mat4 view = glm::mat4(glm::mat3(m_ViewMatrix)); // Remove translation from the view matrix
    m_SkyboxShader->SetMat4("view", view);
    m_SkyboxShader->SetMat4("projection", m_ProjectionMatrix);
    
    // Skybox cube
    glBindVertexArray(m_SkyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); // Set depth function back to default
    
    // Re-enable culling
    glEnable(GL_CULL_FACE);
}

void Renderer::SetupSkybox() {
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    // skybox VAO
    glGenVertexArrays(1, &m_SkyboxVAO);
    glGenBuffers(1, &m_SkyboxVBO);
    glBindVertexArray(m_SkyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_SkyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void Renderer::CreateFramebuffer() {
    glGenFramebuffers(1, &m_ViewportFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFBO);

    glGenTextures(1, &m_TextureID);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_ViewportWidth, m_ViewportHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureID, 0);

    glGenRenderbuffers(1, &m_ViewportRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_ViewportRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_ViewportWidth, m_ViewportHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_ViewportRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::BindFramebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFBO);
}

void Renderer::UnbindFramebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::RescaleFramebuffer(uint32_t width, uint32_t height) {
    m_ViewportWidth = width;
    m_ViewportHeight = height;
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFBO);

    glBindTexture(GL_TEXTURE_2D, m_TextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureID, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, m_ViewportRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_ViewportRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::SetupShadowMap(unsigned int width, unsigned int height) {
    m_ShadowWidth = width;
    m_ShadowHeight = height;
    
    // Generate framebuffer
    glGenFramebuffers(1, &m_DepthMapFBO);
    
    // Create depth texture
    glGenTextures(1, &m_DepthMap);
    glBindTexture(GL_TEXTURE_2D, m_DepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    // Attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::BeginShadowPass(const glm::vec3& lightDir, float lightDistance) {
    // Calculate light space matrix for directional light
    glm::mat4 lightProjection = glm::ortho(-m_OrthoSize, m_OrthoSize, -m_OrthoSize, m_OrthoSize, m_NearPlane, m_FarPlane);
    glm::mat4 lightView = glm::lookAt(-lightDir * lightDistance,
                           glm::vec3(0.0f),
                           glm::vec3(0.0f, 1.0f, 0.0f));
    m_LightSpaceMatrix = lightProjection * lightView;
    
    // Configure viewport to shadow map dimensions
    glViewport(0, 0, m_ShadowWidth, m_ShadowHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    // Adjust face culling
    glDisable(GL_CULL_FACE);
}

void Renderer::EndShadowPass() {
    // Reset face culling
    glEnable(GL_CULL_FACE);
    
    // Reset framebuffer and viewport
    glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFBO);
    glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
}

void Renderer::BeginScene(Camera& camera) {
    BindFramebuffer();
    
    // Clear the framebuffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set viewport
    glViewport(0, 0, m_RenderWidth, m_RenderHeight);
    
    // Store view and projection matrices
    m_ViewMatrix = camera.GetViewMatrix();
    m_ProjectionMatrix = glm::perspective(glm::radians(camera.Zoom), 
                                          (float)m_RenderWidth / (float)m_RenderHeight, 
                                          0.1f, 50000.0f);
}

void Renderer::EndScene() {
    UnbindFramebuffer();
}

void Renderer::RenderModel(Model& model, const glm::mat4& transform, Shader& shader) {
    shader.Use();
    
    // Set common uniforms
    shader.SetBool("debugNormals", m_DebugNormals);
    shader.SetBool("debugSpec", m_DebugSpecular);
    
    // Set lighting parameters
    shader.SetVec3("dirLight.direction", m_DirectionalLightDir);
    shader.SetVec3("dirLight.ambient", 0.1f, 0.1f, 0.1f);
    shader.SetVec3("dirLight.diffuse", 1.0f, 1.0f, 1.0f);
    shader.SetVec3("dirLight.specular", 0.3f, 0.3f, 0.3f);
    
    // Set shadow mapping uniforms
    shader.SetMat4("lightSpaceMatrix", m_LightSpaceMatrix);
    shader.SetFloat("shadowBias", m_ShadowBias);
    
    // Bind shadow map
    glActiveTexture(GL_TEXTURE0 + 5);
    glBindTexture(GL_TEXTURE_2D, m_DepthMap);
    shader.SetInt("shadowMap", 5);
    
    // Set view/projection matrices
    shader.SetMat4("projection", m_ProjectionMatrix);
    shader.SetMat4("view", m_ViewMatrix);
    
    // Set model transform
    shader.SetMat4("model", transform);
    
    // Draw the model
    model.Draw(shader);
}

void Renderer::LoadSkybox(const std::vector<std::string>& skyboxFaces) {
    if (m_SkyboxTexture != 0) {
        glDeleteTextures(1, &m_SkyboxTexture);
    }
    m_SkyboxTexture = LoadCubemap(skyboxFaces);
}

void Renderer::SetRenderResolution(uint32_t width, uint32_t height) {
    if (m_RenderWidth != width || m_RenderHeight != height) {
        m_RenderWidth = width;
        m_RenderHeight = height;
        
        RescaleFramebuffer(width, height);
    }
}

unsigned int Renderer::LoadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = SOIL_load_image(faces[i].c_str(), &width, &height, &nrComponents, 0);
        GLenum format = GL_RGB;
        if (data)
        {
            if (nrComponents == 1) {
                format = GL_RED;
            }
            else if (nrComponents == 2) {
                format = GL_RG;
            }
            else if (nrComponents == 3) {
                format = GL_RGB;
            }
            else if (nrComponents == 4) {
                format = GL_RGBA;
            }
            
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            SOIL_free_image_data(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    return textureID;
}

}
