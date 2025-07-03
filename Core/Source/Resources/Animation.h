#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>
#include <map>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <assimp/scene.h>
#include "AnimData.h"

namespace SockEngine {

// Represents a keyframe for position
struct PositionKeyframe {
    glm::vec3 position;
    float timeStamp;
};

// Represents a keyframe for rotation
struct RotationKeyframe {
    glm::quat orientation;
    float timeStamp;
};

// Represents a keyframe for scaling
struct ScaleKeyframe {
    glm::vec3 scale;
    float timeStamp;
};

// Represents an animated bone/node
class Bone {
public:
    std::vector<PositionKeyframe> m_Positions;
    std::vector<RotationKeyframe> m_Rotations;
    std::vector<ScaleKeyframe> m_Scales;
    int m_NumPositions;
    int m_NumRotations;
    int m_NumScalings;

    glm::mat4 m_LocalTransform;
    std::string m_Name;
    int m_ID;

    Bone(const std::string& name, int ID, const aiNodeAnim* channel);

    // Interpolates between keyframes and updates m_LocalTransform
    void Update(float animationTime);

    // Get the current position interpolated between keyframes
    glm::mat4 InterpolatePosition(float animationTime);
    
    // Get the current rotation interpolated between keyframes
    glm::mat4 InterpolateRotation(float animationTime);
    
    // Get the current scale interpolated between keyframes
    glm::mat4 InterpolateScaling(float animationTime);

private:
    // Get the index of the position keyframe before the current time
    int GetPositionIndex(float animationTime);
    
    // Get the index of the rotation keyframe before the current time
    int GetRotationIndex(float animationTime);
    
    // Get the index of the scale keyframe before the current time
    int GetScaleIndex(float animationTime);

    // Calculate interpolation factor between keyframes
    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
};

// Helper structure to store node hierarchy
struct AssimpNodeData {
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

// Represents an animation sequence
class Animation {
public:
    float m_Duration;
    int m_TicksPerSecond;
    std::vector<Bone> m_Bones;
    AssimpNodeData m_RootNode;
    BoneInfoMap m_BoneInfoMap;

    Animation() = default;
    
    // Constructor that loads from file with provided bone info
    Animation(const std::string& animationPath, 
              const BoneInfoMap& boneInfoMap);

    // Find a bone in the animation by name
    Bone* FindBone(const std::string& name);

private:
    // Read keyframes from assimp animation
    void ReadBonesFromAnimation(const aiAnimation* animation, 
                               const BoneInfoMap& boneInfoMap);
    
    // Read hierarchy data from assimp node
    void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);
};

// Handles animation playback and bone matrix calculation
class Animator {
public:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    Animation* m_CurrentAnimation;
    float m_CurrentTime;
    float m_DeltaTime;
    bool m_HasEnded = false;

    Animator(Animation* animation);

    // Update animation and calculate bone matrices
    void UpdateAnimation(float dt, bool looping = true);

    // Play a specific animation
    void PlayAnimation(Animation* pAnimation);

    // Calculate bone transforms recursively
    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);

    // Reset method for when animation ends
    void ResetToFirstFrame();

    // Get the final bone matrices for shader upload
    std::vector<glm::mat4> GetFinalBoneMatrices() { return m_FinalBoneMatrices; }
};

}

#endif