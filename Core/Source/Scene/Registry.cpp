#include "Registry.h"
#include <regex>

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

std::string Registry::MakeNameUnique(const std::string& desiredName, entt::entity entityToExclude) {
    // If the name is not already used, it's already unique
    auto it = m_NameToEntity.find(desiredName);
    if (it == m_NameToEntity.end() || it->second == entityToExclude) {
        return desiredName;
    }

    // Try to extract base name and number if it already has a numbered suffix like "Name (1)"
    std::regex pattern(R"(^(.+) \((\d+)\)$)");
    std::smatch matches;
    std::string baseName = desiredName;
    int startNumber = 1;

    if (std::regex_match(desiredName, matches, pattern) && matches.size() == 3) {
        baseName = matches[1].str();
        startNumber = std::stoi(matches[2].str()) + 1;
    }

    // Find the next available number
    std::string candidateName;
    int suffix = startNumber;
    do {
        candidateName = baseName + " (" + std::to_string(suffix) + ")";
        it = m_NameToEntity.find(candidateName);
        suffix++;
    } while (it != m_NameToEntity.end() && it->second != entityToExclude);

    return candidateName;
}

void Registry::SetName(entt::entity entity, const std::string& name) {
    if (!IsValid(entity)) return;

    // Get the current name if any
    std::string oldName;
    auto it = m_EntityNames.find(entity);
    if (it != m_EntityNames.end()) {
        oldName = it->second;
    }

    // Make sure the new name is unique
    std::string uniqueName = MakeNameUnique(name, entity);

    // Remove old name mapping
    if (!oldName.empty()) {
        m_NameToEntity.erase(oldName);
    }

    // Add new name mapping
    m_EntityNames[entity] = uniqueName;
    m_NameToEntity[uniqueName] = entity;
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