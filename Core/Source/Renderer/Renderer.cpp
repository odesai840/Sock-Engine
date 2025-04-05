#include "Renderer.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

namespace SockEngine {

Renderer::Renderer()
    : m_ViewportWidth(1920), m_ViewportHeight(1080),
      m_DirectionalLightDir(glm::vec3(-0.2f, -1.0f, -0.3f))
{
    m_SkyboxShader = new Shader("../Shaders/Skybox.vert", "../Shaders/Skybox.frag");
}

Renderer::~Renderer() {
    delete m_SkyboxShader;
}

void Renderer::Initialize(uint32_t viewportWidth, uint32_t viewportHeight) {
    m_ViewportWidth = viewportWidth;
    m_ViewportHeight = viewportHeight;
    
    // Enable OpenGL states
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    
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
    glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
    
    // Store view and projection matrices
    m_ViewMatrix = camera.GetViewMatrix();
    m_ProjectionMatrix = glm::perspective(glm::radians(camera.Zoom), 
                                          (float)m_ViewportWidth / (float)m_ViewportHeight, 
                                          0.1f, 50000.0f);
}

void Renderer::EndScene() {
    // If not in debug mode, render skybox
    if (!m_DebugNormals && !m_DebugSpecular) {
        // Disable culling
        glDisable(GL_CULL_FACE);
        
        // Draw skybox
        glDepthFunc(GL_LEQUAL); // Change depth function so depth test passes when values are equal to depth buffer's content
        m_SkyboxShader->use();
        glm::mat4 view = glm::mat4(glm::mat3(m_ViewMatrix)); // Remove translation from the view matrix
        m_SkyboxShader->setMat4("view", view);
        m_SkyboxShader->setMat4("projection", m_ProjectionMatrix);
        
        // Skybox cube
        glBindVertexArray(m_SkyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // Set depth function back to default
        
        // Re-enable culling
        glEnable(GL_CULL_FACE);
    }
    
    UnbindFramebuffer();
}

void Renderer::RenderModel(Model& model, const glm::mat4& transform, Shader& shader) {
    shader.use();
    
    // Set common uniforms
    shader.setBool("debugNormals", m_DebugNormals);
    shader.setBool("debugSpec", m_DebugSpecular);
    
    // Set lighting parameters
    shader.setVec3("dirLight.direction", m_DirectionalLightDir);
    shader.setVec3("dirLight.ambient", 0.1f, 0.1f, 0.1f);
    shader.setVec3("dirLight.diffuse", 1.0f, 1.0f, 1.0f);
    shader.setVec3("dirLight.specular", 0.3f, 0.3f, 0.3f);
    
    // Set shadow mapping uniforms
    shader.setMat4("lightSpaceMatrix", m_LightSpaceMatrix);
    shader.setFloat("shadowBias", m_ShadowBias);
    
    // Bind shadow map
    glActiveTexture(GL_TEXTURE0 + 5);
    glBindTexture(GL_TEXTURE_2D, m_DepthMap);
    shader.setInt("shadowMap", 5);
    
    // Set view/projection matrices
    shader.setMat4("projection", m_ProjectionMatrix);
    shader.setMat4("view", m_ViewMatrix);
    
    // Set model transform
    shader.setMat4("model", transform);
    
    // Draw the model
    model.Draw(shader);
}

void Renderer::RenderSkybox(const std::vector<std::string>& skyboxFaces) {
    if (m_CubemapTexture != 0) {
        glDeleteTextures(1, &m_CubemapTexture);
    }
    m_CubemapTexture = Model::loadCubemap(skyboxFaces);
}

void Renderer::SetViewportSize(uint32_t width, uint32_t height) {
    m_ViewportWidth = width;
    m_ViewportHeight = height;
    RescaleFramebuffer(width, height);
}

}