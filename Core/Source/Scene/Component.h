#ifndef COMPONENT_H
#define COMPONENT_H

#include <entt/entt.hpp>
#include "Resources/Model.h"
#include "Resources/Animation.h"
#include "Resources/AnimData.h"
#include <string>
#include <memory>
#include <vector>
#include <map>
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

// Animator component for skeletal animation
struct AnimatorComponent {
    // Animation data
    std::shared_ptr<Animation> currentAnimation;
    std::unique_ptr<Animator> animator;
    std::map<std::string, std::shared_ptr<Animation>> animations; // Named animations
    
    // Bone information extracted from model
    BoneInfoMap boneInfoMap;
    
    // Playback state
    bool isPlaying = true;
    bool isLooping = true;
    float playbackSpeed = 1.0f;
    float currentTime = 0.0f;
    std::string currentAnimationName = "";
    
    // Animation file paths for editor
    std::vector<std::string> animationPaths;
    int selectedAnimationIndex = 0;
    
    // Constructor
    AnimatorComponent() = default;
    
    // Initialize with a model and animation
    void Initialize(std::shared_ptr<Model> model, const std::string& animationPath);
    
    // Load additional animations
    void LoadAnimation(const std::string& name, const std::string& path);
    
    // Playback controls
    void Play();
    void Pause();
    void Stop();
    void SetLooping(bool loop);
    void SetPlaybackSpeed(float speed);
    
    // Animation switching
    void PlayAnimation(const std::string& animationName);
    bool HasAnimation(const std::string& name) const;
    
    // Update method (called each frame)
    void Update(float deltaTime);
    
    // Get bone matrices for rendering
    std::vector<glm::mat4> GetBoneMatrices() const;
    
    // Get animation info
    float GetDuration() const;
    float GetCurrentTime() const;
    bool IsPlaying() const { return isPlaying; }
    bool IsLooping() const { return isLooping; }
    float GetPlaybackSpeed() const { return playbackSpeed; }
    
private:
    void UpdateAnimator(float deltaTime);
    void ExtractBoneInfoFromModel(std::shared_ptr<Model> model);
};

}

#endif