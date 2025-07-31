/**
 * @file Texture.h
 * @brief OpenGL texture loading and management system
 * 
 * Provides a comprehensive texture management system for OpenGL applications.
 * Supports loading textures from image files, generating procedural textures,
 * and managing OpenGL texture resources with automatic cleanup.
 * 
 * Features:
 * - Image file loading (PNG, JPG, BMP, etc.)
 * - Procedural texture generation
 * - Automatic resource management
 * - Flexible texture binding system
 * - Support for different texture types (diffuse, specular, normal, etc.)
 */

#pragma once

#include <glad/glad.h>
#include <string>

/**
 * @brief OpenGL texture wrapper class
 * 
 * Manages the lifecycle of OpenGL 2D textures including:
 * - Loading texture data from image files
 * - Creating procedural textures
 * - Binding textures to OpenGL texture units
 * - Automatic resource cleanup
 * - Texture type identification for shader usage
 * 
 * The class handles various image formats and provides both
 * file-based and procedural texture generation capabilities.
 */
class Texture {
public:
    unsigned int ID;      // OpenGL texture object ID
    std::string type;     // Texture type identifier (diffuse, specular, normal, etc.)
    std::string path;     // File path of loaded texture (for debugging/caching)

    /**
     * @brief Default constructor - creates empty texture
     */
    Texture();
    
    /**
     * @brief Constructor with immediate loading
     * 
     * Creates a texture object and immediately loads texture data
     * from the specified image file.
     * 
     * @param path Path to image file to load
     * @param type Texture type identifier (used in shaders)
     */
    Texture(const std::string& path, const std::string& type = "texture_diffuse");
    
    /**
     * @brief Destructor - cleanup OpenGL resources
     */
    ~Texture();

    /**
     * @brief Load texture from image file
     * 
     * Loads texture data from various image formats (PNG, JPG, BMP, etc.)
     * using STB Image library. Automatically detects format and handles
     * different channel configurations (RGB, RGBA, grayscale).
     * 
     * @param path Path to image file
     * @param flip Whether to flip texture vertically (for OpenGL coordinate system)
     * @return True if loading successful, false otherwise
     */
    bool LoadTexture(const std::string& path, bool flip = true);
    
    /**
     * @brief Generate procedural checkerboard texture
     * 
     * Creates a checkerboard pattern texture programmatically.
     * Useful for testing, debugging UV coordinates, or as placeholder textures.
     * 
     * @param width Texture width in pixels
     * @param height Texture height in pixels
     */
    void generateCheckerboard(int width, int height);
    
    /**
     * @brief Bind texture to OpenGL texture unit
     * 
     * Activates this texture for use in rendering by binding it to
     * the specified texture unit. Multiple textures can be bound
     * simultaneously to different units.
     * 
     * @param slot/unit OpenGL texture unit to bind to (0-31 typically)
     */
    void bind(unsigned int slot = 0) const;     // Legacy naming
    void Bind(unsigned int unit = 0) const;     // Modern naming
    
    /**
     * @brief Unbind texture from current context
     */
    void Unbind() const;

    /**
     * @brief Get OpenGL texture ID
     * @return OpenGL texture object identifier
     */
    unsigned int GetID() const { return ID; }

private:
    bool m_loaded;  // Track whether texture data has been successfully loaded
    
    /**
     * @brief Generate OpenGL texture from raw image data
     * 
     * Creates OpenGL texture object from raw pixel data and applies
     * appropriate filtering and wrapping settings.
     * 
     * @param data Raw pixel data
     * @param width Image width in pixels
     * @param height Image height in pixels
     * @param nrChannels Number of color channels (1=grayscale, 3=RGB, 4=RGBA)
     */
    void GenerateTexture(unsigned char* data, int width, int height, int nrChannels);
};
