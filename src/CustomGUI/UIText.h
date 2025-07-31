#pragma once

#include "UIElement.h"
#include <memory>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace CustomGUI {

    enum class TextAlignment {
        LEFT,
        CENTER,
        RIGHT
    };

    
    struct Character {
        unsigned int textureID;  
        glm::ivec2   size;       
        glm::ivec2   bearing;    
        unsigned int advance;    
    };

    class UIText : public UIElement {
    private:
        std::string text;
        float fontSize;
        glm::vec3 textColor;
        TextAlignment alignment;
        bool multiline;
        
        
        static bool fontSystemInitialized;
        static unsigned int fontShaderProgram;
        static unsigned int fontVAO, fontVBO;
        static glm::mat4 fontProjection;
        static Character characters[128]; 
        
        
        static bool InitializeShaders();
        static void GenerateCharacterTextures();
        static void RenderTextInternal(const std::string& text, float x, float y, float scale, const glm::vec3& color);

    public:
        UIText(const std::string& displayText = "Text", const std::string& elementId = "");
        
        void Render() override;
        void Update(float deltaTime) override;
        
        
        void SetText(const std::string& newText);
        void SetFontSize(float size);
        void SetTextColor(const glm::vec3& color);
        void SetAlignment(TextAlignment align);
        void SetMultiline(bool multi);
        
        const std::string& GetText() const;
        float GetFontSize() const;
        const glm::vec3& GetTextColor() const;
        TextAlignment GetAlignment() const;
        bool IsMultiline() const;
        
        
        glm::vec4 GetColor() const { return glm::vec4(textColor, 1.0f); }
        
        
        static bool InitializeFontSystem();
        static void ShutdownFontSystem();
        static void SetFontProjection(const glm::mat4& projection);
        static bool IsFontSystemInitialized() { return fontSystemInitialized; }
        static void RenderTextStatic(const std::string& text, float x, float y, float scale, const glm::vec3& color) {
            RenderTextInternal(text, x, y, scale, color);
        }
        
        
        glm::vec2 CalculateTextSize() const;
        void AutoSizeToFitText();
    };

} 
