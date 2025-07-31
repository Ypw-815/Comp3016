/**
 * @file GameInteraction.h
 * @brief Game interaction and mechanics management system
 * 
 * Manages core gameplay interactions including item collection,
 * raycasting for object selection, game state transitions, and
 * event-driven gameplay mechanics. Central hub for coordinating
 * player actions with game systems.
 * 
 * Features:
 * - Player-object interaction through raycasting
 * - Collectible item management and treasure hunting mechanics
 * - Game state management and progression
 * - Event system integration for responsive gameplay
 * - Audio feedback and visual effects coordination
 * - Terrain integration for environmental interactions
 */

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>
#include <string>

#include "GameState.h"
#include "EventSystem.h"
#include "CollectibleItem.h"
#include "TerrainGenerator.h"
#include "IrrklangAudioManager.h"

// Forward declarations
class Camera;

/**
 * @brief Ray structure for 3D raycasting operations
 * 
 * Represents a ray in 3D space with an origin point and direction vector.
 * Used for mouse picking, line-of-sight checks, and object interaction.
 */
struct Ray {
    glm::vec3 origin;     // Starting point of the ray
    glm::vec3 direction;  // Direction vector (should be normalized)
    
    /**
     * @brief Constructor for ray with origin and direction
     * @param orig Starting point of the ray
     * @param dir Direction vector of the ray
     */
    Ray(const glm::vec3& orig, const glm::vec3& dir) : origin(orig), direction(dir) {}
};

/**
 * @brief Ray intersection result structure
 * 
 * Contains information about ray-object intersections including
 * hit status, collision point, surface normal, and object identification.
 */
struct RayHit {
    bool hit;                // Whether the ray hit an object
    glm::vec3 point;         // World position of intersection
    glm::vec3 normal;        // Surface normal at intersection point
    float distance;          // Distance from ray origin to hit point
    std::string objectName;  // Identifier of the hit object
    
    /**
     * @brief Default constructor - initializes to no hit
     */
    RayHit() : hit(false), distance(0.0f) {}
};

/**
 * @brief Central game interaction management system
 * 
 * Coordinates all gameplay interactions between the player and the game world.
 * Manages treasure hunting mechanics, item collection, player feedback,
 * and game state progression. Integrates multiple game systems to provide
 * cohesive interactive experiences.
 * 
 * Key responsibilities:
 * - Processing player input and converting to game actions
 * - Managing collectible items and treasure hunting mechanics
 * - Coordinating raycasting for object selection and interaction
 * - Handling game state transitions and progression
 * - Providing audio and visual feedback for player actions
 * - Integrating with terrain system for environmental interactions
 */
class GameInteraction {
private:
    // Core game state management
    GameState currentState;                    // Current game state (menu, playing, etc.)
    AncientTreasureGameData treasureGameData;  // Treasure hunting game specific data
    
    // Event system for responsive gameplay
    std::unique_ptr<EventSystem> eventSystem;  // Event-driven game mechanics
    
    // Item management system
    std::vector<std::unique_ptr<CollectibleItem>> collectibleItems;  // All collectible objects
    
    // World integration
    TerrainGenerator* terrainGenerator;  // Reference to terrain system
    
    // Interaction parameters
    float raycastDistance;      // Maximum distance for object interaction
    bool debugMode;            // Whether to display debug information
    
    // Player interaction state
    CollectibleItem* currentNearbyItem;  // Item currently within interaction range

public:
    GameInteraction(TerrainGenerator* terrain);
    ~GameInteraction();
    
    
    void Initialize();
    
    
    void Update(float deltaTime, Camera* camera);
    
    
    void HandleInput(int key, int action, const glm::vec3& playerPos, const glm::vec3& viewDir);
    
    
    RayHit PerformRaycast(const Ray& ray);
    
    
    void CheckItemCollection(const glm::vec3& playerPos);
    void AddCollectibleItem(CollectibleType type, const glm::vec3& position, 
                           const std::string& name, int value = 10);
    
    
    void DestroyBlock(const glm::vec3& position);
    void PlaceBlock(const glm::vec3& position);
    
    
    void SetGameState(GameState newState);
    GameState GetGameState() const { return currentState; }
    const AncientTreasureGameData& GetTreasureGameData() const { return treasureGameData; }
    
    
    void UpdateCurrentTask();
    std::string GetCurrentTask() const { return treasureGameData.currentTask; }
    
    
    std::string GetInteractionHint() const;
    bool HasNearbyInteractable() const { return currentNearbyItem != nullptr; }
    
    
    EventSystem* GetEventSystem() { return eventSystem.get(); }
    
    
    void CheckWinCondition();
    
    
    void SetDebugMode(bool enabled) { debugMode = enabled; }
    bool IsDebugMode() const { return debugMode; }
    
    
    int GetCollectedItemsCount() const;
    int GetTotalItemsCount() const { return collectibleItems.size(); }
    
private:
    
    void SetupEventListeners();
    
    
    bool RayTerrainIntersection(const Ray& ray, glm::vec3& hitPoint, float& distance);
    
    
    bool RayItemIntersection(const Ray& ray, CollectibleItem* item, float& distance);
    
    
    void UpdateGameData(float deltaTime, const glm::vec3& playerPos);
    
    
    void OnItemCollected(const EventData& eventData);
    void OnKeyCollected(const EventData& eventData);
    void OnChestOpened(const EventData& eventData);
};
