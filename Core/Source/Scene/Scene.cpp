#include "Scene.h"
#include "Component.h"
#include <memory>

namespace SockEngine {

Scene::Scene(const std::string& name)
    : m_Name(name), m_EditorCamera(glm::vec3(0.0f, 90.0f, 0.0f))
{
    m_ShadowMapShader = std::make_unique<Shader>("../Shaders/ShadowMap.vert", "../Shaders/ShadowMap.frag");
    m_LightingShader = std::make_unique<Shader>("../Shaders/Lighting.vert", "../Shaders/Lighting.frag");
}

Scene::~Scene() {
    // The EnTT registry automatically cleans up all entities and components
}

void Scene::OnUpdate(float deltaTime) {
    // Update all entities with TransformComponent and ActiveComponent
    auto view = m_Registry.GetRegistry().view<TransformComponent, ActiveComponent>();
    for (auto entity : view) {
        auto& active = view.get<ActiveComponent>(entity);
        
        if (active.active) {
            auto& transform = view.get<TransformComponent>(entity);
            // Perform any per-entity updates here
        }
    }
}

void Scene::Render(Renderer& renderer) {
    // First render pass: shadow mapping
    renderer.BeginShadowPass(glm::vec3(-0.2f, -1.0f, -0.3f), 50000.0f);
    
    m_ShadowMapShader->Use();
    m_ShadowMapShader->SetMat4("lightSpaceMatrix", renderer.GetLightSpaceMatrix());
    
    // Render all entities with model components that cast shadows
    auto shadowView = m_Registry.GetRegistry().view<TransformComponent, ModelComponent, ActiveComponent>();
    for (auto entity : shadowView) {
        auto& active = shadowView.get<ActiveComponent>(entity);
        auto& model = shadowView.get<ModelComponent>(entity);
        
        if (active.active && model.castShadows && model.model) {
            auto& transform = shadowView.get<TransformComponent>(entity);
            
            // Set model matrix
            m_ShadowMapShader->SetMat4("model", transform.GetWorldModelMatrix(m_Registry.GetRegistry()));
            
            // Draw the model
            model.model->Draw(*m_ShadowMapShader);
        }
    }
    
    renderer.EndShadowPass();
    
    // Second render pass: main scene rendering
    renderer.BeginScene(m_EditorCamera);
    
    m_LightingShader->Use();
    m_LightingShader->SetVec3("viewPos", m_EditorCamera.Position);
    
    // Render all entities with model components
    auto renderView = m_Registry.GetRegistry().view<TransformComponent, ModelComponent, ActiveComponent>();
    for (auto entity : renderView) {
        auto& active = renderView.get<ActiveComponent>(entity);
        auto& model = renderView.get<ModelComponent>(entity);
        
        if (active.active && model.model) {
            auto& transform = renderView.get<TransformComponent>(entity);
            
            // Set material properties
            m_LightingShader->SetFloat("material.shininess", model.shininess);
            
            // Render the model
            renderer.RenderModel(*model.model, transform.GetWorldModelMatrix(m_Registry.GetRegistry()), *m_LightingShader);
        }
    }
    
    renderer.EndScene();
}

void Scene::SetSkybox(const std::vector<std::string>& skyboxFaces) {
    m_SkyboxFaces = skyboxFaces;
    m_HasSkybox = true;
}

Entity Scene::CreateEntity(const std::string& name) {
    // Generate a unique name
    std::string uniqueName = name;
    
    // Check if this name is already used
    if (m_Registry.FindEntityByName(uniqueName) != entt::null) {
        // Name already exists, need to generate a unique version
        int suffix = 1;
        do {
            uniqueName = name + " (" + std::to_string(suffix) + ")";
            suffix++;
        } while (m_Registry.FindEntityByName(uniqueName) != entt::null);
    }
    
    // Create the entity with the unique name
    entt::entity entityHandle = m_Registry.CreateEntity(uniqueName);
    
    // Add default components
    m_Registry.GetRegistry().emplace<TransformComponent>(entityHandle);
    m_Registry.GetRegistry().emplace<ActiveComponent>(entityHandle);
    m_Registry.GetRegistry().emplace<RelationshipComponent>(entityHandle);
    
    return Entity(entityHandle, &m_Registry);
}

Entity Scene::DuplicateEntity(Entity entity) {
    if (!entity) {
        return Entity();
    }
    
    // Create duplicate with the same parent
    Entity parent;
    if (entity.HasComponent<RelationshipComponent>()) {
        auto& relationship = entity.GetComponent<RelationshipComponent>();
        if (relationship.parent != entt::null) {
            parent = Entity(relationship.parent, &m_Registry);
        }
    }
    
    return DuplicateEntityHierarchy(entity, parent);
}

Entity Scene::DuplicateEntityHierarchy(Entity entity, Entity parent) {
    if (!entity) {
        return Entity();
    }
    
    // Get the original entity name
    std::string originalName = entity.GetName();
    std::string newName = originalName;

    // Add a numbered suffix for duplicate
    int highestNum = 0;
    std::string searchPattern = originalName + " (";
    
    // Find entities with similar names
    auto view = m_Registry.GetRegistry().view<ActiveComponent>();
    for (auto e : view) {
        Entity checkEntity(e, &m_Registry);
        const std::string& name = checkEntity.GetName();
        
        // Check if this entity starts with our original name plus a space and opening parenthesis
        if (name.length() > searchPattern.length() && 
            name.substr(0, searchPattern.length()) == searchPattern &&
            name.back() == ')') {
            
            // Extract the number from the suffix
            std::string numStr = name.substr(searchPattern.length(), name.length() - searchPattern.length() - 1);
            bool isNumber = !numStr.empty() && std::all_of(numStr.begin(), numStr.end(), ::isdigit);
            
            if (isNumber) {
                int num = std::stoi(numStr);
                highestNum = std::max(highestNum, num);
            }
        }
    }
    
    // Create the new name with sequential number suffix
    newName = originalName + " (" + std::to_string(highestNum + 1) + ")";
    
    // Create a new entity
    Entity newEntity = CreateEntity(newName);
    
    // Copy components from the original entity
    
    // Copy transform component
    if (entity.HasComponent<TransformComponent>() && newEntity.HasComponent<TransformComponent>()) {
        auto& srcTransform = entity.GetComponent<TransformComponent>();
        auto& dstTransform = newEntity.GetComponent<TransformComponent>();
        
        dstTransform.localPosition = srcTransform.localPosition;
        dstTransform.localRotation = srcTransform.localRotation;
        dstTransform.localScale = srcTransform.localScale;
        dstTransform.localRotationDegrees = srcTransform.localRotationDegrees;
        dstTransform.localMatrixDirty = true;
        dstTransform.worldMatrixDirty = true;
    }
    
    // Copy model component
    if (entity.HasComponent<ModelComponent>()) {
        auto& srcModel = entity.GetComponent<ModelComponent>();
        
        auto& dstModel = newEntity.HasComponent<ModelComponent>() ? 
                           newEntity.GetComponent<ModelComponent>() : 
                           newEntity.AddComponent<ModelComponent>();
        
        dstModel.shininess = srcModel.shininess;
        dstModel.castShadows = srcModel.castShadows;
        dstModel.receiveShadows = srcModel.receiveShadows;
        
        // Share the model resource
        dstModel.model = srcModel.model;
        dstModel.modelPath = srcModel.modelPath;
    }
    
    // Set the parent
    if (parent) {
        UpdateRelationship(newEntity, parent);
    }
    
    // Recursively duplicate all children
    if (entity.HasComponent<RelationshipComponent>()) {
        auto& relationship = entity.GetComponent<RelationshipComponent>();
        
        for (auto childHandle : relationship.children) {
            Entity child(childHandle, &m_Registry);
            if (child) {
                DuplicateEntityHierarchy(child, newEntity);
            }
        }
    }
    
    return newEntity;
}

void Scene::DestroyEntity(Entity entity) {
    if (!entity) {
        return;
    }
    
    // If this was the selected entity, clear selection
    if (m_SelectedEntity == entity) {
        m_SelectedEntity = Entity();
    }
    
    // Get all children before destroying the entity
    std::vector<Entity> children;
    if (entity.HasComponent<RelationshipComponent>()) {
        auto& relationship = entity.GetComponent<RelationshipComponent>();
        
        for (auto childHandle : relationship.children) {
            children.emplace_back(childHandle, &m_Registry);
        }
    }
    
    // Recursively destroy all children
    for (auto child : children) {
        DestroyEntity(child);
    }
    
    // Remove from parent's children list
    if (entity.HasComponent<RelationshipComponent>()) {
        auto& relationship = entity.GetComponent<RelationshipComponent>();
        
        if (relationship.parent != entt::null) {
            Entity parent(relationship.parent, &m_Registry);
            if (parent && parent.HasComponent<RelationshipComponent>()) {
                auto& parentRelationship = parent.GetComponent<RelationshipComponent>();
                
                // Remove from parent's children list
                auto it = std::find(parentRelationship.children.begin(), 
                                    parentRelationship.children.end(), 
                                    static_cast<entt::entity>(entity));
                                    
                if (it != parentRelationship.children.end()) {
                    parentRelationship.children.erase(it);
                }
            }
        }
    }
    
    // Destroy the entity
    m_Registry.DestroyEntity(static_cast<entt::entity>(entity));
}

Entity Scene::LoadModel(const std::string& filepath, const glm::vec3& position, const glm::vec3& scale) {
    // Create a new entity
    std::string name = filepath.substr(filepath.find_last_of("/\\") + 1);
    Entity entity = CreateEntity(name);
    
    // Set transform
    auto& transform = entity.GetComponent<TransformComponent>();
    transform.localPosition = position;
    transform.localScale = scale;
    transform.localMatrixDirty = true;
    transform.worldMatrixDirty = true;
    
    // Add a model component
    auto& modelComponent = entity.AddComponent<ModelComponent>();
    modelComponent.model = std::make_shared<Model>(filepath);
    modelComponent.modelPath = filepath;
    
    return entity;
}

Entity Scene::FindEntityByName(const std::string& name) {
    entt::entity entityHandle = m_Registry.FindEntityByName(name);
    if (entityHandle != entt::null) {
        return Entity(entityHandle, &m_Registry);
    }
    return Entity();
}

std::vector<Entity> Scene::GetRootEntities() {
    std::vector<Entity> rootEntities;
    
    // Get all entities with relationship components
    auto view = m_Registry.GetRegistry().view<RelationshipComponent>();
    
    for (auto entity : view) {
        auto& relationship = view.get<RelationshipComponent>(entity);
        
        // If entity has no parent, it's a root entity
        if (relationship.parent == entt::null) {
            rootEntities.emplace_back(entity, &m_Registry);
        }
    }
    
    // Reverse the order so that new entities appear at the bottom
    std::reverse(rootEntities.begin(), rootEntities.end());
    
    return rootEntities;
}

void Scene::UpdateRelationship(Entity child, Entity parent) {
    if (!child) {
        return;
    }
    
    // Get the child's current parent (if any)
    Entity oldParent;
    if (child.HasComponent<RelationshipComponent>()) {
        auto& childRelationship = child.GetComponent<RelationshipComponent>();
        
        if (childRelationship.parent != entt::null) {
            oldParent = Entity(childRelationship.parent, &m_Registry);
        }
    }
    
    // If old parent is the same as new parent, nothing to do
    if (oldParent == parent) {
        return;
    }
    
    // Remove child from old parent's children list
    if (oldParent && oldParent.HasComponent<RelationshipComponent>()) {
        auto& oldParentRelationship = oldParent.GetComponent<RelationshipComponent>();
        
        auto it = std::find(oldParentRelationship.children.begin(), 
                            oldParentRelationship.children.end(), 
                            static_cast<entt::entity>(child));
                            
        if (it != oldParentRelationship.children.end()) {
            oldParentRelationship.children.erase(it);
        }
    }
    
    // Update child's parent reference
    auto& childRelationship = child.GetComponent<RelationshipComponent>();
    childRelationship.parent = parent ? static_cast<entt::entity>(parent) : entt::null;
    
    // Add child to new parent's children list
    if (parent && parent.HasComponent<RelationshipComponent>()) {
        auto& parentRelationship = parent.GetComponent<RelationshipComponent>();
        
        // Only add if not already in the list
        auto it = std::find(parentRelationship.children.begin(), 
                            parentRelationship.children.end(), 
                            static_cast<entt::entity>(child));
                            
        if (it == parentRelationship.children.end()) {
            parentRelationship.children.push_back(static_cast<entt::entity>(child));
        }
    }
    
    // Mark transform matrices as dirty to update world transform
    if (child.HasComponent<TransformComponent>()) {
        auto& childTransform = child.GetComponent<TransformComponent>();
        childTransform.worldMatrixDirty = true;
    }
}

}