#include "Animation.h"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace SockEngine {

Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel)
    : m_Name(name), m_ID(ID), m_LocalTransform(1.0f)
{
    m_NumPositions = channel->mNumPositionKeys;
    for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex) {
        aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
        float timeStamp = channel->mPositionKeys[positionIndex].mTime;
        PositionKeyframe data;
        data.position = glm::vec3(aiPosition.x, aiPosition.y, aiPosition.z);
        data.timeStamp = timeStamp;
        m_Positions.push_back(data);
    }

    m_NumRotations = channel->mNumRotationKeys;
    for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex) {
        aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
        float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
        RotationKeyframe data;
        data.orientation = glm::quat(aiOrientation.w, aiOrientation.x, aiOrientation.y, aiOrientation.z);
        data.timeStamp = timeStamp;
        m_Rotations.push_back(data);
    }

    m_NumScalings = channel->mNumScalingKeys;
    for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex) {
        aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
        float timeStamp = channel->mScalingKeys[keyIndex].mTime;
        ScaleKeyframe data;
        data.scale = glm::vec3(scale.x, scale.y, scale.z);
        data.timeStamp = timeStamp;
        m_Scales.push_back(data);
    }
}

void Bone::Update(float animationTime) {
    glm::mat4 translation = InterpolatePosition(animationTime);
    glm::mat4 rotation = InterpolateRotation(animationTime);
    glm::mat4 scale = InterpolateScaling(animationTime);
    m_LocalTransform = translation * rotation * scale;
}

glm::mat4 Bone::InterpolatePosition(float animationTime) {
    if (1 == m_NumPositions)
        return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

    int p0Index = GetPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp,
        m_Positions[p1Index].timeStamp, animationTime);
    glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position,
        m_Positions[p1Index].position, scaleFactor);
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Bone::InterpolateRotation(float animationTime) {
    if (1 == m_NumRotations) {
        auto rotation = glm::normalize(m_Rotations[0].orientation);
        return glm::mat4_cast(rotation);
    }

    int p0Index = GetRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp,
        m_Rotations[p1Index].timeStamp, animationTime);
    glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation,
        m_Rotations[p1Index].orientation, scaleFactor);
    finalRotation = glm::normalize(finalRotation);
    return glm::mat4_cast(finalRotation);
}

glm::mat4 Bone::InterpolateScaling(float animationTime) {
    if (1 == m_NumScalings)
        return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

    int p0Index = GetScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp,
        m_Scales[p1Index].timeStamp, animationTime);
    glm::vec3 finalScale = glm::mix(m_Scales[p0Index].scale,
        m_Scales[p1Index].scale, scaleFactor);
    return glm::scale(glm::mat4(1.0f), finalScale);
}

int Bone::GetPositionIndex(float animationTime) {
    for (int index = 0; index < m_NumPositions - 1; ++index) {
        if (animationTime < m_Positions[index + 1].timeStamp)
            return index;
    }
    return 0;
}

int Bone::GetRotationIndex(float animationTime) {
    for (int index = 0; index < m_NumRotations - 1; ++index) {
        if (animationTime < m_Rotations[index + 1].timeStamp)
            return index;
    }
    return 0;
}

int Bone::GetScaleIndex(float animationTime) {
    for (int index = 0; index < m_NumScalings - 1; ++index) {
        if (animationTime < m_Scales[index + 1].timeStamp)
            return index;
    }
    return 0;
}

float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) {
    float midWayLength = animationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    float scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

Animation::Animation(const std::string& animationPath, 
                    const BoneInfoMap& boneInfoMap) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
    
    if (!scene || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    
    if (scene->mNumAnimations == 0) {
        std::cout << "ERROR: No animations found in file: " << animationPath << std::endl;
        return;
    }
    
    auto animation = scene->mAnimations[0];
    m_Duration = animation->mDuration;
    m_TicksPerSecond = animation->mTicksPerSecond;
    
    ReadHierarchyData(m_RootNode, scene->mRootNode);
    ReadBonesFromAnimation(animation, boneInfoMap);
}

Bone* Animation::FindBone(const std::string& name) {
    auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
        [&](const Bone& bone) {
            return bone.m_Name == name;
        });
    if (iter == m_Bones.end()) return nullptr;
    else return &(*iter);
}

void Animation::ReadBonesFromAnimation(const aiAnimation* animation, 
                                      const BoneInfoMap& boneInfoMap) {
    int size = animation->mNumChannels;
    
    // Copy the provided bone info map
    m_BoneInfoMap = boneInfoMap;

    for (int i = 0; i < size; i++) {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        // Check if this bone exists in the provided bone info map
        auto it = m_BoneInfoMap.find(boneName);
        if (it != m_BoneInfoMap.end()) {
            m_Bones.push_back(Bone(channel->mNodeName.data, it->second.id, channel));
        } else {
            // Clean the bone name
            std::string cleanedName = boneName;
            
            // Remove Assimp suffixes
            if (cleanedName.find("_$AssimpFbx$_Rotation") != std::string::npos) {
                cleanedName = cleanedName.substr(0, cleanedName.find("_$AssimpFbx$_Rotation"));
            } else if (cleanedName.find("_$AssimpFbx$_Translation") != std::string::npos) {
                cleanedName = cleanedName.substr(0, cleanedName.find("_$AssimpFbx$_Translation"));
            } else if (cleanedName.find("_$AssimpFbx$_Scaling") != std::string::npos) {
                cleanedName = cleanedName.substr(0, cleanedName.find("_$AssimpFbx$_Scaling"));
            }
            
            // Try to find with cleaned name
            auto cleanIt = m_BoneInfoMap.find(cleanedName);
            if (cleanIt != m_BoneInfoMap.end()) {
                m_Bones.push_back(Bone(cleanedName, cleanIt->second.id, channel));
            } else {
                std::cout << "WARNING: Animation bone '" << boneName << "' (cleaned: '" << cleanedName << "') not found in model bone info" << std::endl;
            }
        }
    }
}

void Animation::ReadHierarchyData(AssimpNodeData& dest, const aiNode* src) {
    dest.name = src->mName.data;
    dest.transformation = glm::mat4(
        src->mTransformation.a1, src->mTransformation.b1, src->mTransformation.c1, src->mTransformation.d1,
        src->mTransformation.a2, src->mTransformation.b2, src->mTransformation.c2, src->mTransformation.d2,
        src->mTransformation.a3, src->mTransformation.b3, src->mTransformation.c3, src->mTransformation.d3,
        src->mTransformation.a4, src->mTransformation.b4, src->mTransformation.c4, src->mTransformation.d4
    );
    dest.childrenCount = src->mNumChildren;

    for (int i = 0; i < src->mNumChildren; i++) {
        AssimpNodeData newData;
        ReadHierarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}

Animator::Animator(Animation* animation) {
    m_CurrentTime = 0.0;
    m_CurrentAnimation = animation;
    m_FinalBoneMatrices.reserve(100);
    m_HasEnded = false;

    for (int i = 0; i < 100; i++)
        m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
}

void Animator::UpdateAnimation(float dt, bool looping) {
    m_DeltaTime = dt;
    if (m_CurrentAnimation) {
        if (!m_HasEnded) {
            m_CurrentTime += m_CurrentAnimation->m_TicksPerSecond * dt;
        }
        
        // Only wrap time if looping is enabled
        if (looping) {
            m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->m_Duration);
            m_HasEnded = false;  // Reset end state for looping animations
            CalculateBoneTransform(&m_CurrentAnimation->m_RootNode, glm::mat4(1.0f));
        } else {
            // Check if animation has ended
            if (m_CurrentTime >= m_CurrentAnimation->m_Duration && !m_HasEnded) {
                m_HasEnded = true;
                ResetToFirstFrame();
            } else if (!m_HasEnded) {
                CalculateBoneTransform(&m_CurrentAnimation->m_RootNode, glm::mat4(1.0f));
            }
        }
    }
}

void Animator::PlayAnimation(Animation* pAnimation) {
    m_CurrentAnimation = pAnimation;
    m_CurrentTime = 0.0f;
    m_HasEnded = false;
}

void Animator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform) {
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    Bone* bone = m_CurrentAnimation->FindBone(nodeName);

    if (bone) {
        bone->Update(m_CurrentTime);
        nodeTransform = bone->m_LocalTransform;
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    auto boneInfoMap = m_CurrentAnimation->m_BoneInfoMap;
    if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
        int index = boneInfoMap[nodeName].id;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        m_FinalBoneMatrices[index] = globalTransformation * offset;
    }

    for (int i = 0; i < node->childrenCount; i++)
        CalculateBoneTransform(&node->children[i], globalTransformation);
}

void Animator::ResetToFirstFrame() {
    if (m_CurrentAnimation) {
        // Calculate transforms for first frame
        float savedTime = m_CurrentTime;
        m_CurrentTime = 0.0f;
        CalculateBoneTransform(&m_CurrentAnimation->m_RootNode, glm::mat4(1.0f));
        m_CurrentTime = savedTime;  // Restore time for UI purposes
    }
}

}