#include "GameInteraction.h"
#include "Camera.h"
#include <iostream>
#include <algorithm>
#include <GLFW/glfw3.h>

GameInteraction::GameInteraction(TerrainGenerator* terrain)
    : currentState(GameState::IN_GAME), terrainGenerator(terrain),
      raycastDistance(10.0f), debugMode(false), currentNearbyItem(nullptr) {
    
    eventSystem = std::make_unique<EventSystem>();
}

GameInteraction::~GameInteraction() {
}

void GameInteraction::Initialize() {
    std::cout << "\n=== Initializing Game Interaction System ===" << std::endl;
    
    
    treasureGameData = AncientTreasureGameData();
    treasureGameData.currentTask = "Find the ancient keys scattered across the ruins";
    treasureGameData.totalKeys = 0; 
    
    
    SetupEventListeners();
    
    
    
    // GameInteraction collectible items disabled - using TreasureGame system instead
    // AddCollectibleItem(CollectibleType::KEY, glm::vec3(-6.0f, 1.0f, -6.0f), "key_1", 20);
    // AddCollectibleItem(CollectibleType::CHEST, glm::vec3(-8.0f, 1.0f, 0.0f), "treasure_chest_1", 50);
    
    std::cout << "Ancient Treasure Hunter System initialized!" << std::endl;
    std::cout << "Features: Treasure collection, Ray casting, Event system" << std::endl;
    std::cout << "Controls: E key to interact, Mouse to aim" << std::endl;
    std::cout << "Total collectible items: " << collectibleItems.size() << std::endl;
}

void GameInteraction::Update(float deltaTime, Camera* camera) {
    if (currentState != GameState::IN_GAME) {
        return;
    }
    
    
    UpdateGameData(deltaTime, camera->Position);
    
    
    CheckItemCollection(camera->Position);
    
    
    UpdateCurrentTask();
    
    
    CheckWinCondition();
}

void GameInteraction::HandleInput(int key, int action, const glm::vec3& playerPos, const glm::vec3& viewDir) {
    if (currentState != GameState::IN_GAME) {
        return;
    }
    
    
    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        if (currentNearbyItem) {
            if (currentNearbyItem->TryCollect(playerPos)) {
                
                AudioEventHandler::PlayEventSound(AudioEvent::ITEM_COLLECTED, currentNearbyItem->GetPosition());
                
                
                EventData eventData(EventType::ITEM_COLLECTED, 
                                  currentNearbyItem->GetPosition(),
                                  currentNearbyItem->GetName(),
                                  currentNearbyItem->GetValue());
                
                if (currentNearbyItem->GetType() == CollectibleType::KEY) {
                    eventData.type = EventType::KEY_COLLECTED;
                } else if (currentNearbyItem->GetType() == CollectibleType::CHEST) {
                    eventData.type = EventType::CHEST_OPENED;
                }
                
                eventSystem->TriggerEvent(eventData);
            } else {
                
                AudioEventHandler::PlayEventSound(AudioEvent::ERROR_SOUND);
            }
        } else {
            
            AudioEventHandler::PlayEventSound(AudioEvent::ERROR_SOUND);
        }
    }
    
    
    
    
    
    
    
    
    if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
        debugMode = !debugMode;
        std::cout << "Debug mode: " << (debugMode ? "ON" : "OFF") << std::endl;
    }
}

RayHit GameInteraction::PerformRaycast(const Ray& ray) {
    RayHit result;
    float closestDistance = raycastDistance;
    
    
    glm::vec3 terrainHitPoint;
    float terrainDistance;
    if (RayTerrainIntersection(ray, terrainHitPoint, terrainDistance)) {
        if (terrainDistance < closestDistance) {
            result.hit = true;
            result.point = terrainHitPoint;
            result.distance = terrainDistance;
            result.objectName = "terrain";
            result.normal = glm::vec3(0.0f, 1.0f, 0.0f); 
            closestDistance = terrainDistance;
        }
    }
    
    
    for (auto& item : collectibleItems) {
        if (item->IsCollected() && item->GetType() != CollectibleType::SWITCH) {
            continue; 
        }
        
        float itemDistance;
        if (RayItemIntersection(ray, item.get(), itemDistance)) {
            if (itemDistance < closestDistance) {
                result.hit = true;
                result.point = ray.origin + ray.direction * itemDistance;
                result.distance = itemDistance;
                result.objectName = item->GetName();
                result.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                closestDistance = itemDistance;
            }
        }
    }
    
    return result;
}

void GameInteraction::CheckItemCollection(const glm::vec3& playerPos) {
    currentNearbyItem = nullptr;
    
    for (auto& item : collectibleItems) {
        if (item->IsPlayerInRange(playerPos)) {
            currentNearbyItem = item.get();
            break; 
        }
    }
    
    
    treasureGameData.nearInteractable = (currentNearbyItem != nullptr);
    if (currentNearbyItem) {
        treasureGameData.interactionHint = currentNearbyItem->GetInteractionHint();
    } else {
        treasureGameData.interactionHint = "";
    }
}

void GameInteraction::AddCollectibleItem(CollectibleType type, const glm::vec3& position, 
                                        const std::string& name, int value) {
    auto item = std::make_unique<CollectibleItem>(type, position, name, value);
    collectibleItems.push_back(std::move(item));
    treasureGameData.totalKeys = collectibleItems.size();
}

void GameInteraction::DestroyBlock(const glm::vec3& position) {
    
    EventData eventData(EventType::BLOCK_DESTROYED, position);
    eventSystem->TriggerEvent(eventData);
    
    if (debugMode) {
        std::cout << "Block destroyed at (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    }
}

void GameInteraction::PlaceBlock(const glm::vec3& position) {
    
    if (debugMode) {
        std::cout << "Block placed at (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    }
}

void GameInteraction::SetGameState(GameState newState) {
    currentState = newState;
    
    switch (currentState) {
        case GameState::IN_GAME:
            std::cout << "Game state: IN_GAME" << std::endl;
            break;
        case GameState::PAUSED:
            std::cout << "Game state: PAUSED" << std::endl;
            break;
        case GameState::MAIN_MENU:
            std::cout << "Game state: MAIN_MENU" << std::endl;
            break;
        case GameState::SETTINGS:
            std::cout << "Game state: SETTINGS" << std::endl;
            break;
        default:
            break;
    }
}

void GameInteraction::UpdateCurrentTask() {
    int collectedKeys = 0;
    int totalKeys = 0;
    int collectedChests = 0;
    
    for (const auto& item : collectibleItems) {
        if (item->GetType() == CollectibleType::KEY) {
            totalKeys++;
            if (item->IsCollected()) {
                collectedKeys++;
            }
        } else if (item->GetType() == CollectibleType::CHEST && item->IsCollected()) {
            collectedChests++;
        }
    }
    
    
    if (collectedKeys == 0) {
        treasureGameData.currentTask = "Explore the ancient ruins and find the key";
    } else if (collectedKeys < totalKeys) {
        treasureGameData.currentTask = "Great! Find the remaining ancient key";
    } else if (collectedKeys == totalKeys && collectedChests == 0) {
        treasureGameData.currentTask = "Use the key to unlock the treasure chest!";
    } else {
        treasureGameData.currentTask = "Congratulations! You are now a legendary treasure hunter!";
    }
    
    treasureGameData.allTreasuresFound = (collectedKeys == totalKeys);
}

std::string GameInteraction::GetInteractionHint() const {
    return treasureGameData.interactionHint;
}

void GameInteraction::CheckWinCondition() {
    if (treasureGameData.gameWon) {
        return; 
    }
    
    
    bool allKeysCollected = true;
    bool allChestsOpened = true;
    
    for (const auto& item : collectibleItems) {
        if (item->GetType() == CollectibleType::KEY && !item->IsCollected()) {
            allKeysCollected = false;
        }
        if (item->GetType() == CollectibleType::CHEST && !item->IsCollected()) {
            allChestsOpened = false;
        }
    }
    
    if (allKeysCollected && allChestsOpened) {
        
        AudioEventHandler::PlayEventSound(AudioEvent::GAME_VICTORY);
        
        EventData winEvent(EventType::GAME_WON, glm::vec3(0.0f), "victory", treasureGameData.explorationDistance);
        eventSystem->TriggerEvent(winEvent);
        SetGameState(GameState::PAUSED);  
    }
}

int GameInteraction::GetCollectedItemsCount() const {
    int count = 0;
    for (const auto& item : collectibleItems) {
        if (item->IsCollected()) {
            count++;
        }
    }
    return count;
}

void GameInteraction::SetupEventListeners() {
    
    eventSystem->AddListener(EventType::ITEM_COLLECTED, 
        [this](const EventData& data) { OnItemCollected(data); });
    
    eventSystem->AddListener(EventType::KEY_COLLECTED, 
        [this](const EventData& data) { OnKeyCollected(data); });
    
    eventSystem->AddListener(EventType::CHEST_OPENED, 
        [this](const EventData& data) { OnChestOpened(data); });
}

bool GameInteraction::RayTerrainIntersection(const Ray& ray, glm::vec3& hitPoint, float& distance) {
    
    
    const float stepSize = 0.5f;
    const int maxSteps = static_cast<int>(raycastDistance / stepSize);
    
    for (int i = 1; i <= maxSteps; ++i) {
        glm::vec3 testPoint = ray.origin + ray.direction * (stepSize * i);
        
        if (terrainGenerator) {
            float terrainHeight = terrainGenerator->GetHeightAt(testPoint.x, testPoint.z);
            if (testPoint.y <= terrainHeight) {
                hitPoint = testPoint;
                distance = stepSize * i;
                return true;
            }
        }
    }
    
    return false;
}

bool GameInteraction::RayItemIntersection(const Ray& ray, CollectibleItem* item, float& distance) {
    
    glm::vec3 itemPos = item->GetPosition();
    float itemRadius = item->GetInteractionRange() * 0.5f;
    
    
    glm::vec3 toItem = itemPos - ray.origin;
    float projLength = glm::dot(toItem, ray.direction);
    
    if (projLength < 0.0f) {
        return false; 
    }
    
    glm::vec3 closestPoint = ray.origin + ray.direction * projLength;
    float distToItem = glm::length(closestPoint - itemPos);
    
    if (distToItem <= itemRadius && projLength <= raycastDistance) {
        distance = projLength;
        return true;
    }
    
    return false;
}

void GameInteraction::UpdateGameData(float deltaTime, const glm::vec3& playerPos) {
    treasureGameData.gameTime += deltaTime;
    treasureGameData.playerPosition = playerPos;
    treasureGameData.keysCollected = GetCollectedItemsCount();
    
    
    EventData moveEvent(EventType::PLAYER_MOVE, playerPos);
    eventSystem->TriggerEvent(moveEvent);
}

void GameInteraction::OnItemCollected(const EventData& eventData) {
    treasureGameData.explorationDistance += eventData.value;
    std::cout << "Item collected! Exploration Distance: " << treasureGameData.explorationDistance << std::endl;
}

void GameInteraction::OnKeyCollected(const EventData& eventData) {
    treasureGameData.explorationDistance += eventData.value;
    std::cout << "Key collected: " << eventData.objectName << "! Exploration Distance: " << treasureGameData.explorationDistance << std::endl;
}

void GameInteraction::OnChestOpened(const EventData& eventData) {
    treasureGameData.explorationDistance += eventData.value;
    std::cout << "Chest opened: " << eventData.objectName << "! Exploration Distance: " << treasureGameData.explorationDistance << std::endl;
}
