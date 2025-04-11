#include "Registry.h"

namespace SockEngine {

entt::entity Registry::CreateEntity(const std::string& name) {
    entt::entity entity = m_Registry.create();
    SetName(entity, name);
    return entity;
}

void Registry::DestroyEntity(entt::entity entity) {
    if (!IsValid(entity)) return;

    // Remove from name maps
    auto nameIt = m_EntityNames.find(entity);
    if (nameIt != m_EntityNames.end()) {
        m_NameToEntity.erase(nameIt->second);
        m_EntityNames.erase(nameIt);
    }

    // Destroy the entity in EnTT
    m_Registry.destroy(entity);
}

bool Registry::IsValid(entt::entity entity) const {
    return m_Registry.valid(entity);
}

void Registry::SetName(entt::entity entity, const std::string& name) {
    if (!IsValid(entity)) return;

    // Remove old name mapping if it exists
    auto it = m_EntityNames.find(entity);
    if (it != m_EntityNames.end()) {
        m_NameToEntity.erase(it->second);
    }

    // Add new name mapping
    m_EntityNames[entity] = name;
    m_NameToEntity[name] = entity;
}

const std::string& Registry::GetName(entt::entity entity) const {
    static const std::string empty = "";
    auto it = m_EntityNames.find(entity);
    return it != m_EntityNames.end() ? it->second : empty;
}

entt::entity Registry::FindEntityByName(const std::string& name) const {
    auto it = m_NameToEntity.find(name);
    return it != m_NameToEntity.end() ? it->second : entt::null;
}

}