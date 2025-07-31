/**
 * @file Geometry.h
 * @brief Basic 3D geometric primitives for rendering
 * 
 * Provides simple geometric primitives (cube, quad, sphere) for
 * basic 3D rendering needs. These are useful for debugging,
 * testing, placeholder geometry, and simple effects.
 * 
 * Features:
 * - Pre-built geometric primitives with proper vertex data
 * - Automatic OpenGL buffer management
 * - Simple rendering interface
 * - Efficient indexed geometry where applicable
 */

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"

/**
 * @brief 3D cube primitive
 * 
 * Provides a simple textured cube geometry for basic 3D rendering.
 * Useful for testing, debugging, placeholder objects, or simple
 * geometric effects like particles or building blocks.
 */
class Cube {
public:
    /**
     * @brief Constructor - creates and initializes cube geometry
     */
    Cube();
    
    /**
     * @brief Destructor - cleans up OpenGL resources
     */
    ~Cube();
    
    /**
     * @brief Render the cube using specified shader and transform
     * 
     * @param shader Shader program for rendering
     * @param model Model transformation matrix (default: identity)
     */
    void Draw(Shader& shader, const glm::mat4& model = glm::mat4(1.0f));
    
    /**
     * @brief Initialize cube vertex data and OpenGL buffers
     */
    void SetupCube();
    
private:
    unsigned int VAO, VBO;     // OpenGL buffer objects
    std::vector<float> vertices; // Vertex data (position + texture coords)
};

/**
 * @brief 2D quad primitive
 * 
 * Provides a simple textured quad for 2D rendering, UI elements,
 * screen-space effects, or billboards. Can be oriented in 3D space
 * using the model transformation matrix.
 */
class Quad {
public:
    /**
     * @brief Constructor - creates and initializes quad geometry
     */
    Quad();
    
    /**
     * @brief Destructor - cleans up OpenGL resources
     */
    ~Quad();
    
    /**
     * @brief Render the quad using specified shader and transform
     * 
     * @param shader Shader program for rendering
     * @param model Model transformation matrix (default: identity)
     */
    void Draw(Shader& shader, const glm::mat4& model = glm::mat4(1.0f));
    
    /**
     * @brief Initialize quad vertex data and OpenGL buffers
     */
    /**
     * @brief Initialize quad vertex data and OpenGL buffers
     */
    void SetupQuad();
    
private:
    unsigned int VAO, VBO, EBO; // OpenGL buffer objects
    std::vector<float> vertices;  // Vertex data (position + texture coords)
    std::vector<unsigned int> indices; // Triangle indices for indexed rendering
};

/**
 * @brief 3D sphere primitive
 * 
 * Provides a procedurally generated sphere with configurable tessellation.
 * Useful for representing spherical objects, particles, celestial bodies,
 * or testing lighting and shading algorithms.
 */
class Sphere {
public:
    /**
     * @brief Constructor - creates sphere with specified tessellation
     * 
     * @param segments Number of horizontal and vertical segments (default: 32)
     */
    Sphere(int segments = 32);
    
    /**
     * @brief Destructor - cleans up OpenGL resources
     */
    ~Sphere();
    
    /**
     * @brief Render the sphere using specified shader and transform
     * 
     * @param shader Shader program for rendering
     * @param model Model transformation matrix (default: identity)
     */
    void Draw(Shader& shader, const glm::mat4& model = glm::mat4(1.0f));
    
    /**
     * @brief Generate sphere geometry with specified tessellation
     * 
     * Creates vertex positions, normals, and texture coordinates for
     * a unit sphere centered at origin using spherical coordinates.
     * 
     * @param segments Number of horizontal and vertical segments
     */
    void SetupSphere(int segments);
    
private:
    unsigned int VAO, VBO, EBO;        // OpenGL buffer objects
    std::vector<float> vertices;       // Vertex data (position + normal + texcoords)
    std::vector<unsigned int> indices; // Triangle indices for indexed rendering
    int m_segments;                    // Tessellation level
};
