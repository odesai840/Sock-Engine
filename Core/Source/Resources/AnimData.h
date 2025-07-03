#ifndef ANIM_DATA_H
#define ANIM_DATA_H

#include <glm/glm.hpp>
#include <map>
#include <string>

namespace SockEngine {

// Represents a bone in the skeleton
struct BoneInfo {
    int id;                     // Index in finalBoneMatrices
    glm::mat4 offset;           // Offset matrix (inverse bind pose)
};

// Type alias for bone info map
using BoneInfoMap = std::map<std::string, BoneInfo>;

}

#endif