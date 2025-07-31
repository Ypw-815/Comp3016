#include "PostProcessing.h"
#include "Shader.h"
#include <iostream>
#include <chrono>


FramebufferManager::FramebufferManager() : m_currentBuffer(0) {
}

FramebufferManager::~FramebufferManager() {
    Cleanup();
}

bool FramebufferManager::Initialize(int width, int height) {
    
    if (!CreateFramebuffer(m_sceneBuffer, width, height, true)) {
        std::cerr << "Failed to create scene framebuffer" << std::endl;
        return false;
    }

    
    for (int i = 0; i < 2; ++i) {
        if (!CreateFramebuffer(m_postProcessBuffer[i], width, height, false)) {
            std::cerr << "Failed to create post-process framebuffer " << i << std::endl;
            return false;
        }
    }

    
    int bloomWidth = width / 2;
    int bloomHeight = height / 2;
    for (int i = 0; i < 4; ++i) {
        if (!CreateFramebuffer(m_bloomBuffer[i], bloomWidth, bloomHeight, false)) {
            std::cerr << "Failed to create bloom framebuffer " << i << std::endl;
            return false;
        }
        bloomWidth /= 2;
        bloomHeight /= 2;
    }

    std::cout << "FramebufferManager initialized successfully" << std::endl;
    return true;
}

bool FramebufferManager::CreateFramebuffer(FramebufferData& fb, int width, int height, bool needsNormal) {
    fb.width = width;
    fb.height = height;

    
    glGenFramebuffers(1, &fb.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);

    
    glGenTextures(1, &fb.colorTexture);
    glBindTexture(GL_TEXTURE_2D, fb.colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.colorTexture, 0);

    
    glGenTextures(1, &fb.depthTexture);
    glBindTexture(GL_TEXTURE_2D, fb.depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb.depthTexture, 0);

    
    if (needsNormal) {
        glGenTextures(1, &fb.normalTexture);
        glBindTexture(GL_TEXTURE_2D, fb.normalTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fb.normalTexture, 0);

        
        GLenum drawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, drawBuffers);
    }

    
    if (!CheckFramebufferComplete(fb.fbo)) {
        DeleteFramebuffer(fb);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void FramebufferManager::DeleteFramebuffer(FramebufferData& fb) {
    if (fb.fbo) {
        glDeleteFramebuffers(1, &fb.fbo);
        fb.fbo = 0;
    }
    if (fb.colorTexture) {
        glDeleteTextures(1, &fb.colorTexture);
        fb.colorTexture = 0;
    }
    if (fb.depthTexture) {
        glDeleteTextures(1, &fb.depthTexture);
        fb.depthTexture = 0;
    }
    if (fb.normalTexture) {
        glDeleteTextures(1, &fb.normalTexture);
        fb.normalTexture = 0;
    }
}

void FramebufferManager::Cleanup() {
    DeleteFramebuffer(m_sceneBuffer);
    for (int i = 0; i < 2; ++i) {
        DeleteFramebuffer(m_postProcessBuffer[i]);
    }
    for (int i = 0; i < 4; ++i) {
        DeleteFramebuffer(m_bloomBuffer[i]);
    }
}

void FramebufferManager::BindSceneFramebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_sceneBuffer.fbo);
    glViewport(0, 0, m_sceneBuffer.width, m_sceneBuffer.height);
}

void FramebufferManager::BindPostProcessFramebuffer(int index) {
    if (index >= 0 && index < 2) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_postProcessBuffer[index].fbo);
        glViewport(0, 0, m_postProcessBuffer[index].width, m_postProcessBuffer[index].height);
        m_currentBuffer = index;
    }
}

void FramebufferManager::BindDefaultFramebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint FramebufferManager::GetPostProcessTexture(int index) const {
    if (index >= 0 && index < 2) {
        return m_postProcessBuffer[index].colorTexture;
    }
    return 0;
}

GLuint FramebufferManager::GetBloomTexture(int level) const {
    if (level >= 0 && level < 4) {
        return m_bloomBuffer[level].colorTexture;
    }
    return 0;
}

bool FramebufferManager::CheckFramebufferComplete(GLuint fbo) const {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer incomplete: ";
        switch (status) {
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << std::endl;
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            std::cerr << "GL_FRAMEBUFFER_UNSUPPORTED" << std::endl;
            break;
        default:
            std::cerr << "Unknown error " << status << std::endl;
            break;
        }
        return false;
    }
    return true;
}

void FramebufferManager::ResizeFramebuffers(int width, int height) {
    
    Cleanup();
    
    Initialize(width, height);
}


ToneMappingEffect::ToneMappingEffect() 
    : m_mode(Mode::REINHARD), m_exposure(1.0f), m_gamma(2.2f) {
}

bool ToneMappingEffect::Initialize() {
    try {
        m_shader = std::make_unique<Shader>("resources/shaders/postprocess/tonemapping.vert", 
                                           "resources/shaders/postprocess/tonemapping.frag");
        std::cout << "ToneMappingEffect initialized successfully" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize ToneMappingEffect: " << e.what() << std::endl;
        return false;
    }
}

void ToneMappingEffect::Apply(GLuint inputTexture, GLuint outputFBO, int width, int height) {
    if (!m_enabled || !m_shader) return;

    glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);
    glViewport(0, 0, width, height);
    
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    
    m_shader->Use();
    m_shader->SetInt("screenTexture", 0);
    m_shader->SetInt("toneMappingMode", static_cast<int>(m_mode));
    m_shader->SetFloat("exposure", m_exposure);
    m_shader->SetFloat("gamma", m_gamma);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
}


BloomEffect::BloomEffect(FramebufferManager* fbManager) 
    : m_fbManager(fbManager), m_threshold(1.0f), m_blurPasses(5) {
}

bool BloomEffect::Initialize() {
    try {
        m_brightFilterShader = std::make_unique<Shader>("resources/shaders/postprocess/fullscreen.vert", 
                                                        "resources/shaders/postprocess/bright_filter.frag");
        m_blurShader = std::make_unique<Shader>("resources/shaders/postprocess/fullscreen.vert", 
                                               "resources/shaders/postprocess/gaussian_blur.frag");
        m_combineShader = std::make_unique<Shader>("resources/shaders/postprocess/fullscreen.vert", 
                                                  "resources/shaders/postprocess/bloom_combine.frag");
        std::cout << "BloomEffect initialized successfully" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize BloomEffect: " << e.what() << std::endl;
        return false;
    }
}

void BloomEffect::Apply(GLuint inputTexture, GLuint outputFBO, int width, int height) {
    if (!m_enabled || !m_brightFilterShader || !m_blurShader || !m_combineShader) return;

    glDisable(GL_DEPTH_TEST);
    
    
    
    
    
    
    
    
    glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);
    glViewport(0, 0, width, height);
    
    m_combineShader->Use();
    m_combineShader->SetInt("sceneTexture", 0);
    m_combineShader->SetInt("bloomTexture", 1);
    m_combineShader->SetFloat("bloomIntensity", m_intensity);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
}


FXAAEffect::FXAAEffect() 
    : m_lumaThreshold(0.5f), m_mulReduce(8.0f), m_minReduce(128.0f), m_maxSpan(8.0f) {
}

bool FXAAEffect::Initialize() {
    try {
        m_shader = std::make_unique<Shader>("resources/shaders/postprocess/fullscreen.vert", 
                                           "resources/shaders/postprocess/fxaa.frag");
        std::cout << "FXAAEffect initialized successfully" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize FXAAEffect: " << e.what() << std::endl;
        return false;
    }
}

void FXAAEffect::Apply(GLuint inputTexture, GLuint outputFBO, int width, int height) {
    if (!m_enabled || !m_shader) return;

    glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);
    glViewport(0, 0, width, height);
    
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    
    m_shader->Use();
    m_shader->SetInt("screenTexture", 0);
    m_shader->SetFloat("lumaThreshold", m_lumaThreshold);
    m_shader->SetFloat("mulReduce", m_mulReduce);
    m_shader->SetFloat("minReduce", m_minReduce);
    m_shader->SetFloat("maxSpan", m_maxSpan);
    m_shader->SetVec2("texelStep", glm::vec2(1.0f / width, 1.0f / height));
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
}


PostProcessManager::PostProcessManager() 
    : m_quality(PostProcessQuality::BASIC), m_enabled(true), 
      m_quadVAO(0), m_quadVBO(0), m_postProcessTime(0.0f),
      m_frameWidth(0), m_frameHeight(0) {
}

PostProcessManager::~PostProcessManager() {
    Cleanup();
}

bool PostProcessManager::Initialize(int width, int height) {
    m_frameWidth = width;
    m_frameHeight = height;

    
    m_fbManager = std::make_unique<FramebufferManager>();
    if (!m_fbManager->Initialize(width, height)) {
        std::cerr << "Failed to initialize FramebufferManager" << std::endl;
        return false;
    }

    
    CreateFullscreenQuad();

    
    m_toneMappingEffect = std::make_unique<ToneMappingEffect>();
    if (!m_toneMappingEffect->Initialize()) {
        std::cerr << "Failed to initialize ToneMappingEffect" << std::endl;
        return false;
    }

    m_bloomEffect = std::make_unique<BloomEffect>(m_fbManager.get());
    if (!m_bloomEffect->Initialize()) {
        std::cerr << "Failed to initialize BloomEffect" << std::endl;
        return false;
    }

    m_fxaaEffect = std::make_unique<FXAAEffect>();
    if (!m_fxaaEffect->Initialize()) {
        std::cerr << "Failed to initialize FXAAEffect" << std::endl;
        return false;
    }

    std::cout << "PostProcessManager initialized successfully" << std::endl;
    return true;
}

void PostProcessManager::CreateFullscreenQuad() {
    float quadVertices[] = {
        
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void PostProcessManager::RenderFullscreenQuad() {
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void PostProcessManager::BeginSceneRender() {
    if (!m_enabled || !m_fbManager) return;
    m_fbManager->BindSceneFramebuffer();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcessManager::EndSceneRender() {
    if (!m_enabled || !m_fbManager) return;
    m_fbManager->BindDefaultFramebuffer();
}

void PostProcessManager::ApplyPostProcessing() {
    if (!m_enabled || m_quality == PostProcessQuality::DISABLED) {
        
        
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();

    GLuint currentTexture = m_fbManager->GetSceneColorTexture();
    int currentBuffer = 0;

    
    switch (m_quality) {
    case PostProcessQuality::BASIC:
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0); 
        glViewport(0, 0, m_frameWidth, m_frameHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_toneMappingEffect->Apply(currentTexture, 0, m_frameWidth, m_frameHeight);
        RenderFullscreenQuad();
        break;

    case PostProcessQuality::ADVANCED:
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, m_frameWidth, m_frameHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_toneMappingEffect->Apply(currentTexture, 0, m_frameWidth, m_frameHeight);
        RenderFullscreenQuad();
        break;

    case PostProcessQuality::ULTRA:
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, m_frameWidth, m_frameHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_toneMappingEffect->Apply(currentTexture, 0, m_frameWidth, m_frameHeight);
        RenderFullscreenQuad();
        break;
    }

    auto end = std::chrono::high_resolution_clock::now();
    m_postProcessTime = std::chrono::duration<float, std::milli>(end - start).count();
}

void PostProcessManager::SetQuality(PostProcessQuality quality) {
    m_quality = quality;
    std::cout << "Post-Processing Quality: ";
    switch (quality) {
    case PostProcessQuality::DISABLED: std::cout << "DISABLED"; break;
    case PostProcessQuality::BASIC: std::cout << "BASIC"; break;
    case PostProcessQuality::ADVANCED: std::cout << "ADVANCED"; break;
    case PostProcessQuality::ULTRA: std::cout << "ULTRA"; break;
    }
    std::cout << std::endl;
}

void PostProcessManager::OnWindowResize(int width, int height) {
    m_frameWidth = width;
    m_frameHeight = height;
    if (m_fbManager) {
        m_fbManager->ResizeFramebuffers(width, height);
    }
}

void PostProcessManager::Cleanup() {
    if (m_quadVAO) {
        glDeleteVertexArrays(1, &m_quadVAO);
        m_quadVAO = 0;
    }
    if (m_quadVBO) {
        glDeleteBuffers(1, &m_quadVBO);
        m_quadVBO = 0;
    }
    
    m_fbManager.reset();
    m_toneMappingEffect.reset();
    m_bloomEffect.reset();
    m_fxaaEffect.reset();
}
