#include "UIPanel.h"
#include <algorithm>

namespace CustomGUI {

    UIPanel::UIPanel(const std::string& elementId)
        : UIElement(UIElementType::PANEL, elementId)
        , backgroundColor(0.2f, 0.2f, 0.2f, 0.8f)  
        , borderColor(0.5f, 0.5f, 0.5f, 1.0f)      
        , borderWidth(2.0f)
        , hasBorder(true)
    {
        size = glm::vec2(300.0f, 200.0f); 
    }

    void UIPanel::Render() {
        if (!visible) return;

        
        

        
        for (auto& child : children) {
            if (child && child->IsVisible()) {
                child->Render();
            }
        }
    }

    void UIPanel::Update(float deltaTime) {
        
        for (auto& child : children) {
            if (child) {
                child->Update(deltaTime);
            }
        }
    }

    bool UIPanel::HandleClick(glm::vec2 mousePos) {
        if (!visible || !interactive) return false;

        
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            auto& child = *it;
            if (child && child->IsInteractive()) {
                if (child->HandleClick(mousePos)) {
                    return true; 
                }
            }
        }

        
        return IsPointInside(mousePos);
    }

    bool UIPanel::HandleMouseMove(glm::vec2 mousePos) {
        if (!visible || !interactive) return false;

        bool handled = false;
        
        
        for (auto& child : children) {
            if (child && child->IsInteractive()) {
                if (child->HandleMouseMove(mousePos)) {
                    handled = true;
                }
            }
        }

        return handled || IsPointInside(mousePos);
    }

    void UIPanel::AddChild(std::shared_ptr<UIElement> child) {
        if (child) {
            children.push_back(child);
            
            
            std::sort(children.begin(), children.end(), 
                [](const std::shared_ptr<UIElement>& a, const std::shared_ptr<UIElement>& b) {
                    return a->GetZOrder() < b->GetZOrder();
                });
        }
    }

    void UIPanel::RemoveChild(std::shared_ptr<UIElement> child) {
        children.erase(
            std::remove(children.begin(), children.end(), child),
            children.end()
        );
    }

    void UIPanel::RemoveChild(const std::string& childId) {
        children.erase(
            std::remove_if(children.begin(), children.end(),
                [&childId](const std::shared_ptr<UIElement>& child) {
                    return child && child->GetId() == childId;
                }),
            children.end()
        );
    }

    void UIPanel::ClearChildren() {
        children.clear();
    }

} 
