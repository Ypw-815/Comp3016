/**
 * @file TextureManager.h
 * @brief Centralized texture loading and management system
 * 
 * Provides efficient texture loading with automatic caching to prevent
 * duplicate texture loading and optimize memory usage. Integrates with
 * STB image library for wide format support.
 * 
 * Features:
 * - Automatic texture caching and reuse
 * - Support for multiple image formats (PNG, JPG, TGA, etc.)
 * - Optional vertical flipping for different coordinate systems
 * - Proper OpenGL texture object management
 * - Memory optimization through texture sharing
 */

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

/**
 * @brief Centralized texture management and caching system
 * 
 * Manages texture loading, caching, and OpenGL texture object lifecycle.
 * Prevents duplicate loading of the same texture files and provides
 * efficient texture resource management for the entire application.
 */
class TextureManager {
private:
    std::map<std::string, unsigned int> loadedTextures;  // Cache of loaded textures by filepath
    
public:
    /**
     * @brief Load texture from file with automatic caching
     * 
     * Loads an image file as an OpenGL texture. If the texture has been
     * loaded before, returns the cached texture ID instead of reloading.
     * 
     * @param path Filesystem path to the image file
     * @param flip Whether to flip the image vertically (default: false)
     * @return OpenGL texture ID, or 0 if loading failed
     */
    unsigned int LoadTexture(const std::string& path, bool flip = false) {
        // Check if texture is already loaded and cached
        if (loadedTextures.find(path) != loadedTextures.end()) {
            return loadedTextures[path];
        }
        
        unsigned int textureID;
        glGenTextures(1, &textureID);
        
        // Configure STB image loading options
        stbi_set_flip_vertically_on_load(flip);
        
        int width, height, nrChannels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
        
        if (data) {
            GLenum internalFormat, dataFormat;
            
            switch (nrChannels) {
                case 1:
                    internalFormat = dataFormat = GL_RED;
                    break;
                case 3:
                    internalFormat = GL_RGB;
                    dataFormat = GL_RGB;
                    break;
                case 4:
                    internalFormat = GL_RGBA;
                    dataFormat = GL_RGBA;
                    break;
                default:
                    std::cerr << "Unsupported texture format: " << nrChannels << " channels" << std::endl;
                    stbi_image_free(data);
                    return 0;
            }
            
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            
            
            if (glfwExtensionSupported("GL_EXT_texture_filter_anisotropic")) {
                float maxAnisotropy;
                glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(16.0f, maxAnisotropy));
            }
            
            stbi_image_free(data);
            loadedTextures[path] = textureID;
            
            std::cout << "Loaded texture: " << path << " (" << width << "x" << height << ", " << nrChannels << " channels)" << std::endl;
        } else {
            std::cerr << "Failed to load texture: " << path << std::endl;
            std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        }
        
        return textureID;
    }
    
    
    struct PBRMaterial {
        unsigned int albedo = 0;
        unsigned int normal = 0;
        unsigned int metallic = 0;
        unsigned int roughness = 0;
        unsigned int ao = 0;
        unsigned int height = 0;
        
        glm::vec3 albedoColor = glm::vec3(1.0f);
        float metallicValue = 0.0f;
        float roughnessValue = 0.5f;
    };
    
    PBRMaterial LoadPBRMaterial(const std::string& basePath, const std::string& name) {
        PBRMaterial material;
        
        
        std::string albedoPath = basePath + "/" + name + "_albedo.jpg";
        std::string normalPath = basePath + "/" + name + "_normal.jpg";
        std::string metallicPath = basePath + "/" + name + "_metallic.jpg";
        std::string roughnessPath = basePath + "/" + name + "_roughness.jpg";
        std::string aoPath = basePath + "/" + name + "_ao.jpg";
        std::string heightPath = basePath + "/" + name + "_height.jpg";
        
        
        std::vector<std::string> albedoNames = {"_albedo.jpg", "_diff.jpg", "_diffuse.jpg", "_color.jpg", ".jpg"};
        std::vector<std::string> normalNames = {"_normal.jpg", "_norm.jpg", "_nrm.jpg"};
        std::vector<std::string> metallicNames = {"_metallic.jpg", "_metal.jpg", "_met.jpg"};
        std::vector<std::string> roughnessNames = {"_roughness.jpg", "_rough.jpg", "_rgh.jpg"};
        std::vector<std::string> aoNames = {"_ao.jpg", "_ambient.jpg", "_occlusion.jpg"};
        std::vector<std::string> heightNames = {"_height.jpg", "_disp.jpg", "_displacement.jpg"};
        
        
        for (const auto& suffix : albedoNames) {
            std::string path = basePath + "/" + name + suffix;
            std::ifstream file(path);
            if (file.good()) {
                material.albedo = LoadTexture(path);
                break;
            }
        }
        
        for (const auto& suffix : normalNames) {
            std::string path = basePath + "/" + name + suffix;
            std::ifstream file(path);
            if (file.good()) {
                material.normal = LoadTexture(path);
                break;
            }
        }
        
        
        
        std::cout << "Loaded PBR material: " << name << std::endl;
        return material;
    }
    
    
    void Cleanup() {
        for (auto& pair : loadedTextures) {
            glDeleteTextures(1, &pair.second);
        }
        loadedTextures.clear();
    }
    
    ~TextureManager() {
        Cleanup();
    }
};


/*

TextureManager textureManager;



m_diffuseTexture = textureManager.LoadTexture("resources/textures/hq/wood/wood_planks_4k.jpg");
m_specularTexture = textureManager.LoadTexture("resources/textures/hq/metal/brushed_metal_4k.jpg");


TextureManager::PBRMaterial woodMaterial = textureManager.LoadPBRMaterial("resources/textures/hq/wood", "wood_planks_4k");
*/
