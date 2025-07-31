/**
 * @file AdvancedTexture.h
 * @brief Advanced texture management system with modern graphics features
 * 
 * Provides comprehensive texture management for modern 3D rendering including:
 * - Multiple texture types (PBR materials, normal maps, etc.)
 * - Advanced filtering modes (anisotropic, mipmapping)
 * - Various texture formats (HDR, depth, multi-channel)
 * - Procedural texture generation
 * - Texture streaming and LOD management
 * - GPU memory optimization
 * 
 * Features:
 * - Physical-based rendering (PBR) texture support
 * - High dynamic range (HDR) texture formats
 * - Advanced sampling and filtering options
 * - Texture atlasing and streaming capabilities
 * - Memory-efficient texture management
 */

#pragma once

#include <glad/glad.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

/**
 * @brief Texture types for different rendering purposes
 * 
 * Defines semantic meaning of textures for proper shader binding
 * and material system integration, especially for PBR workflows.
 */
enum class TextureType {
    DIFFUSE,            // Base color/albedo texture
    SPECULAR,           // Specular reflection map
    NORMAL,             // Normal/bump mapping texture
    HEIGHT,             // Height/displacement mapping
    AMBIENT_OCCLUSION,  // Ambient occlusion map
    ROUGHNESS,          // Surface roughness for PBR
    METALLIC,           // Metallic/non-metallic classification
    NOISE,              // Procedural noise textures
    CUSTOM              // User-defined texture types
};

/**
 * @brief Texture filtering modes for different quality/performance needs
 * 
 * Controls how textures are sampled when magnified or minified,
 * affecting both visual quality and rendering performance.
 */
enum class FilterMode {
    NEAREST,            // Point sampling - crisp pixels
    LINEAR,             // Bilinear interpolation - smooth
    MIPMAP_NEAREST,     // Nearest mipmap level
    MIPMAP_LINEAR,      // Trilinear filtering with mipmaps
    ANISOTROPIC         // Anisotropic filtering - highest quality
};

/**
 * @brief Texture wrapping modes for texture coordinate handling
 * 
 * Defines behavior when texture coordinates exceed the 0-1 range,
 * important for tiling textures and avoiding edge artifacts.
 */
enum class WrapMode {
    REPEAT,             // Tile texture infinitely
    MIRRORED_REPEAT,    // Mirror texture at edges
    CLAMP_TO_EDGE,      // Extend edge pixels
    CLAMP_TO_BORDER     // Use specified border color
};

/**
 * @brief Internal texture formats for different data types
 * 
 * Specifies how texture data is stored in GPU memory,
 * affecting precision, memory usage, and rendering capabilities.
 */
enum class TextureFormat {
    RGB8,               // 8-bit RGB (standard color)
    RGBA8,              // 8-bit RGBA (standard with alpha)
    RGB16F,             // 16-bit floating point RGB (HDR)
    RGBA16F,            // 16-bit floating point RGBA (HDR)
    RGB32F,             // 32-bit floating point RGB (full precision)
    RGBA32F,            // 32-bit floating point RGBA (full precision)
    DEPTH24,            // 24-bit depth buffer
    DEPTH32F,           // 32-bit floating point depth
    R8,                 // Single channel 8-bit
    RG8,                // Two channel 8-bit
    COMPRESSED_RGB,
    COMPRESSED_RGBA
};


class AdvancedTexture {
public:
    unsigned int ID;
    TextureType type;
    std::string path;
    int width, height, channels;
    TextureFormat format;
    FilterMode filterMode;
    WrapMode wrapMode;
    bool compressed;
    float anisotropyLevel;

    
    AdvancedTexture();
    AdvancedTexture(const std::string& path, TextureType type = TextureType::DIFFUSE);
    ~AdvancedTexture();

    
    bool LoadFromFile(const std::string& path, bool flipY = true);
    bool LoadFromMemory(const unsigned char* data, int width, int height, int channels, TextureFormat format = TextureFormat::RGB8);
    void Bind(unsigned int unit = 0) const;
    void Unbind() const;

    
    void GenerateNoise(int width, int height, float scale = 1.0f, int octaves = 4);
    void GeneratePerlinNoise(int width, int height, float frequency = 0.1f, int octaves = 4);
    void GenerateWorleyNoise(int width, int height, int numPoints = 16);
    void GenerateNormalMap(const AdvancedTexture& heightMap, float strength = 1.0f);
    
    
    void SetFilterMode(FilterMode mode);
    void SetWrapMode(WrapMode mode);
    void SetAnisotropy(float level);
    void EnableCompression(bool enable);
    void GenerateMipmaps();
    
    
    void Resize(int newWidth, int newHeight);
    std::vector<unsigned char> GetPixelData() const;
    void UpdateRegion(int x, int y, int width, int height, const unsigned char* data);

    
    static std::shared_ptr<AdvancedTexture> CreateColorTexture(int width, int height, float r, float g, float b, float a = 1.0f);
    static std::shared_ptr<AdvancedTexture> CreateCheckerboard(int width, int height, int checkSize = 8);
    static std::shared_ptr<AdvancedTexture> CreateGradient(int width, int height, float startR, float startG, float startB, float endR, float endG, float endB);

private:
    bool m_loaded;
    
    void SetupTexture();
    void SetTextureParameters();
    GLenum GetGLFormat(TextureFormat format) const;
    GLenum GetGLInternalFormat(TextureFormat format) const;
    GLenum GetGLType(TextureFormat format) const;
    
    
    float Noise(int x, int y) const;
    float SmoothNoise(float x, float y) const;
    float InterpolateNoise(float x, float y) const;
    float PerlinNoise2D(float x, float y, float persistence, int octaves) const;
    float Fade(float t) const;
    float Lerp(float t, float a, float b) const;
    int FastFloor(float x) const;
    float Grad(int hash, float x, float y) const;
};


class TextureManager {
public:
    static TextureManager& Instance();
    
    
    std::shared_ptr<AdvancedTexture> LoadTexture(const std::string& path, TextureType type = TextureType::DIFFUSE);
    std::shared_ptr<AdvancedTexture> GetTexture(const std::string& name);
    void RegisterTexture(const std::string& name, std::shared_ptr<AdvancedTexture> texture);
    void UnloadTexture(const std::string& name);
    void UnloadAll();
    
    
    unsigned int CreateTextureArray(const std::vector<std::string>& paths, int width, int height);
    void BindTextureArray(unsigned int arrayID, unsigned int unit = 0);
    
    
    struct AtlasRegion {
        float u1, v1, u2, v2;  
        int width, height;     
    };
    
    unsigned int CreateTextureAtlas(const std::vector<std::string>& paths, int atlasWidth, int atlasHeight);
    AtlasRegion GetAtlasRegion(unsigned int atlasID, int index);
    
    
    void OptimizeMemoryUsage();
    size_t GetTotalMemoryUsage() const;
    void SetCompressionEnabled(bool enabled) { m_compressionEnabled = enabled; }

private:
    TextureManager() = default;
    ~TextureManager() = default;
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    
    std::unordered_map<std::string, std::shared_ptr<AdvancedTexture>> m_textures;
    std::unordered_map<unsigned int, std::vector<AtlasRegion>> m_atlasRegions;
    bool m_compressionEnabled = true;
    size_t m_memoryUsage = 0;
};


class TextureAnimator {
public:
    struct AnimationFrame {
        std::shared_ptr<AdvancedTexture> texture;
        float duration;
    };
    
    TextureAnimator();
    ~TextureAnimator();
    
    void AddFrame(std::shared_ptr<AdvancedTexture> texture, float duration);
    void Update(float deltaTime);
    std::shared_ptr<AdvancedTexture> GetCurrentTexture() const;
    
    void SetLoop(bool loop) { m_loop = loop; }
    void SetSpeed(float speed) { m_speed = speed; }
    void Reset();
    void Pause() { m_playing = false; }
    void Play() { m_playing = true; }

private:
    std::vector<AnimationFrame> m_frames;
    int m_currentFrame;
    float m_currentTime;
    float m_speed;
    bool m_loop;
    bool m_playing;
};
