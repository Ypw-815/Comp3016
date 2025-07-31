/**
 * @file ShadowMapping.h
 * @brief Advanced shadow mapping system for realistic lighting
 * 
 * Implements comprehensive shadow mapping techniques including:
 * - Multiple shadow quality levels for performance scaling
 * - Various filtering methods (PCF, linear, nearest)
 * - Support for directional, point, and spot lights
 * - Cascaded shadow maps for large scenes
 * - Bias and peter-panning mitigation
 * - Real-time shadow map updates
 * 
 * Features:
 * - High-quality shadow rendering with minimal performance impact
 * - Configurable shadow map resolution and filtering
 * - Advanced shadow bias techniques to reduce artifacts
 * - Multiple light type support with optimized shadow volumes
 * - Integration with main rendering pipeline
 */

#ifndef SHADOW_MAPPING_H
#define SHADOW_MAPPING_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>

class Shader;

/**
 * @brief Advanced shadow mapping system for realistic dynamic lighting
 * 
 * Provides high-quality shadow rendering with multiple quality levels,
 * filtering options, and support for various light types. Optimized
 * for real-time performance while maintaining visual fidelity.
 */
class ShadowMapping {
public:

    /**
     * @brief Shadow map quality levels
     * 
     * Defines resolution presets for shadow maps balancing
     * visual quality with rendering performance.
     */
    enum class ShadowQuality {
        LOW = 512,      // 512x512 - mobile/low-end hardware
        MEDIUM = 1024,  // 1024x1024 - balanced quality/performance
        HIGH = 2048,    // 2048x2048 - high-end desktops
        ULTRA = 4096    // 4096x4096 - maximum quality
    };

    /**
     * @brief Shadow filtering methods
     * 
     * Different filtering techniques for shadow edge softening
     * and artifact reduction.
     */
    enum class FilterMode {
        NEAREST,        // Hard shadows, maximum performance
        LINEAR,         // Basic linear filtering
        PCF_2x2,        // 2x2 Percentage Closer Filtering
        PCF_3x3,        // 3x3 PCF - good balance
        PCF_5x5         // 5x5 PCF - softest shadows, highest cost
    };

    /**
     * @brief Supported light types for shadow casting
     * 
     * Different light types require different shadow projection
     * matrices and rendering techniques.
     */
    enum class LightType {
        DIRECTIONAL,    // Sun-like directional lighting
        POINT,          // Omnidirectional point lights
        SPOT            // Cone-shaped spot lights
    };

private:
    // OpenGL shadow mapping resources
    GLuint m_shadowMapFBO;      // Shadow map framebuffer object
    GLuint m_shadowMap;         // Shadow depth texture
    int m_shadowMapSize;        // Current shadow map resolution
    ShadowQuality m_quality;    // Current quality setting
    FilterMode m_filterMode;    // Current filtering method

    // Light configuration
    LightType m_lightType;      // Type of light casting shadows
    glm::vec3 m_lightPosition;  // World position of light source
    glm::vec3 m_lightDirection; // Direction vector for directional/spot lights
    glm::vec3 m_lightTarget;
    float m_lightFOV;
    float m_nearPlane;
    float m_farPlane;

    
    glm::mat4 m_lightSpaceMatrix;
    glm::mat4 m_lightProjection;
    glm::mat4 m_lightView;

    
    float m_depthBias;
    float m_normalBias;
    bool m_enableSlopeScaledBias;

    
    std::unique_ptr<Shader> m_shadowMapShader;
    std::unique_ptr<Shader> m_shadowReceiveShader;

    
    bool m_enableDebugView;
    bool m_enableCulling;
    glm::vec4 m_frustumPlanes[6];

public:

    ShadowMapping(ShadowQuality quality = ShadowQuality::MEDIUM, 
                  FilterMode filterMode = FilterMode::PCF_3x3);


    ~ShadowMapping();


    bool Initialize();


    void Cleanup();


    void SetLight(LightType type, 
                  const glm::vec3& position, 
                  const glm::vec3& direction = glm::vec3(0, -1, 0),
                  float fov = 90.0f);


    void SetShadowRange(float nearPlane, float farPlane);


    void BeginShadowMapPass();


    void EndShadowMapPass(int windowWidth, int windowHeight);


    void BeginShadowReceivePass(const glm::mat4& viewMatrix, const glm::mat4& projMatrix);


    void BindShadowMap(int textureUnit = 0);


    void SetShadowUniforms(Shader* shader);


    const glm::mat4& GetLightSpaceMatrix() const { return m_lightSpaceMatrix; }


    GLuint GetShadowMapTexture() const { return m_shadowMap; }

    
    void SetShadowQuality(ShadowQuality quality);
    void SetFilterMode(FilterMode mode);
    void SetDepthBias(float bias) { m_depthBias = bias; }
    void SetNormalBias(float bias) { m_normalBias = bias; }
    void EnableSlopeScaledBias(bool enable) { m_enableSlopeScaledBias = enable; }

    
    void EnableDebugView(bool enable) { m_enableDebugView = enable; }
    void EnableFrustumCulling(bool enable) { m_enableCulling = enable; }
    void RenderDebugQuad();

    
    struct ShadowStats {
        int shadowMapSize;
        int renderedObjects;
        int culledObjects;
        float shadowMapRenderTime;
        float totalShadowTime;
    };
    
    ShadowStats GetPerformanceStats() const;

private:

    bool CreateShadowMapFBO();

    void CalculateLightSpaceMatrix();

    glm::mat4 CalculateOrthographicProjection(const glm::vec3& sceneMin, const glm::vec3& sceneMax);


    void CalculateFrustumPlanes();


    bool IsInLightFrustum(const glm::vec3& center, float radius);


    bool LoadShaders();
};


class ShadowMappingManager {
private:
    std::vector<std::unique_ptr<ShadowMapping>> m_shadowMaps;
    bool m_globalShadowsEnabled;
    float m_globalShadowStrength;

public:
    ShadowMappingManager();
    ~ShadowMappingManager();


    ShadowMapping* AddShadowLight(int lightIndex, 
                                  ShadowMapping::LightType type,
                                  ShadowMapping::ShadowQuality quality = ShadowMapping::ShadowQuality::MEDIUM);


    void RemoveShadowLight(int lightIndex);


    ShadowMapping* GetShadowMapping(int lightIndex);


    void RenderAllShadowMaps(int windowWidth, int windowHeight);


    void SetupShadowReceive(const glm::mat4& viewMatrix, const glm::mat4& projMatrix);


    void BindAllShadowMaps(int startTextureUnit = 8);

    
    void SetGlobalShadowsEnabled(bool enabled) { m_globalShadowsEnabled = enabled; }
    void SetGlobalShadowStrength(float strength) { m_globalShadowStrength = glm::clamp(strength, 0.0f, 1.0f); }
    
    bool AreGlobalShadowsEnabled() const { return m_globalShadowsEnabled; }
    float GetGlobalShadowStrength() const { return m_globalShadowStrength; }
};

#endif 
