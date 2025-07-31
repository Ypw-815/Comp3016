#include "GUIManager.h"
#include "UIText.h"
#include <glad/glad.h>
#include <iostream>
#include <algorithm>

namespace CustomGUI {

    GUIManager* GUIManager::instance = nullptr;

    GUIManager::GUIManager()
        : focusedElement(nullptr)
        , hoveredElement(nullptr)
        , quadVAO(0)
        , quadVBO(0)
        , quadEBO(0)
        , screenWidth(800)
        , screenHeight(600)
        , lastMousePos(0.0f, 0.0f)
        , mousePressed(false)
    {
    }

    GUIManager::~GUIManager() {
        Shutdown();
    }

    GUIManager* GUIManager::GetInstance() {
        if (!instance) {
            instance = new GUIManager();
        }
        return instance;
    }

    void GUIManager::DestroyInstance() {
        if (instance) {
            delete instance;
            instance = nullptr;
        }
    }

    bool GUIManager::Initialize(unsigned int windowWidth, unsigned int windowHeight) {
        screenWidth = windowWidth;
        screenHeight = windowHeight;
        
        std::cout << "Initializing CustomGUI system..." << std::endl;
        std::cout << "Screen resolution: " << screenWidth << "x" << screenHeight << std::endl;
        
        try {
            InitializeRenderingResources();
            
            
            uiShader = std::make_unique<Shader>("resources/shaders/ui.vert", "resources/shaders/ui.frag");
            
            
            if (!UIText::InitializeFontSystem()) {
                std::cerr << "Warning: Failed to initialize font rendering system" << std::endl;
            }
            
            
            glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(screenWidth), 
                                            static_cast<float>(screenHeight), 0.0f, 
                                            -1.0f, 1.0f);
            UIText::SetFontProjection(projection);
            
            std::cout << "✓ CustomGUI system initialized successfully!" << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Failed to initialize CustomGUI: " << e.what() << std::endl;
            return false;
        }
    }

    void GUIManager::Shutdown() {
        std::cout << "Shutting down CustomGUI system..." << std::endl;
        
        
        if (quadVAO != 0) {
            glDeleteVertexArrays(1, &quadVAO);
            quadVAO = 0;
        }
        if (quadVBO != 0) {
            glDeleteBuffers(1, &quadVBO);
            quadVBO = 0;
        }
        if (quadEBO != 0) {
            glDeleteBuffers(1, &quadEBO);
            quadEBO = 0;
        }
        
        
        ClearElements();
        
        
        UIText::ShutdownFontSystem();
        
        
        focusedElement = nullptr;
        hoveredElement = nullptr;
        
        std::cout << "CustomGUI system shut down." << std::endl;
    }

    void GUIManager::Update(float deltaTime) {
        
        for (auto& element : rootElements) {
            if (element) {
                element->Update(deltaTime);
            }
        }
    }

    void GUIManager::Render() {
        if (rootElements.empty()) {
            std::cout << "[GUI] No elements to render" << std::endl;
            return;
        }
        
        if (!uiShader) {
            std::cout << "[GUI] ERROR: UI shader not initialized!" << std::endl;
            return;
        }

        static bool debugOnce = false;
        if (!debugOnce) {
            std::cout << "[GUI] Starting render - Elements: " << rootElements.size() << std::endl;
            std::cout << "[GUI] Screen size: " << screenWidth << "x" << screenHeight << std::endl;
            debugOnce = true;
        }

        
        GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
        GLboolean blendEnabled = glIsEnabled(GL_BLEND);
        GLint blendSrc, blendDst;
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrc);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDst);

        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        
        glDisable(GL_DEPTH_TEST);
        
        while (glGetError() != GL_NO_ERROR);
        
        glUseProgram(0);
        
        
        uiShader->Use();
        
        
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "[GUI] OpenGL error after shader use: " << error << std::endl;
        }
        
        
        glm::mat4 projection = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);
        GLint projectionLoc = glGetUniformLocation(uiShader->ID, "projection");
        if (projectionLoc != -1) {
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        }
        GLint useTextureLoc = glGetUniformLocation(uiShader->ID, "useTexture");
        if (useTextureLoc != -1) {
            glUniform1i(useTextureLoc, 0);
        }
        
        
        for (auto& element : rootElements) {
            if (element && element->IsVisible()) {
                RenderElement(element.get());
            }
        }
        
        
        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "[GUI] OpenGL error after rendering: " << error << std::endl;
        }
        
        
        if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
        if (!blendEnabled) glDisable(GL_BLEND);
        else glBlendFunc(blendSrc, blendDst);
    }

    void GUIManager::HandleMouseClick(double x, double y, bool pressed) {
        glm::vec2 guiPos = ScreenToGUI(glm::vec2(x, y));
        mousePressed = pressed;
        
        if (pressed) {
            
            for (auto it = rootElements.rbegin(); it != rootElements.rend(); ++it) {
                auto& element = *it;
                if (element && element->IsInteractive()) {
                    if (element->HandleClick(guiPos)) {
                        SetFocusedElement(element.get());
                        break; 
                    }
                }
            }
        }
    }

    void GUIManager::HandleMouseMove(double x, double y) {
        glm::vec2 guiPos = ScreenToGUI(glm::vec2(x, y));
        lastMousePos = guiPos;
        
        
        hoveredElement = nullptr;
        for (auto it = rootElements.rbegin(); it != rootElements.rend(); ++it) {
            auto& element = *it;
            if (element && element->IsInteractive()) {
                if (element->HandleMouseMove(guiPos)) {
                    hoveredElement = element.get();
                    break; 
                }
            }
        }
    }

    void GUIManager::HandleKeyPress(int key, int scancode, int action, int mods) {
        
        
    }

    void GUIManager::HandleWindowResize(unsigned int width, unsigned int height) {
        screenWidth = width;
        screenHeight = height;
        std::cout << "GUI window resized to: " << width << "x" << height << std::endl;
    }

    void GUIManager::AddElement(std::shared_ptr<UIElement> element) {
        if (element) {
            rootElements.push_back(element);
            SortElementsByZOrder();
        }
    }

    void GUIManager::RemoveElement(std::shared_ptr<UIElement> element) {
        rootElements.erase(
            std::remove(rootElements.begin(), rootElements.end(), element),
            rootElements.end()
        );
    }

    void GUIManager::RemoveElement(const std::string& elementId) {
        rootElements.erase(
            std::remove_if(rootElements.begin(), rootElements.end(),
                [&elementId](const std::shared_ptr<UIElement>& element) {
                    return element && element->GetId() == elementId;
                }),
            rootElements.end()
        );
    }

    void GUIManager::ClearElements() {
        rootElements.clear();
        focusedElement = nullptr;
        hoveredElement = nullptr;
    }

    std::shared_ptr<UIElement> GUIManager::GetElementById(const std::string& elementId) {
        for (auto& element : rootElements) {
            if (element && element->GetId() == elementId) {
                return element;
            }
        }
        return nullptr;
    }

    glm::vec2 GUIManager::ScreenToGUI(glm::vec2 screenPos) const {
        
        
        
        return screenPos;
    }

    void GUIManager::InitializeRenderingResources() {
        
        float vertices[] = {
            
            0.0f, 1.0f,         0.0f, 1.0f,   
            1.0f, 1.0f,         1.0f, 1.0f,   
            1.0f, 0.0f,         1.0f, 0.0f,   
            0.0f, 0.0f,         0.0f, 0.0f    
        };
        
        unsigned int indices[] = {
            0, 1, 2,  
            0, 2, 3   
        };
        
        
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glGenBuffers(1, &quadEBO);
        
        glBindVertexArray(quadVAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        glBindVertexArray(0);
        
        std::cout << "GUI rendering resources initialized." << std::endl;
    }

    void GUIManager::RenderQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
        if (!uiShader || quadVAO == 0) {
            std::cout << "[GUI] ERROR: RenderQuad failed - shader: " << (uiShader ? "OK" : "NULL") 
                      << " VAO: " << quadVAO << std::endl;
            return;
        }
        
        std::cout << "[GUI] RenderQuad: pos(" << position.x << "," << position.y << ") "
                  << "size(" << size.x << "," << size.y << ") "
                  << "color(" << color.r << "," << color.g << "," << color.b << "," << color.a << ")" << std::endl;
        
        GLint savedVAO, savedProgram, savedEBO;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &savedVAO);
        glGetIntegerv(GL_CURRENT_PROGRAM, &savedProgram);
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &savedEBO);
        
        while (glGetError() != GL_NO_ERROR);
        
        glm::mat4 projection = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);

        if (savedProgram != (GLint)uiShader->ID) {
            uiShader->Use();
            
            GLint projectionLoc = glGetUniformLocation(uiShader->ID, "projection");
            if (projectionLoc != -1) {
                glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
            }
        }
        
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "[GUI] OpenGL error after shader check: " << error << std::endl;
            return; 
        }
        
        GLint positionLoc = glGetUniformLocation(uiShader->ID, "position");
        GLint sizeLoc = glGetUniformLocation(uiShader->ID, "size");
        GLint colorLoc = glGetUniformLocation(uiShader->ID, "color");
        GLint useTextureLoc = glGetUniformLocation(uiShader->ID, "useTexture");
        
        if (positionLoc != -1) glUniform2f(positionLoc, position.x, position.y);
        if (sizeLoc != -1) glUniform2f(sizeLoc, size.x, size.y);
        if (colorLoc != -1) glUniform4f(colorLoc, color.r, color.g, color.b, color.a);
        if (useTextureLoc != -1) glUniform1i(useTextureLoc, 0);
        
        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "[GUI] OpenGL error after setting uniforms: " << error << std::endl;
            return; 
        }
        
        glBindVertexArray(quadVAO);
        if (savedEBO != quadEBO) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
        }
        
        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "[GUI] OpenGL error after binding: " << error << std::endl;
            glBindVertexArray(savedVAO);
            if (savedEBO != quadEBO) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, savedEBO);
            }
            return;
        }
        
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "[GUI] OpenGL error in DrawElements: " << error << std::endl;
        }
        
        if (savedEBO != quadEBO) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, savedEBO);
        }
        glBindVertexArray(savedVAO);
    }

    void GUIManager::RenderElement(UIElement* element) {
        if (!element || !element->IsVisible()) return;
        
        
        switch (element->GetType()) {
            case UIElementType::PANEL:
                RenderQuad(element->GetPosition(), element->GetSize(), element->GetColor());
                break;
                
            case UIElementType::BUTTON:
                
                std::cout << "[DEBUG_EXIT_BUTTON] Processing button: " << element->GetId() 
                          << " at position(" << element->GetPosition().x << "," << element->GetPosition().y 
                          << ") size(" << element->GetSize().x << "," << element->GetSize().y 
                          << ") color(" << element->GetColor().r << "," << element->GetColor().g 
                          << "," << element->GetColor().b << "," << element->GetColor().a << ")" << std::endl;
                std::cout << "[DEBUG_EXIT_BUTTON] About to call RenderQuad for button: " << element->GetId() << std::endl;
                RenderQuad(element->GetPosition(), element->GetSize(), element->GetColor());
                std::cout << "[DEBUG_EXIT_BUTTON] RenderQuad called successfully for button: " << element->GetId() << std::endl;
                
                element->Render();
                break;
                
            case UIElementType::TEXT:
                
                element->Render();
                
                break;
                
            default:
                RenderQuad(element->GetPosition(), element->GetSize(), element->GetColor());
                break;
        }
        
        
        if (element->GetType() == UIElementType::PANEL) {
            UIPanel* panel = static_cast<UIPanel*>(element);
            for (const auto& child : panel->GetChildren()) {
                RenderElement(child.get());
            }
        }
    }

    void GUIManager::SortElementsByZOrder() {
        std::sort(rootElements.begin(), rootElements.end(),
            [](const std::shared_ptr<UIElement>& a, const std::shared_ptr<UIElement>& b) {
                return a->GetZOrder() < b->GetZOrder();
            });
    }

} 
