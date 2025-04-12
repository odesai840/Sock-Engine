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
    
    // Mark transform as dirty
    if (HasComponent<TransformComponent>()) {
        auto& transform = GetComponent<TransformComponent>();
        transform.worldMatrixDirty = true;
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
