#include "Entity.h"
#include "Component.h"

namespace SockEngine {

Entity::Entity(entt::entity handle, Registry* registry)
    : m_EntityHandle(handle), m_Registry(registry)
{
}

bool Entity::IsValid() const {
    return m_Registry && m_EntityHandle != entt::null && 
            m_Registry->IsValid(m_EntityHandle);
}

const std::string& Entity::GetName() const {
    if (!IsValid()) {
        static const std::string empty = "";
        return empty;
    }
    return m_Registry->GetName(m_EntityHandle);
}

void Entity::SetName(const std::string& name) {
    if (IsValid()) {
        m_Registry->SetName(m_EntityHandle, name);
    }
}

Entity Entity::GetParent() const {
    if (!IsValid() || !HasComponent<RelationshipComponent>())
        return Entity();
        
    auto& relationship = GetComponent<RelationshipComponent>();
    if (relationship.parent == entt::null)
        return Entity();
        
    return Entity(relationship.parent, m_Registry);
}

void Entity::SetParent(Entity parent) {
    if (!IsValid())
        return;

    // Store the current world transform before changing parent
    glm::mat4 worldTransform = glm::mat4(1.0f);
    bool hasTransform = HasComponent<TransformComponent>();
    
    if (hasTransform) {
        auto& transform = GetComponent<TransformComponent>();
        worldTransform = transform.GetWorldModelMatrix(m_Registry->GetRegistry());
    }
    
    // Get or create relationship component
    if (!HasComponent<RelationshipComponent>())
        AddComponent<RelationshipComponent>();
    
    auto& relationship = GetComponent<RelationshipComponent>();
    
    // Remove from old parent's children list
    Entity oldParent = GetParent();
    if (oldParent && oldParent.HasComponent<RelationshipComponent>()) {
        auto& oldParentRelationship = oldParent.GetComponent<RelationshipComponent>();
        auto it = std::find(oldParentRelationship.children.begin(), 
                           oldParentRelationship.children.end(), 
                           m_EntityHandle);
        if (it != oldParentRelationship.children.end()) {
            oldParentRelationship.children.erase(it);
        }
    }
    
    // Set new parent
    relationship.parent = parent ? parent.m_EntityHandle : entt::null;
    
    // Add to new parent's children list
    if (parent && parent.IsValid()) {
        if (!parent.HasComponent<RelationshipComponent>())
            parent.AddComponent<RelationshipComponent>();
            
        auto& parentRelationship = parent.GetComponent<RelationshipComponent>();
        parentRelationship.children.push_back(m_EntityHandle);
    }

    // Recalculate local transform to preserve world position
    if (hasTransform) {
        auto& transform = GetComponent<TransformComponent>();
        
        if (parent && parent.HasComponent<TransformComponent>()) {
            // Get the parent's world transform
            auto& parentTransform = parent.GetComponent<TransformComponent>();
            glm::mat4 parentWorldMatrix = parentTransform.GetWorldModelMatrix(m_Registry->GetRegistry());
            
            // Calculate the inverse of parent's world transform
            glm::mat4 inverseParentMatrix = glm::inverse(parentWorldMatrix);
            
            // Calculate new local matrix relative to parent
            glm::mat4 newLocalMatrix = inverseParentMatrix * worldTransform;
            
            // Extract position, rotation, and scale from the local matrix
            transform.localPosition = glm::vec3(newLocalMatrix[3]);
            
            // Extract scale - length of each basis vector
            glm::vec3 xAxis(newLocalMatrix[0].x, newLocalMatrix[0].y, newLocalMatrix[0].z);
            glm::vec3 yAxis(newLocalMatrix[1].x, newLocalMatrix[1].y, newLocalMatrix[1].z);
            glm::vec3 zAxis(newLocalMatrix[2].x, newLocalMatrix[2].y, newLocalMatrix[2].z);
            
            transform.localScale = glm::vec3(
                glm::length(xAxis),
                glm::length(yAxis),
                glm::length(zAxis)
            );
            
            // Create rotation matrix by normalizing basis vectors
            glm::mat3 rotationMatrix(
                xAxis / transform.localScale.x,
                yAxis / transform.localScale.y,
                zAxis / transform.localScale.z
            );
            
            // Convert to quaternion
            transform.localRotation = glm::quat_cast(rotationMatrix);
            
            // Update rotation degrees for the editor
            transform.localRotationDegrees = glm::degrees(glm::eulerAngles(transform.localRotation));
        } else {
            // Extract position, rotation, and scale directly from world matrix
            transform.localPosition = glm::vec3(worldTransform[3]);
            
            // Extract scale - length of each basis vector
            glm::vec3 xAxis(worldTransform[0].x, worldTransform[0].y, worldTransform[0].z);
            glm::vec3 yAxis(worldTransform[1].x, worldTransform[1].y, worldTransform[1].z);
            glm::vec3 zAxis(worldTransform[2].x, worldTransform[2].y, worldTransform[2].z);
            
            transform.localScale = glm::vec3(
                glm::length(xAxis),
                glm::length(yAxis),
                glm::length(zAxis)
            );
            
            // Create rotation matrix by normalizing basis vectors
            glm::mat3 rotationMatrix(
                xAxis / transform.localScale.x,
                yAxis / transform.localScale.y,
                zAxis / transform.localScale.z
            );
            
            // Convert to quaternion
            transform.localRotation = glm::quat_cast(rotationMatrix);
            
            // Update rotation degrees for the editor
            transform.localRotationDegrees = glm::degrees(glm::eulerAngles(transform.localRotation));
        }

        // Fix any possible precision issues by clamping very small values to exact zero
        for (int i = 0; i < 3; i++) {
            if (std::abs(transform.localRotationDegrees[i]) < 0.0001f) {
                transform.localRotationDegrees[i] = 0.0f;
            }
        }
        
        transform.localMatrixDirty = true;
        transform.worldMatrixDirty = true;
        
        // Mark all children's world matrices as dirty
        MarkChildrenWorldMatrixDirty();
    }
}

std::vector<Entity> Entity::GetChildren() const {
    std::vector<Entity> children;
    
    if (!IsValid() || !HasComponent<RelationshipComponent>())
        return children;
        
    auto& relationship = GetComponent<RelationshipComponent>();
    
    for (auto childHandle : relationship.children) {
        children.emplace_back(childHandle, m_Registry);
    }
    
    return children;
}

void Entity::MarkChildrenWorldMatrixDirty() {
    if (!IsValid() || !HasComponent<RelationshipComponent>())
        return;
        
    auto& relationship = GetComponent<RelationshipComponent>();
    
    for (auto childHandle : relationship.children) {
        Entity childEntity(childHandle, m_Registry);
        if (childEntity.IsValid() && childEntity.HasComponent<TransformComponent>()) {
            auto& childTransform = childEntity.GetComponent<TransformComponent>();
            childTransform.worldMatrixDirty = true;
            
            // Recursively mark all children's world matrices as dirty
            childEntity.MarkChildrenWorldMatrixDirty();
        }
    }
}

}
