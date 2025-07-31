#include "CollectibleItem.h"
#include <algorithm>
#include <cmath>

CollectibleItem::CollectibleItem(CollectibleType itemType, const glm::vec3& pos, 
                                const std::string& itemName, int itemValue)
    : type(itemType), position(pos), name(itemName), value(itemValue),
      collected(false), active(true), interactionRange(2.0f) {
    
    
    switch (type) {
        case CollectibleType::KEY:
            interactionRange = 1.5f;
            break;
        case CollectibleType::CHEST:
            interactionRange = 2.0f;
            break;
        case CollectibleType::SWITCH:
            interactionRange = 1.8f;
            break;
        case CollectibleType::DECORATION:
            interactionRange = 1.2f;
            break;
    }
    
    UpdateBoundingBox();
}

CollectibleItem::~CollectibleItem() {
}

void CollectibleItem::UpdateBoundingBox() {
    
    float halfSize = 0.5f;
    if (type == CollectibleType::CHEST) {
        halfSize = 0.8f;
    } else if (type == CollectibleType::SWITCH) {
        halfSize = 0.3f;
    }
    
    boundingBoxMin = position - glm::vec3(halfSize);
    boundingBoxMax = position + glm::vec3(halfSize);
}

bool CollectibleItem::IsPlayerInRange(const glm::vec3& playerPos) const {
    if (collected && type != CollectibleType::SWITCH) {
        return false; 
    }
    
    float distance = glm::length(playerPos - position);
    return distance <= interactionRange;
}

bool CollectibleItem::CheckAABBCollision(const glm::vec3& playerPos, float playerRadius) const {
    if (collected && type != CollectibleType::SWITCH) {
        return false;
    }
    
    
    glm::vec3 expandedMin = boundingBoxMin - glm::vec3(playerRadius);
    glm::vec3 expandedMax = boundingBoxMax + glm::vec3(playerRadius);
    
    return (playerPos.x >= expandedMin.x && playerPos.x <= expandedMax.x &&
            playerPos.y >= expandedMin.y && playerPos.y <= expandedMax.y &&
            playerPos.z >= expandedMin.z && playerPos.z <= expandedMax.z);
}

bool CollectibleItem::TryCollect(const glm::vec3& playerPos) {
    if (collected || !active) {
        return false;
    }
    
    if (IsPlayerInRange(playerPos)) {
        if (type == CollectibleType::SWITCH) {
            
            Activate();
            return true;
        } else {
            
            collected = true;
            return true;
        }
    }
    
    return false;
}

void CollectibleItem::Activate() {
    if (type == CollectibleType::SWITCH) {
        active = !active; 
    }
}

std::string CollectibleItem::GetInteractionHint() const {
    if (collected && type != CollectibleType::SWITCH) {
        return "";
    }
    
    switch (type) {
        case CollectibleType::KEY:
            return "Press E to collect key";
        case CollectibleType::CHEST:
            return "Press E to open chest";
        case CollectibleType::SWITCH:
            return active ? "Press E to activate switch" : "Press E to deactivate switch";
        case CollectibleType::DECORATION:
            return "Press E to collect item";
        default:
            return "Press E to interact";
    }
}

void CollectibleItem::SetPosition(const glm::vec3& pos) {
    position = pos;
    UpdateBoundingBox();
}
