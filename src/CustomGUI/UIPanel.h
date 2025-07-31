#pragma once

#include "UIElement.h"
#include <vector>
#include <memory>

namespace CustomGUI {

    class UIPanel : public UIElement {
    private:
        std::vector<std::shared_ptr<UIElement>> children;
        glm::vec4 backgroundColor;
        glm::vec4 borderColor;
        float borderWidth;
        bool hasBorder;

    public:
        UIPanel(const std::string& elementId = "");
        
        void Render() override;
        void Update(float deltaTime) override;
        bool HandleClick(glm::vec2 mousePos) override;
        bool HandleMouseMove(glm::vec2 mousePos) override;
        
        
        void AddChild(std::shared_ptr<UIElement> child);
        void RemoveChild(std::shared_ptr<UIElement> child);
        void RemoveChild(const std::string& childId);
        void ClearChildren();
        
        void SetBackgroundColor(const glm::vec4& color) { 
            backgroundColor = color; 
            SetColor(color);  
        }
        void SetBorderColor(const glm::vec4& color) { borderColor = color; }
        void SetBorderWidth(float width) { borderWidth = width; }
        void SetHasBorder(bool border) { hasBorder = border; }
        
        glm::vec4 GetBackgroundColor() const { return backgroundColor; }
        const std::vector<std::shared_ptr<UIElement>>& GetChildren() const { return children; }
        size_t GetChildCount() const { return children.size(); }
    };

} 
