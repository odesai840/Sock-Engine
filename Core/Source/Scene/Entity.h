#ifndef ENTITY_H
#define ENTITY_H

#include "Registry.h"
#include "Component.h"
#include <string>

namespace SockEngine {

class Entity {
public:
    Entity() = default;
    Entity(entt::entity handle, Registry* registry);
    Entity(const Entity& other) = default;

    // Component management
    template<typename T, typename... Args>
    T& AddComponent(Args&&... args);

    template<typename T>
    T& GetComponent() const;

    template<typename T>
    bool HasComponent() const;

    template<typename T>
    void RemoveComponent();

    // Hierarchy management
    Entity GetParent() const;
    void SetParent(Entity parent);
    std::vector<Entity> GetChildren() const;
    void MarkChildrenWorldMatrixDirty();

    // Utility methods
    bool IsValid() const;
    const std::string& GetName() const;
    void SetName(const std::string& name);

    // Operators
    operator bool() const { return IsValid(); }
    operator entt::entity() const { return m_EntityHandle; }
    bool operator==(const Entity& other) const {
        return m_EntityHandle == other.m_EntityHandle && m_Registry == other.m_Registry;
    }
    bool operator!=(const Entity& other) const { return !(*this == other); }

private:
    entt::entity m_EntityHandle = entt::null;
    Registry* m_Registry = nullptr;

    friend class Scene;
};

// Template implementations
template<typename T, typename... Args>
T& Entity::AddComponent(Args&&... args) {
    return m_Registry->GetRegistry().emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
}

template<typename T>
T& Entity::GetComponent() const {
    return m_Registry->GetRegistry().get<T>(m_EntityHandle);
}

template<typename T>
bool Entity::HasComponent() const {
    return m_Registry->GetRegistry().all_of<T>(m_EntityHandle);
}

template<typename T>
void Entity::RemoveComponent() {
    m_Registry->GetRegistry().remove<T>(m_EntityHandle);
}

}

#endif