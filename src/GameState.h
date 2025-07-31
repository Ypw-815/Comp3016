/**
 * @file GameState.h
 * @brief Game state management and treasure system
 * 
 * Defines the core game states, treasure mechanics, and progression
 * tracking for the ancient treasure hunting game. Provides comprehensive
 * data structures for managing gameplay state and treasure collection.
 * 
 * Features:
 * - Game state transitions (menu, playing, paused, victory)
 * - Treasure type classification (keys, chests, decorations)
 * - Collection status tracking
 * - Key-chest unlock relationships
 * - Progress statistics and scoring
 */

#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

/**
 * @brief Primary game states for application flow
 * 
 * Defines the main states the game can be in, controlling
 * which systems are active and how user input is handled.
 */
enum class GameState {
    MAIN_MENU,      // Initial menu screen
    IN_GAME,        // Active gameplay state
    SETTINGS,       // Configuration menu
    PAUSED,         // Game temporarily suspended
    VICTORY         // Win condition achieved
};

/**
 * @brief Categories of treasures found in the game world
 * 
 * Different treasure types have different gameplay mechanics:
 * - ANCIENT_KEY: Required to unlock treasure chests
 * - TREASURE_CHEST: Contains valuable items, requires keys
 * - DECORATION: Environmental objects with no gameplay function
 */
enum class TreasureType {
    ANCIENT_KEY,    // Collectible key for unlocking chests
    TREASURE_CHEST, // Locked containers with rewards
    DECORATION      // Visual elements only
};

/**
 * @brief Current status of individual treasure items
 * 
 * Tracks the player's interaction with each treasure:
 * - UNCOLLECTED: Available for collection
 * - COLLECTED: Picked up by player (keys only)
 * - UNLOCKED: Opened successfully (chests only)
 */
enum class TreasureStatus {
    UNCOLLECTED,    // Not yet interacted with
    COLLECTED,      // Successfully obtained by player
    UNLOCKED        // Chest has been opened
};

/**
 * @brief Complete data for a single treasure object
 * 
 * Contains all information needed to manage a treasure item
 * including its type, location, status, and unlock requirements.
 */
struct TreasureData {
    int id;                     // Unique identifier for this treasure
    TreasureType type;          // What kind of treasure this is
    TreasureStatus status;      // Current collection/unlock state
    glm::vec3 position;         // World coordinates of treasure
    int requiredKeyId;          // ID of key needed to unlock (chests only)
    std::string description;    // Human-readable treasure description
    
    /**
     * @brief Default constructor initializes safe default values
     */
    TreasureData() : id(-1), type(TreasureType::DECORATION), 
                     status(TreasureStatus::UNCOLLECTED), 
                     position(0.0f), requiredKeyId(-1) {}
};

/**
 * @brief Complete game progress and treasure tracking data
 * 
 * Maintains all information about the current game session including
 * timing, collection progress, and individual treasure states.
 */
struct AncientTreasureGameData {
    float gameTime;                         // Elapsed time in seconds
    int keysCollected;                      // Number of keys found
    int totalKeys;                          // Total keys in the level
    int chestsUnlocked;                     // Number of chests opened
    int totalChests;                        // Total chests in the level
    std::vector<TreasureData> treasures;    // All treasure objects    
    std::string currentTask;                // Current objective description
    glm::vec3 playerPosition;               // Player's world coordinates
    bool nearInteractable;                  // True if player can interact with nearby object
    std::string interactionHint;            // Text hint for current interaction
    int nearestTreasureId;                  // ID of closest treasure to player
    float distanceToNearestTreasure;        // Distance to nearest uncollected treasure
    bool allTreasuresFound;                 // True when all treasures collected
    bool gameWon;                          // True when victory conditions met
    float explorationDistance;             // Total distance player has traveled
    
    /**
     * @brief Default constructor initializes gameplay state
     * 
     * Sets up initial values for a new game session with default
     * task description and safe initial values for all counters.
     */
    AncientTreasureGameData() : 
        gameTime(0.0f), keysCollected(0), totalKeys(0), 
        chestsUnlocked(0), totalChests(0),
        currentTask("Explore the ancient ruins and find treasures"),
        playerPosition(0.0f), nearInteractable(false), 
        nearestTreasureId(-1), distanceToNearestTreasure(999.0f),
        allTreasuresFound(false), gameWon(false), explorationDistance(0.0f) {}
    
    /**
     * @brief Add a new treasure to the game world
     * 
     * Creates and registers a new treasure object with the specified
     * properties. Automatically updates total counts for keys/chests.
     * 
     * @param id Unique identifier for the treasure
     * @param type Type of treasure (key, chest, decoration)
     * @param pos World position for the treasure
     * @param keyId Required key ID for chests (default: -1 for no requirement)
     */
    void AddTreasure(int id, TreasureType type, glm::vec3 pos, int keyId = -1) {
        TreasureData treasure;
        treasure.id = id;
        treasure.type = type;
        treasure.position = pos;
        treasure.requiredKeyId = keyId;
        treasure.status = TreasureStatus::UNCOLLECTED;
        
        switch(type) {
            case TreasureType::ANCIENT_KEY:
                treasure.description = "Ancient Key #" + std::to_string(id);
                totalKeys++;
                break;
            case TreasureType::TREASURE_CHEST:
                treasure.description = "Treasure Chest #" + std::to_string(id);
                totalChests++;
                break;
            case TreasureType::DECORATION:
                treasure.description = "Ancient Stone Pillar";
                break;
        }
        
        treasures.push_back(treasure);
    }
    
    /**
     * @brief Attempt to collect a treasure by ID
     * 
     * Processes treasure collection based on type and requirements.
     * Keys are collected directly, chests require the correct key.
     * 
     * @param treasureId ID of treasure to collect
     * @return True if collection was successful, false otherwise
     */
    bool CollectTreasure(int treasureId) {
        for (auto& treasure : treasures) {
            if (treasure.id == treasureId && treasure.status == TreasureStatus::UNCOLLECTED) {
                if (treasure.type == TreasureType::ANCIENT_KEY) {
                    treasure.status = TreasureStatus::COLLECTED;
                    keysCollected++;
                    return true;
                } else if (treasure.type == TreasureType::TREASURE_CHEST) {
                    // Check if player has the required key
                    if (HasKey(treasure.requiredKeyId)) {
                        treasure.status = TreasureStatus::UNLOCKED;
                        chestsUnlocked++;
                        return true;
                    }
                }
            }
        }
        return false;
    }
    
    /**
     * @brief Check if player has collected a specific key
     * 
     * Searches the player's collected treasures for a specific key ID.
     * Used to determine if chests can be unlocked.
     * 
     * @param keyId ID of key to check for (-1 means no key required)
     * @return True if player has the key or no key is required
     */
    bool HasKey(int keyId) {
        if (keyId == -1) return true; // No key required
        for (const auto& treasure : treasures) {
            if (treasure.id == keyId && 
                treasure.type == TreasureType::ANCIENT_KEY && 
                treasure.status == TreasureStatus::COLLECTED) {
                return true;
            }
        }
        return false;
    }
    
    /**
     * @brief Check if victory conditions have been met
     * 
     * Victory requires collecting all keys and unlocking all chests
     * in the current level.
     * 
     * @return True if all treasures have been found
     */
    bool CheckVictory() {
        return (keysCollected == totalKeys && chestsUnlocked == totalChests);
    }
    
    /**
     * @brief Update game state and current task description
     * 
     * Recalculates victory status and updates the current task
     * description based on progress. Called each frame to keep
     * UI and game state synchronized.
     */
    void UpdateGameState() {
        allTreasuresFound = (keysCollected == totalKeys && chestsUnlocked == totalChests);
        gameWon = allTreasuresFound;
        
        if (keysCollected < totalKeys) {
            currentTask = "Find ancient keys: " + std::to_string(keysCollected) + "/" + std::to_string(totalKeys);
        } else if (chestsUnlocked < totalChests) {
            currentTask = "Unlock treasure chests: " + std::to_string(chestsUnlocked) + "/" + std::to_string(totalChests);
        } else {
            currentTask = "All treasures found! You are a master treasure hunter!";
        }
    }
};
