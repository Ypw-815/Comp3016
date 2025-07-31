/**
 * @file Mesh.h
 * @brief Mesh rendering system for 3D geometry
 * 
 * Provides a mesh rendering system that encapsulates vertex data,
 * indices, textures, and OpenGL buffer management. Works in conjunction
 * with the Model class to render complex 3D objects efficiently.
 * 
 * Features:
 * - Vertex attribute management (position, normal, texture coords, tangents)
 * - OpenGL buffer setup and management
 * - Texture binding and shader integration
 * - Efficient rendering with indexed geometry
 */

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Texture.h"

#include <string>
#include <vector>

/**
 * @brief Vertex structure containing all vertex attributes
 * 
 * Defines the complete set of per-vertex data needed for modern
 * 3D rendering including position, lighting, and advanced effects.
 */
struct Vertex {
    glm::vec3 Position;   // 3D world position
    glm::vec3 Normal;     // Normal vector for lighting calculations
    glm::vec2 TexCoords;  // Texture coordinates (UV mapping)
    glm::vec3 Tangent;    // Tangent vector for normal mapping
    glm::vec3 Bitangent;  // Bitangent vector for normal mapping
};

/**
 * @brief Mesh class for rendering 3D geometry
 * 
 * Encapsulates a renderable mesh with vertices, indices, and textures.
 * Manages OpenGL buffer objects and provides efficient rendering
 * through indexed drawing. Typically used as part of larger Model objects.
 * 
 * Key features:
 * - Indexed geometry rendering for efficiency
 * - Multiple texture support per mesh
 * - Automatic OpenGL buffer management
 * - Shader uniform setup for textures
 * - Support for advanced vertex attributes
 */
class Mesh {
public:
    // Mesh data
    std::vector<Vertex> vertices;        // All vertices in the mesh
    std::vector<unsigned int> indices;   // Triangle indices for rendering
    std::vector<Texture> textures;       // Textures associated with this mesh
    unsigned int VAO;                    // Vertex Array Object ID

    /**
     * @brief Constructor for indexed mesh
     * 
     * Creates a mesh with vertices and indices. Automatically sets up
     * OpenGL buffers and vertex attributes for efficient rendering.
     * 
     * @param vertices Array of vertex data
     * @param indices Array of triangle indices
     */
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
    
    /**
     * @brief Constructor for mesh with textures
     * 
     * Creates a mesh with vertices, indices, and associated textures.
     * 
     * @param vertices Array of vertex data
     * @param indices Array of triangle indices
     * @param textures Array of textures for this mesh
     */
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures); 

    /**
     * @brief Render the mesh using the specified shader
     * 
     * Binds all textures, sets up shader uniforms, and renders
     * the mesh using indexed drawing. Both naming conventions supported.
     * 
     * @param shader Shader program to use for rendering
     */
    void draw(Shader& shader);  // Legacy naming
    void Draw(Shader& shader);  // Modern naming

private:
    // OpenGL buffer object IDs
    unsigned int VBO, EBO;  // Vertex Buffer Object, Element Buffer Object

    /**
     * @brief Initialize OpenGL buffers and vertex attributes
     * 
     * Sets up VAO, VBO, and EBO with the mesh data and configures
     * vertex attribute pointers for position, normal, texture coordinates,
     * tangent, and bitangent vectors.
     */
    void SetupMesh();
};
