#ifndef MODEL_H
#define MODEL_H

#include "Mesh.h"
#include "Shader.h"
#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace SockEngine {

class Model
{
public:
    // Model data 
    std::vector<Texture> textures_loaded; // Stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    std::vector<Mesh> meshes;
    std::string directory;
    bool gammaCorrection;

    // Constructor, expects a filepath to a 3D model.
    Model(std::string const& path, bool gamma = false);

    ~Model();

    // Draws the model, and thus all its meshes
    void Draw(Shader& shader);

private:
    // Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void LoadModel(std::string const& path);

    // Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void ProcessNode(aiNode* node, const aiScene* scene);

    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

    // Checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    std::vector<Texture> LoadMaterialTextures(const aiScene* scene, aiMaterial* mat, aiTextureType type, std::string typeName);

    unsigned int TextureFromFile(const char* path, const std::string& dir, bool gamma = false);

    void UnloadTextures();
};

}

#endif