/**
 * @file EventSystem.h
 * @brief Event-driven game mechanics system
 * 
 * Provides a flexible event system for decoupled game mechanics.
 * Allows different game systems to communicate through events
 * without tight coupling, enabling responsive and modular gameplay.
 * 
 * Features:
 * - Type-safe event dispatching
 * - Multiple listeners per event type
 * - Flexible event data structure
 * - Easy registration and unregistration of handlers
 * - Support for various gameplay events
 */

#pragma once

#include <functional>
#include <string>
#include <map>
#include <vector>
#include <glm/glm.hpp>

/**
 * @brief Types of events that can occur in the game
 * 
 * Defines all possible event types that can be triggered
 * during gameplay. Each event type can carry associated
 * data relevant to that specific event.
 */
enum class EventType {
    BLOCK_DESTROYED,     // When a destructible object is broken
    ITEM_COLLECTED,      // When player picks up an item
    SWITCH_ACTIVATED,    // When a switch or button is pressed
    KEY_COLLECTED,       // When player finds a key
    CHEST_OPENED,        // When a treasure chest is opened
    GAME_WON,           // When victory conditions are met
    PLAYER_MOVE         // When player changes position
};

/**
 * @brief Event data structure containing event information
 * 
 * Carries all relevant information about an event including
 * type, world position, object identification, and numeric values.
 * Flexible structure that can accommodate various event types.
 */
struct EventData {
    EventType type;          // Type of event that occurred
    glm::vec3 position;      // World position where event happened
    std::string objectName;  // Name/ID of object involved in event
    int value;              // Numeric value (score, quantity, etc.)
    
    /**
     * @brief Constructor for event data
     * 
     * @param t Event type
     * @param pos World position (default: origin)
     * @param name Object name (default: empty string)
     * @param val Numeric value (default: 0)
     */
    EventData(EventType t, glm::vec3 pos = glm::vec3(0.0f), 
              const std::string& name = "", int val = 0)
        : type(t), position(pos), objectName(name), value(val) {}
};

/**
 * @brief Function type for event handlers
 * 
 * Event handlers are functions that respond to specific events.
 * They receive event data and can perform any necessary actions.
 */
using EventHandler = std::function<void(const EventData&)>;

/**
 * @brief Central event management system
 * 
 * Manages event registration, dispatching, and handler lifecycle.
 * Enables loose coupling between game systems by allowing them
 * to communicate through events rather than direct references.
 * 
 * Key features:
 * - Multiple handlers can listen to the same event type
 * - Type-safe event dispatching
 * - Easy handler registration and removal
 * - Immediate event processing
 * - Flexible event data system
 */
class EventSystem {
private:
    // Event listener storage: maps event types to handler lists
    std::map<EventType, std::vector<EventHandler>> listeners;
    
public:
    /**
     * @brief Add an event listener for a specific event type
     * 
     * Registers a handler function to be called when the specified
     * event type is triggered. Multiple handlers can listen to the
     * same event type.
     * 
     * @param type Event type to listen for
     * @param handler Function to call when event occurs
     */
    void AddListener(EventType type, EventHandler handler);
    
    /**
     * @brief Remove all listeners for a specific event type
     * 
     * Removes all registered handlers for the given event type.
     * Useful for cleanup when systems are destroyed or reset.
     * 
     * @param type Event type to clear listeners for
     */
    void RemoveListeners(EventType type);
    
    /**
     * @brief Trigger an event and notify all listeners
     * 
     * Dispatches an event to all registered handlers for that event type.
     * Handlers are called immediately in the order they were registered.
     * 
     * @param eventData Event information to pass to handlers
     */
    void TriggerEvent(const EventData& eventData);
    
    /**
     * @brief Clear all event listeners
     * 
     * Removes all registered handlers for all event types.
     * Used for complete cleanup of the event system.
     */
    void Clear();
};
