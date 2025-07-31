/**
 * @file CollectibleItem.h
 * @brief Collectible game objects with interaction mechanics
 * 
 * Defines interactive objects that players can collect or interact with
 * during gameplay. Supports various types of collectibles including keys,
 * chests, decorations, and switches with collision detection and state management.
 * 
 * Features:
 * - Multiple collectible types (keys, chests, decorations, switches)
 * - Bounding box collision detection
 * - Interaction range checking
 * - Collection state tracking
 * - Value and scoring system
 */

#pragma once

#include <glm/glm.hpp>
#include <string>
#include <memory>

/**
 * @brief Types of collectible objects in the game
 * 
 * Defines the different categories of interactive objects:
 * - KEY: Opens locked chests or doors
 * - CHEST: Contains valuable items, may require keys
 * - DECORATION: Visual elements with no gameplay function
 * - SWITCH: Activates mechanisms or triggers events
 */
enum class CollectibleType {
    KEY,            // Collectible key for unlocking
    CHEST,          // Treasure container
    DECORATION,     // Environmental object
    SWITCH          // Interactive mechanism
};

/**
 * @brief Interactive collectible object with collision detection
 * 
 * Represents any object in the game world that players can interact with.
 * Handles collision detection, collection state, and interaction mechanics.
 * Each collectible has a position, bounding box, and specific behavior.
 */
class CollectibleItem {
private:
    CollectibleType type;           // What kind of collectible this is
    glm::vec3 position;             // World position coordinates
    glm::vec3 boundingBoxMin;       // Minimum corner of collision box
    glm::vec3 boundingBoxMax;       // Maximum corner of collision box
    std::string name;               // Display name for the object
    bool collected;                 // Whether this item has been collected
    bool active;                    // Whether this item is currently active/visible
    float interactionRange;         // Maximum distance for player interaction
    int value;                      // Score value when collected
    
public:
    /**
     * @brief Constructor for collectible items
     * 
     * @param itemType Type of collectible object
     * @param pos World position for the item
     * @param itemName Display name for the item
     * @param itemValue Score value when collected (default: 10)
     */
    CollectibleItem(CollectibleType itemType, const glm::vec3& pos, 
                   const std::string& itemName, int itemValue = 10);
    ~CollectibleItem();
    
    /**
     * @brief Check if player is within interaction range
     * 
     * Determines if the player is close enough to interact with this
     * collectible based on the configured interaction range.
     * 
     * @param playerPos Current player position
     * @return True if player is within interaction range
     */
    bool IsPlayerInRange(const glm::vec3& playerPos) const;
    
    /**
     * @brief Check collision with player using bounding boxes
     * 
     * Performs AABB (Axis-Aligned Bounding Box) collision detection
     * between the player and this collectible item.
     * 
     * @param playerPos Player's current position
     * @param playerRadius Player's collision radius (default: 1.0)
     * @return True if player is colliding with this item
     */
    bool CheckAABBCollision(const glm::vec3& playerPos, float playerRadius = 1.0f) const;
    
    /**
     * @brief Attempt to collect this item
     * 
     * Tries to collect the item if the player is in range and the item
     * is available for collection. Updates the collected state.
     * 
     * @param playerPos Current player position
     * @return True if collection was successful
     */
    bool TryCollect(const glm::vec3& playerPos);
    
    /**
     * @brief Activate this collectible item
     * 
     * Makes the item active and available for interaction.
     * Used to enable items that may start disabled.
     */
    void Activate();
    
    /**
     * @brief Get interaction hint text for UI display
     * 
     * Returns appropriate hint text based on the collectible type
     * and current state. Used for player interaction prompts.
     * 
     * @return String containing interaction hint
     */
    std::string GetInteractionHint() const;
    
    // Getter methods for accessing collectible properties
    /**
     * @brief Get the type of this collectible
     * @return CollectibleType enum value
     */
    CollectibleType GetType() const { return type; }
    
    /**
     * @brief Get the world position of this collectible
     * @return 3D position vector
     */
    glm::vec3 GetPosition() const { return position; }
    
    /**
     * @brief Get the display name of this collectible
     * @return String name for UI display
     */
    std::string GetName() const { return name; }
    
    /**
     * @brief Check if this collectible has been collected
     * @return True if already collected
     */
    bool IsCollected() const { return collected; }
    
    /**
     * @brief Check if this collectible is currently active
     * @return True if active and available for interaction
     */
    bool IsActive() const { return active; }
    
    /**
     * @brief Get the score value of this collectible
     * @return Point value when collected
     */
    int GetValue() const { return value; }
    
    /**
     * @brief Get the interaction range for this collectible
     * @return Maximum distance for player interaction
     */
    float GetInteractionRange() const { return interactionRange; }
    
    // Setter methods for modifying collectible state
    /**
     * @brief Update the position of this collectible
     * @param pos New world position
     */
    void SetPosition(const glm::vec3& pos);
    
    /**
     * @brief Set the collected state of this item
     * @param isCollected New collected state
     */
    void SetCollected(bool isCollected) { collected = isCollected; }
    
    /**
     * @brief Set the active state of this item
     * @param isActive New active state
     */
    void SetActive(bool isActive) { active = isActive; }
    
private:
    /**
     * @brief Update the bounding box based on current position
     * 
     * Recalculates the collision bounding box when the position changes.
     * Called automatically when position is updated.
     */
    void UpdateBoundingBox();
};
