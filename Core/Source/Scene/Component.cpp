#include "Component.h"
#include <glm/gtc/matrix_transform.hpp>

namespace SockEngine {

glm::mat4 TransformComponent::GetLocalModelMatrix() const {
    if (localMatrixDirty) {
        // Use GLM functions to build TRS matrix
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), localPosition);
        glm::mat4 rotationMatrix = glm::mat4_cast(localRotation);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), localScale);
        
        localModelMatrix = translationMatrix * rotationMatrix * scaleMatrix;
        localMatrixDirty = false;
    }
    
    return localModelMatrix;
}

glm::mat4 TransformComponent::GetWorldModelMatrix(const entt::registry& registry) const {
    if (worldMatrixDirty) {
        // Check if the entity has a relationship component
        if (owner != entt::null && registry.valid(owner) && registry.all_of<RelationshipComponent>(owner)) {
            auto& relationship = registry.get<RelationshipComponent>(owner);
            
            // If the entity has a parent, multiply by the parent's world matrix
            if (relationship.parent != entt::null && registry.valid(relationship.parent)) {
                auto& parentTransform = registry.get<TransformComponent>(relationship.parent);
                worldModelMatrix = parentTransform.GetWorldModelMatrix(registry) * GetLocalModelMatrix();
            } else {
                worldModelMatrix = GetLocalModelMatrix();
            }
        } else {
            worldModelMatrix = GetLocalModelMatrix();
        }
        
        worldMatrixDirty = false;
    }
    
    return worldModelMatrix;
}

glm::vec3 TransformComponent::GetForward() const {
    glm::vec3 forward(0.0f, 0.0f, -1.0f);
    return glm::normalize(localRotation * forward);
}

glm::vec3 TransformComponent::GetRight() const {
    glm::vec3 right(1.0f, 0.0f, 0.0f);
    return glm::normalize(localRotation * right);
}

glm::vec3 TransformComponent::GetUp() const {
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    return glm::normalize(localRotation * up);
}

glm::vec3 TransformComponent::GetWorldPosition(const entt::registry& registry) const {
    return glm::vec3(GetWorldModelMatrix(registry)[3]);
}

glm::vec3 TransformComponent::GetWorldScale(const entt::registry& registry) const {
    glm::vec3 worldScale = localScale;
    
    // Check if the entity has a relationship component
    if (owner != entt::null && registry.valid(owner) && registry.all_of<RelationshipComponent>(owner)) {
        auto& relationship = registry.get<RelationshipComponent>(owner);
        
        // Traverse up the hierarchy to combine scales
        entt::entity parentEntity = relationship.parent;
        while (parentEntity != entt::null && registry.valid(parentEntity)) {
            auto& parentTransform = registry.get<TransformComponent>(parentEntity);
            worldScale.x *= parentTransform.localScale.x;
            worldScale.y *= parentTransform.localScale.y;
            worldScale.z *= parentTransform.localScale.z;
            
            // Move up to the parent's parent
            if (registry.all_of<RelationshipComponent>(parentEntity)) {
                parentEntity = registry.get<RelationshipComponent>(parentEntity).parent;
            } else {
                break;
            }
        }
    }
    
    return worldScale;
}

glm::quat TransformComponent::GetWorldRotation(const entt::registry& registry) const {
    glm::quat worldRotation = localRotation;
    
    // Check if the entity has a relationship component
    if (owner != entt::null && registry.valid(owner) && registry.all_of<RelationshipComponent>(owner)) {
        auto& relationship = registry.get<RelationshipComponent>(owner);
        
        // Traverse up the hierarchy to combine rotations
        entt::entity parentEntity = relationship.parent;
        while (parentEntity != entt::null && registry.valid(parentEntity)) {
            auto& parentTransform = registry.get<TransformComponent>(parentEntity);
            worldRotation = parentTransform.localRotation * worldRotation;
            
            // Move up to the parent's parent
            if (registry.all_of<RelationshipComponent>(parentEntity)) {
                parentEntity = registry.get<RelationshipComponent>(parentEntity).parent;
            } else {
                break;
            }
        }
    }
    
    return worldRotation;
}

}