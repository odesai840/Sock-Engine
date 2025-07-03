#include "Component.h"
#include <iostream>
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
                // Check if parent has a transform component, otherwise use identity matrix
                if (registry.all_of<TransformComponent>(relationship.parent)) {
                    auto& parentTransform = registry.get<TransformComponent>(relationship.parent);
                    worldModelMatrix = parentTransform.GetWorldModelMatrix(registry) * GetLocalModelMatrix();
                } else {
                    // Parent doesn't have a transform (likely the scene root), so use local matrix directly
                    worldModelMatrix = GetLocalModelMatrix();
                }
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
            // Check if parent has transform component
            if (registry.all_of<TransformComponent>(parentEntity)) {
                auto& parentTransform = registry.get<TransformComponent>(parentEntity);
                worldScale.x *= parentTransform.localScale.x;
                worldScale.y *= parentTransform.localScale.y;
                worldScale.z *= parentTransform.localScale.z;
            }
            
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
            // Check if parent has transform component
            if (registry.all_of<TransformComponent>(parentEntity)) {
                auto& parentTransform = registry.get<TransformComponent>(parentEntity);
                worldRotation = parentTransform.localRotation * worldRotation;
            }
            
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

void AnimatorComponent::Initialize(std::shared_ptr<Model> model, const std::string& animationPath) {
    if (!model) {
        std::cout << "ERROR: Cannot initialize AnimatorComponent without a valid model" << std::endl;
        return;
    }
    
    try {
        // Extract bone information from the model
        ExtractBoneInfoFromModel(model);
        
        if (boneInfoMap.empty()) {
            std::cout << "WARNING: No bone information found in model. Model may not be rigged for animation." << std::endl;
        }
        
        // Load the animation using the extracted bone info
        currentAnimation = std::make_shared<Animation>(animationPath, boneInfoMap);
        
        // Create the animator
        animator = std::make_unique<Animator>(currentAnimation.get());
        
        // Store the animation in map
        std::string animationName = "Default";
        animations[animationName] = currentAnimation;
        currentAnimationName = animationName;
        
        // Store the path for editor
        animationPaths.push_back(animationPath);
    }
    catch (const std::exception& e) {
        std::cout << "ERROR: Failed to initialize AnimatorComponent: " << e.what() << std::endl;
    }
}

void AnimatorComponent::LoadAnimation(const std::string& name, const std::string& path) {
    if (boneInfoMap.empty()) {
        std::cout << "ERROR: Cannot load animation without bone information. Initialize with a model first." << std::endl;
        return;
    }
    
    try {
        auto animation = std::make_shared<Animation>(path, boneInfoMap);
        animations[name] = animation;
        animationPaths.push_back(path);
    }
    catch (const std::exception& e) {
        std::cout << "ERROR: Failed to load animation '" << name << "': " << e.what() << std::endl;
    }
}

void AnimatorComponent::Play() {
    if (animator && animator->m_HasEnded) {
        Stop();  // Reset everything to beginning
    }
    
    isPlaying = true;
    if (animator) {
        animator->m_HasEnded = false;  // Reset end state when manually playing
    }
}

void AnimatorComponent::Pause() {
    isPlaying = false;
}

void AnimatorComponent::Stop() {
    isPlaying = false;
    currentTime = 0.0f;
    if (animator) {
        animator->m_CurrentTime = 0.0f;
        animator->m_HasEnded = false;
    }
}

void AnimatorComponent::SetLooping(bool loop) {
    isLooping = loop;
}

void AnimatorComponent::SetPlaybackSpeed(float speed) {
    playbackSpeed = glm::max(0.0f, speed); // Ensure non-negative speed
}

void AnimatorComponent::PlayAnimation(const std::string& animationName) {
    auto it = animations.find(animationName);
    if (it != animations.end()) {
        currentAnimation = it->second;
        currentAnimationName = animationName;
        
        if (animator) {
            animator->PlayAnimation(currentAnimation.get());
        }
        
        currentTime = 0.0f;
        isPlaying = true;
    }
    else {
        std::cout << "WARNING: Animation '" << animationName << "' not found" << std::endl;
    }
}

bool AnimatorComponent::HasAnimation(const std::string& name) const {
    return animations.find(name) != animations.end();
}

void AnimatorComponent::Update(float deltaTime) {
    if (!isPlaying || !animator || !currentAnimation) {
        return;
    }
    
    // Apply playback speed
    float scaledDeltaTime = deltaTime * playbackSpeed;
    
    // Update the animator
    UpdateAnimator(scaledDeltaTime);
    
    // Handle time display and playback state
    if (isLooping) {
        // For looping animations, just use the animator's time
        currentTime = animator->m_CurrentTime;
    } else {
        // For non-looping animations, check if it has ended
        if (animator->m_HasEnded) {
            currentTime = GetDuration();  // Show full duration in UI
            isPlaying = false;            // Stop playback
        } else {
            currentTime = animator->m_CurrentTime;  // Show current time
        }
    }
}

std::vector<glm::mat4> AnimatorComponent::GetBoneMatrices() const {
    if (animator) {
        return animator->GetFinalBoneMatrices();
    }
    
    // Return identity matrices if no animator
    std::vector<glm::mat4> identityMatrices(100, glm::mat4(1.0f));
    return identityMatrices;
}

float AnimatorComponent::GetDuration() const {
    if (currentAnimation) {
        return currentAnimation->m_Duration;
    }
    return 0.0f;
}

float AnimatorComponent::GetCurrentTime() const {
    return currentTime;
}

void AnimatorComponent::UpdateAnimator(float deltaTime) {
    if (animator) {
        animator->UpdateAnimation(deltaTime, isLooping);
    }
}

void AnimatorComponent::ExtractBoneInfoFromModel(std::shared_ptr<Model> model) {
    if (!model) {
        return;
    }

    // Copy bone information from the model
    boneInfoMap = model->GetBoneInfoMap();
}

}