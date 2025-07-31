/**
 * @file Model.h
 * @brief 3D model loading and rendering system using ASSIMP
 * 
 * Provides a comprehensive model loading system that can handle various
 * 3D file formats (OBJ, FBX, DAE, etc.) and render them efficiently.
 * Uses ASSIMP library for model importing and supports multiple meshes,
 * materials, and textures per model.
 */

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

/**
 * @brief 3D Model class for loading and rendering complex models
 * 
 * This class provides a complete solution for loading 3D models from files
 * and rendering them using OpenGL. It supports:
 * - Multiple mesh handling within a single model
 * - Texture loading and management
 * - Material properties
 * - Gamma correction for proper lighting
 * 
 * The class uses ASSIMP library to import various 3D file formats
 * and converts them into renderable mesh objects with associated textures.
 */
class Model {
public:
    // Model data storage
    std::vector<Texture> textures_loaded;  // Cache of loaded textures to avoid duplicates
    std::vector<Mesh> meshes;              // All meshes that make up this model
    std::string directory;                 // Directory path where model file is located
    bool gammaCorrection;                  // Whether to apply gamma correction to textures

    /**
     * @brief Constructor - loads a model from file
     * 
     * @param path File path to the 3D model file
     * @param gamma Whether to enable gamma correction for textures
     */
    Model(const std::string& path, bool gamma = false);

    /**
     * @brief Render the entire model using the specified shader
     * 
     * Iterates through all meshes in the model and renders each one
     * with the provided shader program. Assumes the shader is already
     * activated and has appropriate uniforms set.
     * 
     * @param shader The shader program to use for rendering
     */
    void Draw(Shader& shader);

private:
    /**
     * @brief Load model data from file using ASSIMP
     * 
     * Uses ASSIMP library to import the model file and process it
     * into a format suitable for OpenGL rendering. Applies post-processing
     * options for optimization and compatibility.
     * 
     * @param path File path to the model file
     */
    void LoadModel(const std::string& path);

    /**
     * @brief Recursively process ASSIMP scene nodes
     * 
     * ASSIMP models are organized in a tree structure of nodes.
     * This function recursively traverses the tree and processes
     * each mesh found in the nodes.
     * 
     * @param node Current ASSIMP node to process
     * @param scene The complete ASSIMP scene data
     */
    void ProcessNode(aiNode* node, const aiScene* scene);

    /**
     * @brief Convert ASSIMP mesh to our Mesh format
     * 
     * Extracts vertex data, indices, and material information from
     * an ASSIMP mesh and creates our own Mesh object that can be
     * rendered with OpenGL.
     * 
     * @param mesh ASSIMP mesh to convert
     * @param scene The complete ASSIMP scene (for material access)
     * @return Converted Mesh object ready for rendering
     */
    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

    /**
     * @brief Load textures for a material from ASSIMP
     * 
     * Extracts texture information from ASSIMP material and loads
     * the actual texture files. Maintains a cache to avoid loading
     * the same texture multiple times.
     * 
     * @param mat ASSIMP material containing texture information
     * @param type Type of texture to load (diffuse, specular, normal, etc.)
     * @param typeName String name for the texture type
     * @return Vector of loaded Texture objects
     */
    std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};
