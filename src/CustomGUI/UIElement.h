#pragma once

#include <glm/glm.hpp>
#include <functional>
#include <string>
#include <memory>

namespace CustomGUI {

    enum class UIElementType {
        PANEL,
        BUTTON,
        TEXT,
        PROGRESS_BAR,
        IMAGE
    };

    class UIElement {
    protected:
        glm::vec2 position;         
        glm::vec2 size;             
        glm::vec4 color;            
        bool visible;               
        bool interactive;           
        int zOrder;                 
        UIElementType type;         
        std::string id;             

    public:
        UIElement(UIElementType elementType, const std::string& elementId = "");
        virtual ~UIElement() = default;

        
        virtual void Render() = 0;
        virtual void Update(float deltaTime) {}
        virtual bool HandleClick(glm::vec2 mousePos) { return false; }
        virtual bool HandleMouseMove(glm::vec2 mousePos) { return false; }

        
        bool IsPointInside(glm::vec2 point) const;
        
        
        void SetPosition(const glm::vec2& pos) { position = pos; }
        void SetSize(const glm::vec2& sz) { size = sz; }
        void SetColor(const glm::vec4& col) { color = col; }
        void SetVisible(bool vis) { visible = vis; }
        void SetInteractive(bool inter) { interactive = inter; }
        void SetZOrder(int order) { zOrder = order; }

        glm::vec2 GetPosition() const { return position; }
        glm::vec2 GetSize() const { return size; }
        glm::vec4 GetColor() const { return color; }
        bool IsVisible() const { return visible; }
        bool IsInteractive() const { return interactive; }
        int GetZOrder() const { return zOrder; }
        UIElementType GetType() const { return type; }
        const std::string& GetId() const { return id; }

        
        glm::vec2 GetCenter() const { return position + size * 0.5f; }
        glm::vec4 GetBounds() const { return glm::vec4(position.x, position.y, position.x + size.x, position.y + size.y); }
    };

} 
