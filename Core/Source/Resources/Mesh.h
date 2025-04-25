#ifndef MESH_H
#define MESH_H

#include "Shader.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define MAX_BONE_INFLUENCE 4

namespace SockEngine {

struct Vertex {
    // Position
    glm::vec3 Position;
    // Normal
    glm::vec3 Normal;
    // TexCoords
    glm::vec2 TexCoords;
    // Tangent
    glm::vec3 Tangent;
    // Bitangent
    glm::vec3 Bitangent;
    // Bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    // Weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
public:
    // Mesh data
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    unsigned int VAO;

    // Constructor
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

    // Render the mesh
    void Draw(Shader& shader);

private:
    // Render data 
    unsigned int VBO, EBO;

    // Initializes all the buffer objects/arrays
    void SetupMesh();
};

}

#endif