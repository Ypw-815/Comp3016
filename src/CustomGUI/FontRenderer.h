/**
 * @file FontRenderer.h
 * @brief Custom bitmap font rendering system for GUI text display
 * 
 * Provides a complete text rendering solution using 8x8 bitmap fonts.
 * Designed for retro-style pixel-perfect text rendering in OpenGL applications.
 * 
 * Features:
 * - 8x8 bitmap font for ASCII characters 32-126
 * - Hardware-accelerated OpenGL rendering
 * - Alpha blending support for transparent backgrounds
 * - Scalable text with nearest-neighbor filtering
 * - Custom color support for text rendering
 * - Optimized vertex buffer management
 */

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <map>
#include <memory>
#include <iostream>

namespace CustomGUI {

/**
 * @brief Character data structure for font rendering
 * 
 * Contains all necessary information to render a single character
 * including texture handle, dimensions, and positioning data.
 */
struct Character {
    GLuint textureID;   // OpenGL texture ID for this character
    glm::ivec2 size;    // Character dimensions in pixels (width, height)
    glm::ivec2 bearing; // Offset from baseline to character top-left (x, y)
    GLuint advance;     // Horizontal advance to next character position
};

/**
 * @brief Bitmap font renderer class
 * 
 * Manages the complete pipeline for rendering bitmap text including:
 * - Character texture generation from bitmap data
 * - OpenGL shader program compilation and management
 * - Vertex buffer objects for efficient rendering
 * - Text positioning and scaling calculations
 * 
 * Usage:
 * 1. Initialize() to set up OpenGL resources
 * 2. SetProjection() to configure screen space transforms
 * 3. RenderText() to draw text strings to the screen
 */
class FontRenderer {
public:
    /**
     * @brief Constructor - initializes basic state
     */
    FontRenderer();
    
    /**
     * @brief Destructor - cleans up OpenGL resources
     */
    ~FontRenderer();

    /**
     * @brief Initialize the font rendering system
     * 
     * Sets up OpenGL shaders, vertex buffers, and generates
     * character textures from bitmap font data.
     * 
     * @return true if initialization successful, false otherwise
     */
    bool Initialize();
    
    /**
     * @brief Render a text string to the screen
     * 
     * Draws the specified text using bitmap font textures with
     * support for scaling and custom colors.
     * 
     * @param text String to render
     * @param x Starting X coordinate in screen space
     * @param y Starting Y coordinate in screen space
     * @param scale Text scale factor (1.0 = normal size)
     * @param color RGB color values (0.0-1.0 range)
     */
    void RenderText(const std::string& text, float x, float y, float scale, const glm::vec3& color);
    
    /**
     * @brief Set the projection matrix for text rendering
     * 
     * Updates the projection matrix used to transform text coordinates
     * from screen space to normalized device coordinates.
     * 
     * @param projection 4x4 projection matrix (typically orthographic)
     */
    void SetProjection(const glm::mat4& projection);
    
private:
    /**
     * @brief Load default bitmap font data
     * 
     * Loads the embedded 8x8 bitmap font data for ASCII characters.
     * Called internally during initialization.
     * 
     * @return true if font data loaded successfully
     */
    bool LoadDefaultFont();
    
    /**
     * @brief Generate OpenGL textures for all characters
     * 
     * Converts bitmap font data into individual OpenGL textures
     * for each printable ASCII character (32-126).
     */
    void GenerateCharacterTextures();
    
    // OpenGL rendering resources
    GLuint VAO, VBO;                        // Vertex array and buffer objects
    GLuint shaderProgram;                   // Compiled shader program
    std::map<char, Character> characters;   // Character texture database
    glm::mat4 projectionMatrix;             // Current projection matrix
    
    // Static font data (defined in implementation file)
    static const unsigned char defaultFontData[];  // 8x8 bitmap font data
    static const int defaultFontDataSize;          // Size of font data array
};

}  // namespace CustomGUI