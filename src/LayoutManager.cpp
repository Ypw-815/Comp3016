#include "LayoutManager.h"
#include "TerrainPlacement.h"
#include <random>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

LayoutManager::LayoutManager(TerrainGenerator* terrain) 
    : terrainGenerator(terrain) {
    InitializeDefaultLayout();
}

LayoutManager::~LayoutManager() {
}

std::vector<LayoutManager::LayoutZone> LayoutManager::CreateZonedLayout() {
    std::vector<LayoutZone> layoutZones;
    
    
    LayoutZone coreZone;
    coreZone.center = glm::vec3(0.0f, 0.0f, 0.0f);
    coreZone.radius = 8.0f;
    coreZone.theme = "treasure";
    coreZone.maxObjects = 3; 
    coreZone.minDistance = 2.0f;
    layoutZones.push_back(coreZone);
    
    
    
    LayoutZone forestZone;
    forestZone.center = glm::vec3(15.0f, 0.0f, 15.0f);
    forestZone.radius = 12.0f;
    forestZone.theme = "forest";
    forestZone.maxObjects = 6;
    forestZone.minDistance = 3.0f;
    layoutZones.push_back(forestZone);
    
    
    LayoutZone desertZone;
    desertZone.center = glm::vec3(15.0f, 0.0f, -15.0f);
    desertZone.radius = 12.0f;
    desertZone.theme = "desert";
    desertZone.maxObjects = 4;
    desertZone.minDistance = 4.0f;
    layoutZones.push_back(desertZone);
    
    
    LayoutZone mountainZone;
    mountainZone.center = glm::vec3(-15.0f, 0.0f, 15.0f);
    mountainZone.radius = 12.0f;
    mountainZone.theme = "mountain";
    mountainZone.maxObjects = 3;
    mountainZone.minDistance = 5.0f;
    layoutZones.push_back(mountainZone);
    
    
    LayoutZone grasslandZone;
    grasslandZone.center = glm::vec3(-15.0f, 0.0f, -15.0f);
    grasslandZone.radius = 12.0f;
    grasslandZone.theme = "grassland";
    grasslandZone.maxObjects = 5;
    grasslandZone.minDistance = 3.5f;
    layoutZones.push_back(grasslandZone);
    
    return layoutZones;
}

void LayoutManager::PlaceObjectsInZone(const LayoutZone& zone, std::vector<LayoutObject>& objects) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> radiusDist(0.3f, 0.9f);
    
    std::vector<glm::vec3> usedPositions;
    
    for (int i = 0; i < zone.maxObjects; ++i) {
        glm::vec3 position;
        bool validPosition = false;
        int attempts = 0;
        const int maxAttempts = 50;
        
        while (!validPosition && attempts < maxAttempts) {
            
            float angle = angleDist(gen);
            float radius = radiusDist(gen) * zone.radius;
            
            position = zone.center + glm::vec3(
                radius * cos(angle),
                0.0f,
                radius * sin(angle)
            );
            
            
            if (IsWithinTerrainBounds(position)) {
                
                position = TerrainPlacement::PlaceOnTerrain(position, terrainGenerator, 0.5f);
                
                
                if (CheckMinimumDistance(position, usedPositions, zone.minDistance)) {
                    validPosition = true;
                    usedPositions.push_back(position);
                }
            }
            attempts++;
        }
        
        if (validPosition) {
            LayoutObject obj;
            obj.position = position;
            obj.rotation = glm::vec3(0.0f, angleDist(gen), 0.0f); 
            obj.scale = glm::vec3(1.0f);
            obj.theme = zone.theme;
            
            
            if (zone.theme == "treasure") {
                
                std::uniform_int_distribution<int> treasureTypeDist(0, 1);
                obj.type = (treasureTypeDist(gen) == 0) ? "chest" : "key";
            } else {
                
                std::uniform_int_distribution<int> typeDist(0, 2);
                int typeChoice = typeDist(gen);
                if (typeChoice == 0) obj.type = "chest";
                else if (typeChoice == 1) obj.type = "key";
                else obj.type = "decoration";
            }
            
            objects.push_back(obj);
        }
    }
}

glm::vec3 LayoutManager::GetThemeBasedPosition(const std::string& theme, glm::vec3 basePos) {
    glm::vec3 adjustedPos = basePos;
    
    if (theme == "forest") {
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> offset(-1.0f, 1.0f);
        adjustedPos.x += offset(gen);
        adjustedPos.z += offset(gen);
    }
    else if (theme == "desert") {
        
        adjustedPos.x = round(adjustedPos.x / 2.0f) * 2.0f;
        adjustedPos.z = round(adjustedPos.z / 2.0f) * 2.0f;
    }
    else if (theme == "mountain") {
        
        if (terrainGenerator) {
            float height = terrainGenerator->GetHeightAt(adjustedPos.x, adjustedPos.z);
            
            for (int dx = -2; dx <= 2; dx++) {
                for (int dz = -2; dz <= 2; dz++) {
                    float checkHeight = terrainGenerator->GetHeightAt(adjustedPos.x + dx, adjustedPos.z + dz);
                    if (abs(checkHeight - height) < 0.5f) {
                        adjustedPos.x += dx * 0.5f;
                        adjustedPos.z += dz * 0.5f;
                        break;
                    }
                }
            }
        }
    }
    else if (theme == "grassland") {
        
        adjustedPos.x = round(adjustedPos.x / 1.5f) * 1.5f;
        adjustedPos.z = round(adjustedPos.z / 1.5f) * 1.5f;
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> offset(-0.3f, 0.3f);
        adjustedPos.x += offset(gen);
        adjustedPos.z += offset(gen);
    }
    
    return adjustedPos;
}

bool LayoutManager::IsPositionSuitable(glm::vec3 position, const std::string& objectType) {
    if (!IsWithinTerrainBounds(position)) {
        return false;
    }
    
    if (terrainGenerator) {
        
        if (!TerrainPlacement::IsPositionFlat(position, terrainGenerator, 0.8f)) {
            return false;
        }
        
        
        if (objectType == "chest") {
            if (!TerrainPlacement::IsPositionFlat(position, terrainGenerator, 0.5f)) {
                return false;
            }
        }
    }
    
    return true;
}

void LayoutManager::InitializeDefaultLayout() {
    zones = CreateZonedLayout();
}

bool LayoutManager::CheckMinimumDistance(glm::vec3 newPos, const std::vector<glm::vec3>& existingPositions, float minDist) {
    for (const auto& pos : existingPositions) {
        float distance = length(newPos - pos);
        if (distance < minDist) {
            return false;
        }
    }
    return true;
}

glm::vec3 LayoutManager::GenerateRandomPositionInCircle(glm::vec3 center, float radius) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> radiusDist(0.0f, 1.0f);
    
    float angle = angleDist(gen);
    float r = sqrt(radiusDist(gen)) * radius; 
    
    return center + glm::vec3(r * cos(angle), 0.0f, r * sin(angle));
}

bool LayoutManager::IsWithinTerrainBounds(glm::vec3 position) {
    
    const float terrainSize = 40.0f; 
    return (abs(position.x) <= terrainSize && abs(position.z) <= terrainSize);
}

glm::vec3 LayoutManager::GetRandomPositionInZone(const LayoutZone& zone) const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> radiusDist(0.0f, zone.radius);

    float angle = angleDist(gen);
    float radius = radiusDist(gen);

    glm::vec3 position = zone.center + glm::vec3(radius * cos(angle), 0.0f, radius * sin(angle));
    return position;
}
