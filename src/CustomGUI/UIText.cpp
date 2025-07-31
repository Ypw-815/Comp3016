#include "UIText.h"
#include <iostream>
#include <vector>

namespace CustomGUI {

    
    bool UIText::fontSystemInitialized = false;
    unsigned int UIText::fontShaderProgram = 0;
    unsigned int UIText::fontVAO = 0, UIText::fontVBO = 0;
    glm::mat4 UIText::fontProjection = glm::mat4(1.0f);
    Character UIText::characters[128] = {};

    
    static const unsigned char font8x8_basic[128][8] = {
        
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        
        { 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00},
        
        { 0x66, 0x66, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00},
        
        { 0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00},
        
        { 0x30, 0x7C, 0xC0, 0x78, 0x0C, 0xF8, 0x30, 0x00},
        
        { 0x00, 0xC6, 0xCC, 0x18, 0x30, 0x66, 0xC6, 0x00},
        
        { 0x38, 0x6C, 0x38, 0x76, 0xDC, 0xCC, 0x76, 0x00},
        
        { 0x60, 0x60, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00},
        
        { 0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00},
        
        { 0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00},
        
        { 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00},
        
        { 0x00, 0x30, 0x30, 0xFC, 0x30, 0x30, 0x00, 0x00},
        
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x60},
        
        { 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00},
        
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00},
        
        { 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00},
        
        { 0x7C, 0xC6, 0xCE, 0xDE, 0xF6, 0xE6, 0x7C, 0x00},
        
        { 0x30, 0x70, 0x30, 0x30, 0x30, 0x30, 0xFC, 0x00},
        
        { 0x78, 0xCC, 0x0C, 0x38, 0x60, 0xCC, 0xFC, 0x00},
        
        { 0x78, 0xCC, 0x0C, 0x38, 0x0C, 0xCC, 0x78, 0x00},
        
        { 0x1C, 0x3C, 0x6C, 0xCC, 0xFE, 0x0C, 0x1E, 0x00},
        
        { 0xFC, 0xC0, 0xF8, 0x0C, 0x0C, 0xCC, 0x78, 0x00},
        
        { 0x38, 0x60, 0xC0, 0xF8, 0xCC, 0xCC, 0x78, 0x00},
        
        { 0xFC, 0xCC, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00},
        
        { 0x78, 0xCC, 0xCC, 0x78, 0xCC, 0xCC, 0x78, 0x00},
        
        { 0x78, 0xCC, 0xCC, 0x7C, 0x0C, 0x18, 0x70, 0x00},
        
        { 0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x00},
        
        { 0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x60},
        
        { 0x18, 0x30, 0x60, 0xC0, 0x60, 0x30, 0x18, 0x00},
        
        { 0x00, 0x00, 0xFC, 0x00, 0x00, 0xFC, 0x00, 0x00},
        
        { 0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00},
        
        { 0x78, 0xCC, 0x0C, 0x18, 0x18, 0x00, 0x18, 0x00},
        
        { 0x7C, 0xC6, 0xDE, 0xDE, 0xDE, 0xC0, 0x78, 0x00},
        
        { 0x30, 0x78, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0x00},
        
        { 0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xFC, 0x00},
        
        { 0x3C, 0x66, 0xC0, 0xC0, 0xC0, 0x66, 0x3C, 0x00},
        
        { 0xF8, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0xF8, 0x00},
        
        { 0xFE, 0x62, 0x68, 0x78, 0x68, 0x62, 0xFE, 0x00},
        
        { 0xFE, 0x62, 0x68, 0x78, 0x68, 0x60, 0xF0, 0x00},
        
        { 0x3C, 0x66, 0xC0, 0xC0, 0xCE, 0x66, 0x3E, 0x00},
        
        { 0xCC, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0xCC, 0x00},
        
        { 0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00},
        
        { 0x1E, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 0x00},
        
        { 0xE6, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0xE6, 0x00},
        
        { 0xF0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xFE, 0x00},
        
        { 0xC6, 0xEE, 0xFE, 0xFE, 0xD6, 0xC6, 0xC6, 0x00},
        
        { 0xC6, 0xE6, 0xF6, 0xDE, 0xCE, 0xC6, 0xC6, 0x00},
        
        { 0x78, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x78, 0x00},
        
        { 0xFC, 0x66, 0x66, 0x7C, 0x60, 0x60, 0xF0, 0x00},
        
        { 0x78, 0xCC, 0xCC, 0xCC, 0xDC, 0x78, 0x1C, 0x00},
        
        { 0xFC, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0xE6, 0x00},
        
        { 0x78, 0xCC, 0xE0, 0x70, 0x1C, 0xCC, 0x78, 0x00},
        
        { 0xFC, 0xB4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00},
        
        { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xFC, 0x00},
        
        { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00},
        
        { 0xC6, 0xC6, 0xC6, 0xD6, 0xFE, 0xEE, 0xC6, 0x00},
        
        { 0xC6, 0xC6, 0x6C, 0x38, 0x38, 0x6C, 0xC6, 0x00},
        
        { 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x30, 0x78, 0x00},
        
        { 0xFE, 0xC6, 0x8C, 0x18, 0x32, 0x66, 0xFE, 0x00},
        
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  
    };

    UIText::UIText(const std::string& displayText, const std::string& elementId)
        : UIElement(UIElementType::TEXT, elementId)
        , text(displayText)
        , fontSize(16.0f)
        , textColor(1.0f, 1.0f, 1.0f)  
        , alignment(TextAlignment::LEFT)
        , multiline(false)
    {
        interactive = false; 
        AutoSizeToFitText();
    }

    void UIText::Render() {
        if (!visible) return;

        
        if (fontSystemInitialized) {
            float scale = fontSize / 16.0f; 
            
            
            float renderX = position.x;
            if (alignment == TextAlignment::CENTER) {
                
                float textWidth = 0.0f;
                for (char c : text) {
                    if (c >= 0 && c < 128) {
                        textWidth += characters[c].advance * scale;
                    }
                }
                
                renderX = position.x - textWidth / 2.0f;
            }
            
            RenderTextInternal(text, renderX, position.y, scale, textColor);
        } else {
            std::cout << "[UIText::Render] ERROR: Font system not initialized!" << std::endl;
        }
    }

    void UIText::Update(float deltaTime) {
        
    }

    glm::vec2 UIText::CalculateTextSize() const {
        float scale = fontSize / 16.0f;
        float width = text.length() * 8.0f * scale; 
        float height = 8.0f * scale; 
        return glm::vec2(width, height);
    }

    void UIText::AutoSizeToFitText() {
        glm::vec2 textSize = CalculateTextSize();
        size = textSize;
    }

    
    bool UIText::InitializeFontSystem() {
        if (fontSystemInitialized) return true;

        std::cout << "[UIText] Initializing font system..." << std::endl;

        
        if (!InitializeShaders()) {
            std::cout << "[UIText] ERROR: Failed to initialize font shaders" << std::endl;
            return false;
        }

        
        GenerateCharacterTextures();

        
        glGenVertexArrays(1, &fontVAO);
        glGenBuffers(1, &fontVBO);
        glBindVertexArray(fontVAO);
        glBindBuffer(GL_ARRAY_BUFFER, fontVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        fontSystemInitialized = true;
        std::cout << "[UIText] Font system initialized successfully!" << std::endl;
        return true;
    }

    void UIText::ShutdownFontSystem() {
        if (!fontSystemInitialized) return;

        
        if (fontVAO) {
            glDeleteVertexArrays(1, &fontVAO);
            fontVAO = 0;
        }
        if (fontVBO) {
            glDeleteBuffers(1, &fontVBO);
            fontVBO = 0;
        }
        if (fontShaderProgram) {
            glDeleteProgram(fontShaderProgram);
            fontShaderProgram = 0;
        }

        
        for (int i = 0; i < 128; i++) {
            if (characters[i].textureID) {
                glDeleteTextures(1, &characters[i].textureID);
                characters[i].textureID = 0;
            }
        }

        fontSystemInitialized = false;
    }

    void UIText::SetFontProjection(const glm::mat4& projection) {
        fontProjection = projection;
        if (fontSystemInitialized && fontShaderProgram) {
            glUseProgram(fontShaderProgram);
            int projLoc = glGetUniformLocation(fontShaderProgram, "projection");
            if (projLoc >= 0) {
                glUniformMatrix4fv(projLoc, 1, GL_FALSE, &fontProjection[0][0]);
            }
        }
    }

    bool UIText::InitializeShaders() {
        
        const char* vertexShaderSource = R"(
#version 410 core
layout (location = 0) in vec4 vertex; 
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

        
        const char* fragmentShaderSource = R"(
#version 410 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = vec4(textColor, 1.0) * sampled;
}
)";

        
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            return false;
        }

        
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            return false;
        }

        
        fontShaderProgram = glCreateProgram();
        glAttachShader(fontShaderProgram, vertexShader);
        glAttachShader(fontShaderProgram, fragmentShader);
        glLinkProgram(fontShaderProgram);

        
        glGetProgramiv(fontShaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(fontShaderProgram, 512, NULL, infoLog);
            return false;
        }

        
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return true;
    }

    void UIText::GenerateCharacterTextures() {
        
        for (int c = 0; c < 128; ++c) {
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            
            unsigned char bitmap[8 * 8 * 4];
            
            
            
            const unsigned char* fontData;
            if (c >= 32 && c <= 90) {
                
                fontData = font8x8_basic[c - 32]; 
            } else if (c >= 97 && c <= 122) {
                
                
                int upperCaseIndex = (c - 97) + (65 - 32); 
                fontData = font8x8_basic[upperCaseIndex];
            } else {
                
                fontData = font8x8_basic[0]; 
            }
            
            for (int y = 0; y < 8; ++y) {
                unsigned char row = fontData[y];
                for (int x = 0; x < 8; ++x) {
                    bool pixel = (row & (1 << (7-x))) != 0;
                    int idx = (y * 8 + x) * 4;
                    bitmap[idx] = pixel ? 255 : 0;     
                    bitmap[idx+1] = pixel ? 255 : 0;   
                    bitmap[idx+2] = pixel ? 255 : 0;   
                    bitmap[idx+3] = 255; 
                }
            }

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            Character character;
            character.textureID = texture;
            character.size = glm::ivec2(8, 8);
            character.bearing = glm::ivec2(0, 8);
            character.advance = 8;
            
            characters[c] = character;
        }
    }

    void UIText::RenderTextInternal(const std::string& text, float x, float y, float scale, const glm::vec3& color) {
        
        static bool debugShown = false;
        if (!debugShown) {
            std::cout << "[UIText::Debug] Rendering text: \"" << text << "\"" << std::endl;
            std::cout << "[UIText::Debug] Character codes: ";
            for (char c : text) {
                std::cout << (int)(unsigned char)c << " ";
            }
            std::cout << std::endl;
            debugShown = true;
        }

        
        glUseProgram(fontShaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(fontVAO);

        
        int colorLoc = glGetUniformLocation(fontShaderProgram, "textColor");
        if (colorLoc >= 0) {
            glUniform3f(colorLoc, color.x, color.y, color.z);
        }

        
        int projLoc = glGetUniformLocation(fontShaderProgram, "projection");
        if (projLoc >= 0) {
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, &fontProjection[0][0]);
        }

        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        
        float currentX = x;
        for (auto c = text.begin(); c != text.end(); c++) {
            unsigned char ch = *c;
            
            
            if (ch >= 128) {
                std::cout << "[UIText::Debug] Non-ASCII character detected: " << (int)ch << " replacing with '?'" << std::endl;
                ch = '?'; 
            }

            Character character = characters[ch];

            float xpos = currentX;
            float ypos = y - (character.size.y - character.bearing.y) * scale;

            float w = character.size.x * scale;
            float h = character.size.y * scale;

            
            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 1.0f },  
                { xpos,     ypos,       0.0f, 0.0f },  
                { xpos + w, ypos,       1.0f, 0.0f },  

                { xpos,     ypos + h,   0.0f, 1.0f },  
                { xpos + w, ypos,       1.0f, 0.0f },  
                { xpos + w, ypos + h,   1.0f, 1.0f }   
            };

            
            glBindTexture(GL_TEXTURE_2D, character.textureID);
            
            
            glBindBuffer(GL_ARRAY_BUFFER, fontVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            
            glDrawArrays(GL_TRIANGLES, 0, 6);

            
            currentX += character.advance * scale;
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_BLEND);
    }

    void UIText::SetText(const std::string& newText) {
        text = newText;
    }

    void UIText::SetFontSize(float size) {
        fontSize = size;
    }

    void UIText::SetTextColor(const glm::vec3& color) {
        textColor = color;
    }

    void UIText::SetAlignment(TextAlignment align) {
        alignment = align;
    }

    void UIText::SetMultiline(bool multi) {
        multiline = multi;
    }

    const std::string& UIText::GetText() const {
        return text;
    }

    float UIText::GetFontSize() const {
        return fontSize;
    }

    const glm::vec3& UIText::GetTextColor() const {
        return textColor;
    }

    TextAlignment UIText::GetAlignment() const {
        return alignment;
    }

    bool UIText::IsMultiline() const {
        return multiline;
    }

} 
