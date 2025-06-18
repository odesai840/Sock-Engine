#ifndef COMPONENT_H
#define COMPONENT_H

#include <entt/entt.hpp>
#include "Resources/Model.h"
#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace SockEngine {

// Tag component to indicate an entity is active/inactive
struct ActiveComponent {
    bool active = true;
};

// Parent-child relationship component
struct RelationshipComponent {
    entt::entity parent = entt::null;
    std::vector<entt::entity> children;
};

// Transform component
struct TransformComponent {
    glm::vec3 localPosition = glm::vec3(0.0f);
    glm::vec3 localScale = glm::vec3(1.0f);
    glm::quat localRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 localRotationDegrees = glm::vec3(0.0f);

    mutable glm::mat4 localModelMatrix = glm::mat4(1.0f);
    mutable glm::mat4 worldModelMatrix = glm::mat4(1.0f);
    mutable bool localMatrixDirty = true;
    mutable bool worldMatrixDirty = true;

    // Entity that owns this component
    entt::entity owner = entt::null;

    // Utility methods
    glm::mat4 GetLocalModelMatrix() const;
    glm::mat4 GetWorldModelMatrix(const entt::registry& registry) const;

    // Directional vectors
    glm::vec3 GetForward() const;
    glm::vec3 GetRight() const;
    glm::vec3 GetUp() const;

    // World transform properties
    glm::vec3 GetWorldPosition(const entt::registry& registry) const;
    glm::vec3 GetWorldScale(const entt::registry& registry) const;
    glm::quat GetWorldRotation(const entt::registry& registry) const;
};

// Model component
struct ModelComponent {
    std::shared_ptr<Model> model;
    std::string modelPath;
    float shininess = 32.0f;
    bool castShadows = true;
    bool receiveShadows = true;
};

}

#endif