#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in ivec4 aBoneIDs;
layout (location = 6) in vec4 aBoneWeights;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main()
{
    // Calculate the bone transformation matrix
    mat4 boneTransform = mat4(0.0);
    for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if(aBoneIDs[i] == -1)
        continue;
        if(aBoneIDs[i] >= MAX_BONES)
        {
            boneTransform = mat4(1.0);
            break;
        }
        boneTransform += finalBonesMatrices[aBoneIDs[i]] * aBoneWeights[i];
    }

    // Apply bone transformation to vertex position
    vec4 animatedPos = boneTransform * vec4(aPos, 1.0);

    gl_Position = lightSpaceMatrix * model * animatedPos;
}