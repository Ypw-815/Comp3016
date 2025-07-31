#include "FontRenderer.h"

/**
 * @brief 8x8 bitmap font data for basic ASCII characters (32-126)
 * 
 * This static array contains bitmap data for rendering basic text characters.
 * Each character is represented as an 8x8 pixel bitmap where each byte
 * represents one row of pixels. Bit 1 = pixel on, Bit 0 = pixel off.
 * 
 * Characters include:
 * - Space (32) through tilde (126)
 * - Numbers 0-9, uppercase A-Z, lowercase a-z
 * - Common punctuation and symbols
 */
static const unsigned char font8x8_basic[128][8] = {
    
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // Space character (ASCII 32)
    
    { 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00},   // Exclamation mark (!)
    
    
    { 0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00},   // Number 0
    
    { 0x18, 0x18, 0x38, 0x18, 0x18, 0x18, 0x7E, 0x00},   // Number 1
    
    { 0x3C, 0x66, 0x06, 0x0C, 0x30, 0x60, 0x7E, 0x00},   // Number 2
    
    { 0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00},   // Number 3
    
    { 0x06, 0x0E, 0x1E, 0x66, 0x7F, 0x06, 0x06, 0x00},   // Number 4
    
    { 0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00},   // Number 5
    
    { 0x3C, 0x66, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00},   // Number 6
    
    { 0x7E, 0x66, 0x0C, 0x18, 0x18, 0x18, 0x18, 0x00},   // Number 7
    
    { 0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00},   // Number 8
    
    { 0x3C, 0x66, 0x66, 0x3E, 0x06, 0x66, 0x3C, 0x00},   // Number 9
    
    { 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00},   // Colon (:)
    
    
    { 0x18, 0x3C, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},   // Letter A
    
    { 0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00},   // Letter B
    
    { 0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00},   // Letter C
    
    { 0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00},   // Letter D
    
    { 0x7E, 0x60, 0x60, 0x78, 0x60, 0x60, 0x7E, 0x00},   // Letter E
    
    { 0x7E, 0x60, 0x60, 0x78, 0x60, 0x60, 0x60, 0x00},   // Letter F
    
    { 0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3C, 0x00},   // Letter G
    
    { 0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},   // Letter H
    
    { 0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00},   // Letter I
    
    { 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x6C, 0x38, 0x00},   // Letter J
    
    { 0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00},   // Letter K
    
    { 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00},   // Letter L
    
    { 0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00},   // Letter M
    
    { 0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00},   // Letter N
    
    { 0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},   // Letter O
    
    { 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00},   // Letter P
    
    { 0x3C, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x0E, 0x00},   // Letter Q
    
    { 0x7C, 0x66, 0x66, 0x7C, 0x78, 0x6C, 0x66, 0x00},   // Letter R
    
    { 0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00},   // Letter S
    
    { 0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},   // Letter T
    
    { 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},   // Letter U
    
    { 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00},   // Letter V
    
    { 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},   // Letter W
    
    { 0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00},   // Letter X
    
    { 0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00},   // Letter Y
    
    { 0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00},   // Letter Z
};

/**
 * @brief OpenGL vertex shader source code for font rendering
 * 
 * This shader transforms vertex positions using a projection matrix
 * and passes texture coordinates to the fragment shader.
 * 
 * Inputs:
 * - vertex: vec4 containing position (xy) and texture coordinates (zw)
 * - projection: 4x4 projection matrix uniform
 * 
 * Outputs:
 * - gl_Position: transformed vertex position
 * - TexCoords: texture coordinates for fragment shader
 */


const char* fontVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec4 vertex; 
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

/**
 * @brief OpenGL fragment shader source code for font rendering
 * 
 * This shader samples the font texture and applies the specified text color.
 * Uses alpha blending where the texture's red channel serves as the alpha value.
 * 
 * Inputs:
 * - TexCoords: texture coordinates from vertex shader
 * - text: 2D texture sampler for the font bitmap
 * - textColor: RGB color uniform for the text
 * 
 * Output:
 * - color: final RGBA color with alpha from texture
 */


const char* fontFragmentShaderSource = R"(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = vec4(textColor, 1.0) * sampled;
}
)";

namespace CustomGUI {

/**
 * @brief Default constructor for FontRenderer
 * 
 * Initializes all OpenGL object handles to 0 and sets up
 * a default identity projection matrix.
 */
FontRenderer::FontRenderer() : VAO(0), VBO(0), shaderProgram(0) {
    projectionMatrix = glm::mat4(1.0f);
}

/**
 * @brief Destructor for FontRenderer
 * 
 * Properly cleans up all OpenGL resources including:
 * - Vertex Array Object (VAO)
 * - Vertex Buffer Object (VBO) 
 * - Shader program
 * - All character texture objects
 */

FontRenderer::~FontRenderer() {
    // Clean up OpenGL vertex array and buffer objects
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);
    
    // Clean up all character texture objects
    for (auto& pair : characters) {
        glDeleteTextures(1, &pair.second.textureID);
    }
}

/**
 * @brief Initialize the FontRenderer system
 * 
 * Sets up the complete font rendering pipeline:
 * 1. Compiles and links vertex and fragment shaders
 * 2. Creates and configures VAO and VBO for rendering
 * 3. Generates texture atlases for all printable ASCII characters
 * 
 * @return true if initialization successful, false otherwise
 */

bool FontRenderer::Initialize() {
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &fontVertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fontFragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    // Create and link shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // Clean up individual shader objects (no longer needed after linking)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Set up vertex array and buffer objects for text rendering
    // VAO stores the vertex attribute configuration
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // Allocate buffer space for 6 vertices * 4 floats each (position + texcoord)
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    
    // Configure vertex attributes: position (xy) and texture coordinates (zw)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Generate texture atlases for all printable characters
    GenerateCharacterTextures();
    
    std::cout << "FontRenderer initialized successfully!" << std::endl;
    return true;
}

/**
 * @brief Generate OpenGL textures for all printable ASCII characters
 * 
 * Creates individual 8x8 RGBA textures for each character from ASCII 32-126.
 * Each character's bitmap data is converted to a texture with:
 * - White RGB values (255, 255, 255)
 * - Alpha channel based on pixel data (255 for on pixels, 0 for off pixels)
 * - Nearest neighbor filtering for crisp pixel art appearance
 */

void FontRenderer::GenerateCharacterTextures() {
    // Generate textures for printable ASCII characters (space through tilde)
    for (int c = 32; c < 127; ++c) { 
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        // Convert 8x8 bitmap to RGBA texture data
        unsigned char bitmap[8 * 8 * 4];  // 8x8 pixels * 4 channels (RGBA)
        for (int y = 0; y < 8; ++y) {
            // Get the bitmap row for this character (bounds checking)
            unsigned char row = (c < 128) ? font8x8_basic[c][y] : 0;
            for (int x = 0; x < 8; ++x) {
                // Test if pixel is set (bit 7-x in the row byte)
                bool pixel = (row & (1 << (7-x))) != 0;
                int idx = (y * 8 + x) * 4;  // Calculate RGBA pixel index
                
                // Set RGB to white, alpha based on pixel state
                bitmap[idx] = 255;     // Red
                bitmap[idx+1] = 255;   // Green  
                bitmap[idx+2] = 255;   // Blue
                bitmap[idx+3] = pixel ? 255 : 0;  // Alpha (transparent for off pixels)
            }
        }
        
        // Upload texture data to GPU
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
        
        // Configure texture parameters for pixel-perfect rendering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // Crisp pixels
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // No blurring
        
        // Store character data for later use
        CustomGUI::Character character = {
            texture,                    // OpenGL texture ID
            glm::ivec2(8, 8),          // Character size (8x8 pixels)
            glm::ivec2(0, 8),          // Bearing (offset from baseline)
            8                          // Advance width (pixels to next character)
        };
        characters.insert(std::pair<char, CustomGUI::Character>(c, character));
    }
    
    std::cout << "Generated " << characters.size() << " character textures" << std::endl;
}

/**
 * @brief Set the projection matrix for text rendering
 * 
 * Updates the projection matrix used to transform text vertices.
 * Typically set to an orthographic projection for 2D text rendering.
 * 
 * @param projection 4x4 projection matrix (usually orthographic)
 */

void FontRenderer::SetProjection(const glm::mat4& projection) {
    projectionMatrix = projection;
}

/**
 * @brief Render a text string to the screen
 * 
 * Renders the given text string using the bitmap font textures.
 * Each character is rendered as a textured quad with alpha blending.
 * 
 * @param text The string to render
 * @param x Starting X coordinate (in pixels)
 * @param y Starting Y coordinate (in pixels) 
 * @param scale Scale factor for text size (1.0 = normal size)
 * @param color RGB color for the text (vec3 with values 0.0-1.0)
 */

void FontRenderer::RenderText(const std::string& text, float x, float y, float scale, const glm::vec3& color) {
    // Activate the font rendering shader program
    glUseProgram(shaderProgram);
    
    // Get uniform locations for shader parameters
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "textColor");
    
    // Set shader uniforms
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projectionMatrix[0][0]);  // Projection matrix
    glUniform3f(colorLoc, color.x, color.y, color.z);                   // Text color
    
    // Activate texture unit 0 and bind vertex array
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);
    
    // Enable alpha blending for transparent character backgrounds
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Render each character in the string
    for (auto c = text.begin(); c != text.end(); c++) {
        CustomGUI::Character ch = characters[*c];
        
        // Calculate character position accounting for baseline and bearing
        float xpos = x;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;
        
        // Calculate character dimensions with scaling
        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        
        // Define vertices for character quad (2 triangles = 6 vertices)
        // Each vertex: [x, y, texture_u, texture_v]
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },  // Top-left
            { xpos,     ypos,       0.0f, 1.0f },  // Bottom-left
            { xpos + w, ypos,       1.0f, 1.0f },  // Bottom-right

            { xpos,     ypos + h,   0.0f, 0.0f },  // Top-left
            { xpos + w, ypos,       1.0f, 1.0f },  // Bottom-right
            { xpos + w, ypos + h,   1.0f, 0.0f }   // Top-right
        };
        
        // Bind character texture and update vertex buffer
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // Render the character quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // Advance X position for next character
        // Note: advance is in 1/64th pixels, so we shift right by 6 bits
        x += (ch.advance >> 6) * scale;
    }
    
    // Clean up OpenGL state
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}

}  // namespace CustomGUI
