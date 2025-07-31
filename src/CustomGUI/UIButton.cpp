#include "UIButton.h"
#include "UIText.h"
#include "GUIManager.h"
#include <iostream>
#include <algorithm>

namespace CustomGUI {

    UIButton::UIButton(const std::string& buttonText, const std::string& elementId)
        : UIElement(UIElementType::BUTTON, elementId)
        , text(buttonText)
        , state(ButtonState::NORMAL)
        , normalColor(0.6f, 0.6f, 0.6f, 1.0f)    
        , hoverColor(0.8f, 0.8f, 0.8f, 1.0f)     
        , pressedColor(0.4f, 0.4f, 0.4f, 1.0f)   
        , textColor(1.0f, 1.0f, 1.0f, 1.0f)      
        , fontSize(16.0f)
    {
        size = glm::vec2(120.0f, 40.0f); 
    }

    void UIButton::Render() {
        if (!visible) return;

        
        glm::vec4 currentColor = normalColor;
        switch (state) {
            case ButtonState::HOVERED:
                currentColor = hoverColor;
                break;
            case ButtonState::PRESSED:
                currentColor = pressedColor;
                break;
            case ButtonState::NORMAL:
            default:
                currentColor = normalColor;
                break;
        }

        auto* guiManager = GUIManager::GetInstance();
        if (guiManager) {
            guiManager->RenderQuad(position, size, currentColor);
        }
        
        if (!text.empty() && UIText::IsFontSystemInitialized()) {
            glm::vec2 textSize = CalculateTextSize();
            glm::vec2 textPos;
            textPos.x = position.x + (size.x - textSize.x) / 2.0f;  
            textPos.y = position.y + (size.y - textSize.y) / 2.0f;  
            
            std::string cleanText = RemoveEmojis(text);

            std::cout << "[DEBUG_EXIT_TEXT] Button '" << cleanText << "' rendering text at (" << textPos.x << ", " << textPos.y << ")" << std::endl;
            std::cout << "[DEBUG_EXIT_TEXT] Text color: (" << textColor.r << ", " << textColor.g << ", " << textColor.b << ")" << std::endl;
            std::cout << "[DEBUG_EXIT_TEXT] Font size scale: " << (fontSize / 16.0f) << std::endl;
            
            UIText::RenderTextStatic(cleanText, textPos.x, textPos.y, fontSize / 16.0f, 
                                    glm::vec3(textColor.r, textColor.g, textColor.b));
        }
    }

    void UIButton::Update(float deltaTime) {
        
    }

    bool UIButton::HandleClick(glm::vec2 mousePos) {
        if (!visible || !interactive) {
            std::cout << " Button " << GetId() << " not clickable - visible: " << visible << ", interactive: " << interactive << std::endl;
            return false;
        }
        
        std::cout << " Button " << GetId() << " checking click at (" << mousePos.x << ", " << mousePos.y << ")" << std::endl;
        std::cout << " Button bounds: (" << position.x << ", " << position.y << ") to (" 
                  << (position.x + size.x) << ", " << (position.y + size.y) << ")" << std::endl;
        
        if (IsPointInside(mousePos)) {
            std::cout << "✓ Button " << GetId() << " clicked! Executing callback..." << std::endl;
            state = ButtonState::PRESSED;
            if (onClick) {
                onClick();
                std::cout << "✓ Callback executed for button " << GetId() << std::endl;
            } else {
                std::cout << "✗ No onClick callback set for button " << GetId() << std::endl;
            }
            return true;
        } else {
            std::cout << "✗ Click outside button " << GetId() << " bounds" << std::endl;
        }
        return false;
    }

    bool UIButton::HandleMouseMove(glm::vec2 mousePos) {
        if (!visible || !interactive) return false;
        
        if (IsPointInside(mousePos)) {
            if (state != ButtonState::PRESSED) {
                state = ButtonState::HOVERED;
            }
            return true;
        } else {
            if (state != ButtonState::PRESSED) {
                state = ButtonState::NORMAL;
            }
            return false;
        }
    }

    glm::vec2 UIButton::CalculateTextSize() const {
        if (text.empty()) return glm::vec2(0.0f);
        
        std::string cleanText = RemoveEmojis(text);
        float scale = fontSize / 16.0f;
        float width = cleanText.length() * 8.0f * scale; 
        float height = 8.0f * scale; 
        return glm::vec2(width, height);
    }

    std::string UIButton::RemoveEmojis(const std::string& str) const {
        std::string result;
        for (char c : str) {
            
            if (c >= 32 && c <= 126) {
                result += c;
            } else if (c == ' ') {
                result += c; 
            }
            
        }
        return result;
    }

} 
