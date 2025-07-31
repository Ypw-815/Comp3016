#include "ShadowMapping.h"
#include "Shader.h"
#include "PerformanceProfiler.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>





ShadowMapping::ShadowMapping(ShadowQuality quality, FilterMode filterMode)
    : m_shadowMapFBO(0)
    , m_shadowMap(0)
    , m_shadowMapSize(static_cast<int>(quality))
    , m_quality(quality)
    , m_filterMode(filterMode)
    , m_lightType(LightType::DIRECTIONAL)
    , m_lightPosition(0.0f, 10.0f, 0.0f)
    , m_lightDirection(0.0f, -1.0f, 0.0f)
    , m_lightTarget(0.0f, 0.0f, 0.0f)
    , m_lightFOV(90.0f)
    , m_nearPlane(1.0f)
    , m_farPlane(100.0f)
    , m_lightSpaceMatrix(1.0f)
    , m_lightProjection(1.0f)
    , m_lightView(1.0f)
    , m_depthBias(0.005f)
    , m_normalBias(0.05f)
    , m_enableSlopeScaledBias(true)
    , m_enableDebugView(false)
    , m_enableCulling(true)
{
    std::cout << "[ShadowMapping] Initialized with quality: " << m_shadowMapSize << "x" << m_shadowMapSize << std::endl;
}

ShadowMapping::~ShadowMapping() {
    Cleanup();
}

bool ShadowMapping::Initialize() {
    PROFILE_FUNCTION();
    
    std::cout << "[ShadowMapping] Initializing shadow mapping system..." << std::endl;
    
    
    if (!CreateShadowMapFBO()) {
        std::cerr << "[ShadowMapping] Failed to create shadow map FBO" << std::endl;
        return false;
    }
    
    
    if (!LoadShaders()) {
        std::cerr << "[ShadowMapping] Failed to load shadow shaders" << std::endl;
        return false;
    }
    
    std::cout << "[ShadowMapping] Shadow mapping system initialized successfully" << std::endl;
    return true;
}

void ShadowMapping::Cleanup() {
    if (m_shadowMapFBO != 0) {
        glDeleteFramebuffers(1, &m_shadowMapFBO);
        m_shadowMapFBO = 0;
    }
    
    if (m_shadowMap != 0) {
        glDeleteTextures(1, &m_shadowMap);
        m_shadowMap = 0;
    }
    
    m_shadowMapShader.reset();
    m_shadowReceiveShader.reset();
    
    std::cout << "[ShadowMapping] Cleanup completed" << std::endl;
}

bool ShadowMapping::CreateShadowMapFBO() {
    PROFILE_FUNCTION();
    
    
    glGenFramebuffers(1, &m_shadowMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFBO);
    
    
    glGenTextures(1, &m_shadowMap);
    glBindTexture(GL_TEXTURE_2D, m_shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 
                 m_shadowMapSize, m_shadowMapSize, 0, 
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    
    
    switch (m_filterMode) {
        case FilterMode::NEAREST:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case FilterMode::LINEAR:
        case FilterMode::PCF_2x2:
        case FilterMode::PCF_3x3:
        case FilterMode::PCF_5x5:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
    }
    
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    
    if (m_filterMode >= FilterMode::PCF_2x2) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }
    
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMap, 0);
    
    
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[ShadowMapping] Framebuffer not complete: " << status << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    std::cout << "[ShadowMapping] Shadow map FBO created successfully (" << m_shadowMapSize << "x" << m_shadowMapSize << ")" << std::endl;
    return true;
}

void ShadowMapping::SetLight(LightType type, const glm::vec3& position, 
                           const glm::vec3& direction, float fov) {
    m_lightType = type;
    m_lightPosition = position;
    m_lightDirection = normalize(direction);
    m_lightFOV = fov;
    
    
    if (type == LightType::DIRECTIONAL) {
        m_lightTarget = m_lightPosition + m_lightDirection * 50.0f;
    } else {
        m_lightTarget = m_lightPosition + m_lightDirection * 10.0f;
    }
    
    
    CalculateLightSpaceMatrix();
    
    std::cout << "[ShadowMapping] Light configured - Type: " << static_cast<int>(type) 
              << ", Position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
}

void ShadowMapping::SetShadowRange(float nearPlane, float farPlane) {
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    CalculateLightSpaceMatrix();
}

void ShadowMapping::CalculateLightSpaceMatrix() {
    PROFILE_FUNCTION();
    
    
    glm::vec3 up = abs(dot(m_lightDirection, glm::vec3(0, 1, 0))) > 0.99f ? 
                   glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
    m_lightView = lookAt(m_lightPosition, m_lightTarget, up);
    
    
    switch (m_lightType) {
        case LightType::DIRECTIONAL: {
            
            float orthoSize = 50.0f; 
            m_lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 
                                     m_nearPlane, m_farPlane);
            break;
        }
        case LightType::POINT:
        case LightType::SPOT: {
            
            m_lightProjection = glm::perspective(glm::radians(m_lightFOV), 1.0f, m_nearPlane, m_farPlane);
            break;
        }
    }
    
    
    m_lightSpaceMatrix = m_lightProjection * m_lightView;
    
    
    if (m_enableCulling) {
        CalculateFrustumPlanes();
    }
}

void ShadowMapping::CalculateFrustumPlanes() {
    
    glm::mat4 combined = m_lightProjection * m_lightView;
    
    
    m_frustumPlanes[0] = glm::vec4(
        combined[0][3] + combined[0][0],
        combined[1][3] + combined[1][0],
        combined[2][3] + combined[2][0],
        combined[3][3] + combined[3][0]
    );
    
    
    m_frustumPlanes[1] = glm::vec4(
        combined[0][3] - combined[0][0],
        combined[1][3] - combined[1][0],
        combined[2][3] - combined[2][0],
        combined[3][3] - combined[3][0]
    );
    
    
    m_frustumPlanes[2] = glm::vec4(
        combined[0][3] + combined[0][1],
        combined[1][3] + combined[1][1],
        combined[2][3] + combined[2][1],
        combined[3][3] + combined[3][1]
    );
    
    
    m_frustumPlanes[3] = glm::vec4(
        combined[0][3] - combined[0][1],
        combined[1][3] - combined[1][1],
        combined[2][3] - combined[2][1],
        combined[3][3] - combined[3][1]
    );
    
    
    m_frustumPlanes[4] = glm::vec4(
        combined[0][3] + combined[0][2],
        combined[1][3] + combined[1][2],
        combined[2][3] + combined[2][2],
        combined[3][3] + combined[3][2]
    );
    
    
    m_frustumPlanes[5] = glm::vec4(
        combined[0][3] - combined[0][2],
        combined[1][3] - combined[1][2],
        combined[2][3] - combined[2][2],
        combined[3][3] - combined[3][2]
    );
    
    
    for (int i = 0; i < 6; i++) {
        float length = sqrt(m_frustumPlanes[i].x * m_frustumPlanes[i].x +
                           m_frustumPlanes[i].y * m_frustumPlanes[i].y +
                           m_frustumPlanes[i].z * m_frustumPlanes[i].z);
        m_frustumPlanes[i] /= length;
    }
}

bool ShadowMapping::IsInLightFrustum(const glm::vec3& center, float radius) {
    if (!m_enableCulling) return true;
    
    
    for (int i = 0; i < 6; i++) {
        float distance = m_frustumPlanes[i].x * center.x +
                        m_frustumPlanes[i].y * center.y +
                        m_frustumPlanes[i].z * center.z +
                        m_frustumPlanes[i].w;
        
        if (distance < -radius) {
            return false; 
        }
    }
    
    return true; 
}

void ShadowMapping::BeginShadowMapPass() {
    PROFILE_SECTION("Shadow Map Rendering");
    
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFBO);
    glViewport(0, 0, m_shadowMapSize, m_shadowMapSize);
    
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT); 
    
    
    if (m_enableSlopeScaledBias) {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(2.0f, m_depthBias);
    }
    
    
    if (m_shadowMapShader) {
        m_shadowMapShader->use();
        m_shadowMapShader->SetMat4("lightSpaceMatrix", m_lightSpaceMatrix);
    }
}

void ShadowMapping::EndShadowMapPass(int windowWidth, int windowHeight) {
    if (m_enableSlopeScaledBias) {
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, windowWidth, windowHeight);
}

void ShadowMapping::BeginShadowReceivePass(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) {
    
    
}

void ShadowMapping::BindShadowMap(int textureUnit) {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_shadowMap);
}

void ShadowMapping::SetShadowUniforms(Shader* shader) {
    if (!shader) return;
    
    shader->SetMat4("lightSpaceMatrix", m_lightSpaceMatrix);
    shader->SetVec3("lightPos", m_lightPosition);
    shader->SetVec3("lightDir", m_lightDirection);
    shader->SetFloat("shadowBias", m_depthBias);
    shader->SetFloat("normalBias", m_normalBias);
    shader->SetInt("shadowMap", 0); 
    shader->SetInt("filterMode", static_cast<int>(m_filterMode));
    shader->SetFloat("shadowMapSize", static_cast<float>(m_shadowMapSize));
}

void ShadowMapping::SetShadowQuality(ShadowQuality quality) {
    if (quality != m_quality) {
        m_quality = quality;
        int newSize = static_cast<int>(quality);
        
        if (newSize != m_shadowMapSize) {
            m_shadowMapSize = newSize;
            
            
            if (m_shadowMap != 0) {
                glDeleteTextures(1, &m_shadowMap);
                glDeleteFramebuffers(1, &m_shadowMapFBO);
                CreateShadowMapFBO();
            }
            
            std::cout << "[ShadowMapping] Shadow quality updated to: " << m_shadowMapSize << "x" << m_shadowMapSize << std::endl;
        }
    }
}

void ShadowMapping::SetFilterMode(FilterMode mode) {
    if (mode != m_filterMode) {
        m_filterMode = mode;
        
        
        if (m_shadowMap != 0) {
            glBindTexture(GL_TEXTURE_2D, m_shadowMap);
            
            switch (m_filterMode) {
                case FilterMode::NEAREST:
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
                    break;
                case FilterMode::LINEAR:
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
                    break;
                case FilterMode::PCF_2x2:
                case FilterMode::PCF_3x3:
                case FilterMode::PCF_5x5:
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
                    break;
            }
        }
        
        std::cout << "[ShadowMapping] Filter mode updated to: " << static_cast<int>(mode) << std::endl;
    }
}

void ShadowMapping::RenderDebugQuad() {
    if (!m_enableDebugView) return;
    
    
    
    std::cout << "[ShadowMapping] Debug: Shadow map size: " << m_shadowMapSize 
              << ", Light position: (" << m_lightPosition.x << ", " << m_lightPosition.y << ", " << m_lightPosition.z << ")" << std::endl;
}

ShadowMapping::ShadowStats ShadowMapping::GetPerformanceStats() const {
    ShadowStats stats;
    stats.shadowMapSize = m_shadowMapSize;
    stats.renderedObjects = 0; 
    stats.culledObjects = 0;   
    stats.shadowMapRenderTime = 0.0f; 
    stats.totalShadowTime = 0.0f;     
    return stats;
}

bool ShadowMapping::LoadShaders() {
    
    
    std::cout << "[ShadowMapping] Shader loading placeholder - should load actual shadow map shaders" << std::endl;
    return true;
}





ShadowMappingManager::ShadowMappingManager()
    : m_globalShadowsEnabled(true)
    , m_globalShadowStrength(1.0f)
{
    std::cout << "[ShadowMappingManager] Shadow mapping manager initialized" << std::endl;
}

ShadowMappingManager::~ShadowMappingManager() {
    m_shadowMaps.clear();
    std::cout << "[ShadowMappingManager] Shadow mapping manager destroyed" << std::endl;
}

ShadowMapping* ShadowMappingManager::AddShadowLight(int lightIndex, 
                                                   ShadowMapping::LightType type,
                                                   ShadowMapping::ShadowQuality quality) {
    
    if (lightIndex >= static_cast<int>(m_shadowMaps.size())) {
        m_shadowMaps.resize(lightIndex + 1);
    }
    
    
    auto shadowMap = std::make_unique<ShadowMapping>(quality, ShadowMapping::FilterMode::PCF_3x3);
    if (!shadowMap->Initialize()) {
        std::cerr << "[ShadowMappingManager] Failed to initialize shadow mapping for light " << lightIndex << std::endl;
        return nullptr;
    }
    
    ShadowMapping* result = shadowMap.get();
    m_shadowMaps[lightIndex] = std::move(shadowMap);
    
    std::cout << "[ShadowMappingManager] Added shadow light " << lightIndex << " (type: " << static_cast<int>(type) << ")" << std::endl;
    return result;
}

void ShadowMappingManager::RemoveShadowLight(int lightIndex) {
    if (lightIndex >= 0 && lightIndex < static_cast<int>(m_shadowMaps.size())) {
        m_shadowMaps[lightIndex].reset();
        std::cout << "[ShadowMappingManager] Removed shadow light " << lightIndex << std::endl;
    }
}

ShadowMapping* ShadowMappingManager::GetShadowMapping(int lightIndex) {
    if (lightIndex >= 0 && lightIndex < static_cast<int>(m_shadowMaps.size())) {
        return m_shadowMaps[lightIndex].get();
    }
    return nullptr;
}

void ShadowMappingManager::RenderAllShadowMaps(int windowWidth, int windowHeight) {
    if (!m_globalShadowsEnabled) return;
    
    PROFILE_SECTION("All Shadow Maps Rendering");
    
    for (auto& shadowMap : m_shadowMaps) {
        if (shadowMap) {
            shadowMap->BeginShadowMapPass();
            
            
            shadowMap->EndShadowMapPass(windowWidth, windowHeight);
        }
    }
}

void ShadowMappingManager::SetupShadowReceive(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) {
    if (!m_globalShadowsEnabled) return;
    
    for (auto& shadowMap : m_shadowMaps) {
        if (shadowMap) {
            shadowMap->BeginShadowReceivePass(viewMatrix, projMatrix);
        }
    }
}

void ShadowMappingManager::BindAllShadowMaps(int startTextureUnit) {
    if (!m_globalShadowsEnabled) return;
    
    int textureUnit = startTextureUnit;
    for (auto& shadowMap : m_shadowMaps) {
        if (shadowMap) {
            shadowMap->BindShadowMap(textureUnit);
            textureUnit++;
        }
    }
}
