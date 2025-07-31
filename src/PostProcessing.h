/**
 * @file PostProcessing.h
 * @brief Advanced post-processing pipeline for enhanced visual effects
 * 
 * Implements a comprehensive post-processing system for modern 3D graphics
 * including bloom, tone mapping, depth of field, screen space effects,
 * and framebuffer management for multi-pass rendering.
 * 
 * Features:
 * - Multi-pass rendering pipeline with intermediate framebuffers
 * - HDR rendering with tone mapping support
 * - Bloom effect with configurable intensity and blur
 * - Screen-space ambient occlusion (SSAO)
 * - Depth of field with focus distance control
 * - Color grading and post-process color correction
 * - Performance-optimized render target management
 */

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>

// Forward declaration to avoid circular includes
class Shader;

/**
 * @brief Framebuffer management system for multi-pass rendering
 * 
 * Manages multiple framebuffer objects and their associated textures
 * for complex post-processing pipelines. Handles creation, binding,
 * and cleanup of render targets with proper error checking.
 */
class FramebufferManager {
public:
    /**
     * @brief Complete framebuffer data structure
     * 
     * Contains all OpenGL objects needed for a complete framebuffer
     * including color, depth, and auxiliary textures.
     */
    struct FramebufferData {
        GLuint fbo;             // OpenGL framebuffer object
        GLuint colorTexture;    // Main color attachment texture
        GLuint depthTexture;    // Depth buffer texture
        GLuint normalTexture;   // World-space normals texture (for SSAO)
        int width, height;      // Framebuffer dimensions
        
        /**
         * @brief Default constructor initializing all handles to 0
         */
        FramebufferData() : fbo(0), colorTexture(0), depthTexture(0), normalTexture(0), width(0), height(0) {}
    };

private:
    FramebufferData m_sceneBuffer;          // Primary scene rendering buffer
    FramebufferData m_postProcessBuffer[2]; // Ping-pong buffers for multi-pass effects
    FramebufferData m_bloomBuffer[4];       // Bloom effect mip-map chain buffers
    int m_currentBuffer;                    // Current active post-process buffer index

public:
    /**
     * @brief Constructor - initializes framebuffer manager state
     */
    FramebufferManager();
    
    /**
     * @brief Destructor - ensures proper cleanup of OpenGL resources
     */
    ~FramebufferManager();

    /**
     * @brief Initialize all framebuffers with specified dimensions
     * 
     * Creates and configures all framebuffer objects and their textures.
     * Must be called after OpenGL context creation.
     * 
     * @param width Framebuffer width in pixels
     * @param height Framebuffer height in pixels
     * @return True if initialization successful, false otherwise
     */
    bool Initialize(int width, int height);
    
    /**
     * @brief Clean up all OpenGL framebuffer resources
     */
    void Cleanup();
    
    /**
     * @brief Bind the scene framebuffer for primary rendering
     * 
     * Sets the scene framebuffer as the active render target.
     * All 3D scene geometry should be rendered to this buffer.
     */
    void BindSceneFramebuffer();
    
    /**
     * @brief Bind a post-processing framebuffer
     * 
     * Activates one of the post-process framebuffers for intermediate
     * rendering passes in the post-processing pipeline.
     * 
     * @param index Post-process buffer index (0 or 1 for ping-pong)
     */
    void BindPostProcessFramebuffer(int index);
    
    /**
     * @brief Bind the default framebuffer (screen)
     * 
     * Returns rendering to the default framebuffer for final output.
     */
    void BindDefaultFramebuffer();
    
    // Texture accessor methods for shader binding
    /**
     * @brief Get the scene color texture for post-processing
     * @return OpenGL texture ID for scene color buffer
     */
    GLuint GetSceneColorTexture() const { return m_sceneBuffer.colorTexture; }
    
    /**
     * @brief Get the scene depth texture for depth-based effects
     * @return OpenGL texture ID for scene depth buffer
     */
    GLuint GetSceneDepthTexture() const { return m_sceneBuffer.depthTexture; }
    
    /**
     * @brief Get the scene normal texture for SSAO and lighting
     * @return OpenGL texture ID for world-space normals
     */
    GLuint GetSceneNormalTexture() const { return m_sceneBuffer.normalTexture; }
    
    /**
     * @brief Get post-process buffer texture by index
     * 
     * @param index Buffer index (0 or 1)
     * @return OpenGL texture ID for the specified post-process buffer
     */
    GLuint GetPostProcessTexture(int index) const;
    
    /**
     * @brief Get bloom effect texture by mip level
     * 
     * @param level Bloom mip level (0-3, where 0 is highest resolution)
     * @return OpenGL texture ID for the specified bloom buffer
     */
    GLuint GetBloomTexture(int level) const;
    
    /**
     * @brief Resize all framebuffers to new dimensions
     * 
     * Recreates all framebuffer textures with new size. Call when
     * window is resized to maintain proper rendering resolution.
     * 
     * @param width New framebuffer width
     * @param height New framebuffer height
     */
    void ResizeFramebuffers(int width, int height);
    
    /**
     * @brief Check if a framebuffer is complete and valid
     * 
     * Validates framebuffer completeness status for debugging.
     * 
     * @param fbo Framebuffer object to check
     * @return True if framebuffer is complete and ready for use
     */
    bool CheckFramebufferComplete(GLuint fbo) const;

private:
    bool CreateFramebuffer(FramebufferData& fb, int width, int height, bool needsNormal = false);
    void DeleteFramebuffer(FramebufferData& fb);
};


class PostProcessEffect {
protected:
    std::unique_ptr<Shader> m_shader;
    bool m_enabled;
    float m_intensity;

public:
    PostProcessEffect() : m_enabled(true), m_intensity(1.0f) {}
    virtual ~PostProcessEffect() = default;

    virtual bool Initialize() = 0;
    virtual void Apply(GLuint inputTexture, GLuint outputFBO, int width, int height) = 0;
    virtual void SetParameter(const std::string& name, float value) {}
    
    void SetEnabled(bool enabled) { m_enabled = enabled; }
    void SetIntensity(float intensity) { m_intensity = intensity; }
    bool IsEnabled() const { return m_enabled; }
    float GetIntensity() const { return m_intensity; }
};


class ToneMappingEffect : public PostProcessEffect {
public:
    enum class Mode {
        NONE = 0,
        REINHARD = 1,
        ACES = 2,
        UNCHARTED2 = 3
    };

private:
    Mode m_mode;
    float m_exposure;
    float m_gamma;

public:
    ToneMappingEffect();
    virtual ~ToneMappingEffect() = default;

    bool Initialize() override;
    void Apply(GLuint inputTexture, GLuint outputFBO, int width, int height) override;
    
    void SetMode(Mode mode) { m_mode = mode; }
    void SetExposure(float exposure) { m_exposure = exposure; }
    void SetGamma(float gamma) { m_gamma = gamma; }
    
    Mode GetMode() const { return m_mode; }
    float GetExposure() const { return m_exposure; }
    float GetGamma() const { return m_gamma; }
};


class BloomEffect : public PostProcessEffect {
private:
    std::unique_ptr<Shader> m_brightFilterShader;
    std::unique_ptr<Shader> m_blurShader;
    std::unique_ptr<Shader> m_combineShader;
    
    float m_threshold;
    int m_blurPasses;
    FramebufferManager* m_fbManager;

public:
    BloomEffect(FramebufferManager* fbManager);
    virtual ~BloomEffect() = default;

    bool Initialize() override;
    void Apply(GLuint inputTexture, GLuint outputFBO, int width, int height) override;
    
    void SetThreshold(float threshold) { m_threshold = threshold; }
    void SetBlurPasses(int passes) { m_blurPasses = passes; }
    
    float GetThreshold() const { return m_threshold; }
    int GetBlurPasses() const { return m_blurPasses; }
};


class FXAAEffect : public PostProcessEffect {
private:
    float m_lumaThreshold;
    float m_mulReduce;
    float m_minReduce;
    float m_maxSpan;

public:
    FXAAEffect();
    virtual ~FXAAEffect() = default;

    bool Initialize() override;
    void Apply(GLuint inputTexture, GLuint outputFBO, int width, int height) override;
    
    void SetLumaThreshold(float threshold) { m_lumaThreshold = threshold; }
    void SetReduceParameters(float mulReduce, float minReduce) { 
        m_mulReduce = mulReduce; 
        m_minReduce = minReduce; 
    }
    void SetMaxSpan(float maxSpan) { m_maxSpan = maxSpan; }
};


class PostProcessManager {
public:
    enum class PostProcessQuality {
        DISABLED = 0,
        BASIC = 1,
        ADVANCED = 2,
        ULTRA = 3
    };

private:
    std::unique_ptr<FramebufferManager> m_fbManager;
    std::unique_ptr<ToneMappingEffect> m_toneMappingEffect;
    std::unique_ptr<BloomEffect> m_bloomEffect;
    std::unique_ptr<FXAAEffect> m_fxaaEffect;
    
    PostProcessQuality m_quality;
    bool m_enabled;
    
    
    GLuint m_quadVAO;
    GLuint m_quadVBO;
    
    
    float m_postProcessTime;
    int m_frameWidth, m_frameHeight;

public:
    PostProcessManager();
    ~PostProcessManager();

    bool Initialize(int width, int height);
    void Cleanup();
    
    void BeginSceneRender();
    void EndSceneRender();
    void ApplyPostProcessing();
    
    void SetQuality(PostProcessQuality quality);
    void SetEnabled(bool enabled) { m_enabled = enabled; }
    
    PostProcessQuality GetQuality() const { return m_quality; }
    bool IsEnabled() const { return m_enabled; }
    
    
    ToneMappingEffect* GetToneMappingEffect() const { return m_toneMappingEffect.get(); }
    BloomEffect* GetBloomEffect() const { return m_bloomEffect.get(); }
    FXAAEffect* GetFXAAEffect() const { return m_fxaaEffect.get(); }
    
    
    float GetPostProcessTime() const { return m_postProcessTime; }
    
    
    void OnWindowResize(int width, int height);

private:
    void CreateFullscreenQuad();
    void RenderFullscreenQuad();
    GLuint CreateTempFramebuffer(int width, int height);
    void DeleteTempFramebuffer(GLuint fbo, GLuint texture);
};
