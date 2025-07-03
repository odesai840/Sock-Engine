#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in ivec4 aBoneIDs;
layout (location = 6) in vec4 aBoneWeights;

out vec3 FragPos;
out vec2 TexCoords;
out mat3 TBN;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;
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

    // Transform to world space
    vec4 worldPos = model * animatedPos;
    FragPos = worldPos.xyz;
    TexCoords = aTexCoords;

    // Apply bone transformation to normal vectors
    mat3 boneNormalMatrix = mat3(boneTransform);
    vec3 animatedNormal = boneNormalMatrix * aNormal;
    vec3 animatedTangent = boneNormalMatrix * aTangent;
    vec3 animatedBitangent = boneNormalMatrix * aBitangent;

    // Calculate normal matrix and transform animated normals to world space
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * animatedTangent);
    vec3 N = normalize(normalMatrix * animatedNormal);

    // Re-orthogonalize T with respect to N
    T = normalize(T - dot(T, N) * N);
    // Then retrieve perpendicular vector B with the cross product of T and N while accounting for handedness
    vec3 B = cross(N, T) * (dot(cross(animatedNormal, animatedTangent), animatedBitangent) < 0.0 ? -1.0 : 1.0);
    TBN = mat3(T, B, N);

    // Calculate fragment position in light space for shadow mapping
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

    gl_Position = projection * view * worldPos;
}