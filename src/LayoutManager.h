/**
 * @file LayoutManager.h
 * @brief Intelligent object placement and layout management system
 * 
 * Provides advanced algorithms for procedural placement of game objects
 * across procedurally generated terrain. Creates themed zones and ensures
 * logical, balanced distribution of interactive elements.
 * 
 * Features:
 * - Zone-based layout organization with themes
 * - Terrain-aware object placement
 * - Distance-based placement validation
 * - Theme-specific positioning logic
 * - Collision avoidance for object placement
 * - Integration with terrain generation system
 */

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "TerrainGenerator.h"

/**
 * @brief Data structure for placed objects in the game world
 * 
 * Contains complete transformation and identification information
 * for objects placed by the layout manager.
 */
struct LayoutObject {
    glm::vec3 position;     // World position coordinates
    glm::vec3 rotation;     // Rotation angles (pitch, yaw, roll)
    glm::vec3 scale;        // Scale factors (x, y, z)
    std::string type;       // Object type identifier (key, chest, decoration)
    std::string theme;      // Theme classification (forest, mountain, desert, etc.)
};

/**
 * @brief Intelligent layout management system for procedural object placement
 * 
 * Manages the strategic placement of game objects across procedurally
 * generated terrain using zone-based algorithms and theme-aware logic.
 * 
 * Key capabilities:
 * - Creates themed zones with appropriate object distributions
 * - Ensures minimum distances between objects to prevent clustering
 * - Validates placement positions based on terrain suitability
 * - Integrates with terrain biome system for cohesive world design
 * - Provides balanced gameplay progression through strategic placement
 */
class LayoutManager {
public:
    /**
     * @brief Themed zone structure for organized object placement
     * 
     * Defines areas of the game world with specific themes and
     * placement rules for creating coherent, engaging environments.
     */
    struct LayoutZone {
        glm::vec3 center;                       // Zone center coordinates
        float radius;                           // Zone influence radius
        std::string theme;                      // Theme identifier (biome-based)
        std::vector<glm::vec3> objectPositions; // Valid object placement positions
        int maxObjects;                         // Maximum objects allowed in this zone
        float minDistance;                      // Minimum distance between objects
    };
    
private:
    std::vector<LayoutZone> zones;              // All layout zones in the world
    TerrainGenerator* terrainGenerator;         // Reference to terrain system
    
public:
    /**
     * @brief Constructor with terrain generator integration
     * 
     * @param terrain Pointer to terrain generator for position validation
     */
    LayoutManager(TerrainGenerator* terrain);
    
    /**
     * @brief Destructor - cleanup layout resources
     */
    ~LayoutManager();
    
    /**
     * @brief Create a zoned layout across the game world
     * 
     * Analyzes the terrain and creates themed zones with appropriate
     * object placement rules based on biome characteristics.
     * 
     * @return Vector of created layout zones
     */
    std::vector<LayoutZone> CreateZonedLayout();
    
    /**
     * @brief Place objects within a specific zone
     * 
     * Populates a zone with appropriate objects while respecting
     * distance constraints and terrain suitability.
     * 
     * @param zone Zone to populate with objects
     * @param objects Output vector to store placed objects
     */
    void PlaceObjectsInZone(const LayoutZone& zone, std::vector<LayoutObject>& objects);
    
    /**
     * @brief Calculate theme-appropriate position adjustments
     * 
     * Modifies placement positions based on theme-specific rules
     * (e.g., elevating objects in mountain themes, hiding in forests).
     * 
     * @param theme Theme identifier for positioning rules
     * @param basePos Base position to adjust
     * @return Adjusted position coordinates
     */
    glm::vec3 GetThemeBasedPosition(const std::string& theme, glm::vec3 basePos);
    
    /**
     * @brief Validate if a position is suitable for object placement
     * 
     * Checks terrain height, slope, biome compatibility, and collision
     * to determine if an object can be safely placed at the position.
     * 
     * @param position World coordinates to validate
     * @param objectType Type of object to place
     * @return True if position is suitable for placement
     */
    bool IsPositionSuitable(glm::vec3 position, const std::string& objectType);
    
    /**
     * @brief Get all layout zones
     * @return Const reference to the zones vector
     */
    const std::vector<LayoutZone>& GetZones() const { return zones; }
    
    
    void InitializeDefaultLayout();

    
    glm::vec3 GetRandomPositionInZone(const LayoutZone& zone) const;
    
private:
    
    bool CheckMinimumDistance(glm::vec3 newPos, const std::vector<glm::vec3>& existingPositions, float minDist);
    
    
    glm::vec3 GenerateRandomPositionInCircle(glm::vec3 center, float radius);
    
    
    bool IsWithinTerrainBounds(glm::vec3 position);
};
