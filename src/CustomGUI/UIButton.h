#pragma once

#include "UIElement.h"
#include <functional>

namespace CustomGUI {

    enum class ButtonState {
        NORMAL,
        HOVERED,
        PRESSED
    };

    class UIButton : public UIElement {
    private:
        std::string text;
        ButtonState state;
        std::function<void()> onClick;
        
        
        glm::vec4 normalColor;
        glm::vec4 hoverColor;
        glm::vec4 pressedColor;
        glm::vec4 textColor;
        
        float fontSize;

    public:
        UIButton(const std::string& buttonText = "Button", const std::string& elementId = "");
        
        void Render() override;
        void Update(float deltaTime) override;
        bool HandleClick(glm::vec2 mousePos) override;
        bool HandleMouseMove(glm::vec2 mousePos) override;
        
        
        void SetText(const std::string& newText) { text = newText; }
        void SetOnClick(std::function<void()> callback) { onClick = callback; }
        void SetNormalColor(const glm::vec4& color) { 
            normalColor = color; 
            SetColor(color);  
        }
        void SetHoverColor(const glm::vec4& color) { hoverColor = color; }
        void SetPressedColor(const glm::vec4& color) { pressedColor = color; }
        void SetTextColor(const glm::vec4& color) { textColor = color; }
        void SetFontSize(float size) { fontSize = size; }
        
        
        glm::vec4 GetColor() const {
            switch (state) {
                case ButtonState::HOVERED:
                    return hoverColor;
                case ButtonState::PRESSED:
                    return pressedColor;
                case ButtonState::NORMAL:
                default:
                    return normalColor;
            }
        }
        
        const std::string& GetText() const { return text; }
        ButtonState GetState() const { return state; }
        
    private:
        
        glm::vec2 CalculateTextSize() const;
        std::string RemoveEmojis(const std::string& str) const;
    };

} 
