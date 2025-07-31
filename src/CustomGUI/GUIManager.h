#pragma once

#include "UIElement.h"
#include "UIButton.h"
#include "UIPanel.h"
#include "UIText.h"
#include "../Shader.h"
#include <vector>
#include <memory>
#include <map>

namespace CustomGUI {

    class GUIManager {
    private:
        static GUIManager* instance;
        
        std::vector<std::shared_ptr<UIElement>> rootElements;
        UIElement* focusedElement;
        UIElement* hoveredElement;
        
        
        std::unique_ptr<Shader> uiShader;
        unsigned int quadVAO, quadVBO, quadEBO;
        unsigned int screenWidth, screenHeight;
        
        
        glm::vec2 lastMousePos;
        bool mousePressed;

    public:
        GUIManager();
        ~GUIManager();
        
        
        static GUIManager* GetInstance();
        static void DestroyInstance();
        
        
        bool Initialize(unsigned int windowWidth, unsigned int windowHeight);
        void Shutdown();
        void Update(float deltaTime);
        void Render();
        
        
        void HandleMouseClick(double x, double y, bool pressed);
        void HandleMouseMove(double x, double y);
        void HandleKeyPress(int key, int scancode, int action, int mods);
        void HandleWindowResize(unsigned int width, unsigned int height);
        
        
        void AddElement(std::shared_ptr<UIElement> element);
        void RemoveElement(std::shared_ptr<UIElement> element);
        void RemoveElement(const std::string& elementId);
        void ClearElements();
        std::shared_ptr<UIElement> GetElementById(const std::string& elementId);  
        
        
        glm::vec2 ScreenToGUI(glm::vec2 screenPos) const;
        void SetFocusedElement(UIElement* element) { focusedElement = element; }
        UIElement* GetFocusedElement() const { return focusedElement; }
        
        
        unsigned int GetScreenWidth() const { return screenWidth; }
        unsigned int GetScreenHeight() const { return screenHeight; }
        
        void RenderQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
        
    private:
        void InitializeRenderingResources();
        void RenderElement(UIElement* element);
        void SortElementsByZOrder();
    };

} 
