#include "UIElement.h"

namespace CustomGUI {

    UIElement::UIElement(UIElementType elementType, const std::string& elementId)
        : position(0.0f, 0.0f)
        , size(100.0f, 30.0f)
        , color(1.0f, 1.0f, 1.0f, 1.0f)
        , visible(true)
        , interactive(true)
        , zOrder(0)
        , type(elementType)
        , id(elementId)
    {
    }

    bool UIElement::IsPointInside(glm::vec2 point) const {
        return point.x >= position.x && 
               point.x <= position.x + size.x &&
               point.y >= position.y && 
               point.y <= position.y + size.y;
    }

} 
