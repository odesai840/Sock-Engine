#ifndef REGISTRY_H
#define REGISTRY_H

#include <entt/entt.hpp>
#include <string>
#include <unordered_map>

namespace SockEngine {

class Registry {
public:
    Registry() = default;
    ~Registry() = default;

    // Entity management
    entt::entity CreateEntity(const std::string& name);
    void DestroyEntity(entt::entity entity);
    bool IsValid(entt::entity entity) const;

    // Get the underlying EnTT registry
    entt::registry& GetRegistry() { return m_Registry; }
    const entt::registry& GetRegistry() const { return m_Registry; }

    // Name management
    void SetName(entt::entity entity, const std::string& name);
    const std::string& GetName(entt::entity entity) const;

    // Utility functions
    entt::entity FindEntityByName(const std::string& name) const;

private:
    entt::registry m_Registry;

    // Storage for entity names
    std::unordered_map<entt::entity, std::string> m_EntityNames;
    std::unordered_map<std::string, entt::entity> m_NameToEntity;
};

}

#endif