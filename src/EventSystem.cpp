/**
 * @file EventSystem.cpp
 * @brief Implementation of the event system
 * 
 * Provides the concrete implementation of event registration,
 * dispatching, and management functionality. Handles the storage
 * and execution of event handlers in a thread-safe manner.
 */

#include "EventSystem.h"
#include <iostream>

/**
 * @brief Add an event listener for a specific event type
 * 
 * Registers a handler function to be called when the specified
 * event type is triggered. Handlers are stored in the order they
 * are registered and called in that same order.
 */
void EventSystem::AddListener(EventType type, EventHandler handler) {
    listeners[type].push_back(handler);
}

/**
 * @brief Remove all listeners for a specific event type
 * 
 * Clears all registered handlers for the given event type.
 * This is useful when a system is being destroyed or reset.
 */
void EventSystem::RemoveListeners(EventType type) {
    listeners[type].clear();
}

/**
 * @brief Trigger an event and notify all registered listeners
 * 
 * Dispatches an event to all handlers registered for that event type.
 * Handlers are called immediately in registration order. If no handlers
 * are registered for the event type, the event is silently ignored.
 */
void EventSystem::TriggerEvent(const EventData& eventData) {
    auto it = listeners.find(eventData.type);
    if (it != listeners.end()) {
        for (const auto& handler : it->second) {
            handler(eventData);
        }
    }
    
    // Debug output (commented out for performance)
    /*
    std::cout << "Event triggered: ";
    switch (eventData.type) {
        case EventType::BLOCK_DESTROYED:
            std::cout << "BLOCK_DESTROYED at (" << eventData.position.x << ", " 
                     << eventData.position.y << ", " << eventData.position.z << ")";
            break;
        case EventType::ITEM_COLLECTED:
            std::cout << "ITEM_COLLECTED: " << eventData.objectName;
            break;
        case EventType::KEY_COLLECTED:
            std::cout << "KEY_COLLECTED: " << eventData.objectName;
            break;
        case EventType::CHEST_OPENED:
            std::cout << "CHEST_OPENED: " << eventData.objectName;
            break;
        case EventType::GAME_WON:
            std::cout << "GAME_WON! Score: " << eventData.value;
            break;
        case EventType::SWITCH_ACTIVATED:
            std::cout << "SWITCH_ACTIVATED: " << eventData.objectName;
            break;
        case EventType::PLAYER_MOVE:
            // Player movement events are frequent, skip logging
            return;
        default:
            std::cout << "Unknown event type";
            break;
    }
    std::cout << std::endl;
    */
}

/**
 * @brief Clear all event listeners
 * 
 * Removes all registered handlers for all event types.
 * This provides a clean slate for the event system,
 * typically used during application shutdown or reset.
 */
void EventSystem::Clear() {
    listeners.clear();
}
