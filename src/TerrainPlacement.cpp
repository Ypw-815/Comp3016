#include "TerrainPlacement.h"
#include <iostream>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

glm::vec3 TerrainPlacement::PlaceOnTerrain(glm::vec3 position, TerrainGenerator* terrain, float objectHeight) {
    if (!terrain) {
        std::cerr << "TerrainPlacement: Invalid terrain generator!" << std::endl;
        return position;
    }
    
    
    float terrainHeight = terrain->GetHeightAt(position.x, position.z);
    
    
    float safetyOffset = 1.0f;  
    
    
    glm::vec3 adjustedPosition = position;
    adjustedPosition.y = terrainHeight + objectHeight + safetyOffset;
    
    
    float minHeight = 0.5f;  
    if (adjustedPosition.y < minHeight) {
        adjustedPosition.y = minHeight;
    }
    
    std::cout << "TerrainPlacement: Object at (" << position.x << ", " << position.z 
              << ") placed on terrain height " << terrainHeight 
              << ", final Y: " << adjustedPosition.y << std::endl;
    
    return adjustedPosition;
}

bool TerrainPlacement::IsPositionFlat(glm::vec3 position, TerrainGenerator* terrain, float tolerance) {
    if (!terrain) return false;
    
    float variation = GetAreaHeightVariation(position, terrain, 2.0f);  
    return variation <= tolerance;
}

glm::vec3 TerrainPlacement::FindNearbyFlatPosition(glm::vec3 center, TerrainGenerator* terrain, float searchRadius) {
    if (!terrain) return center;
    
    std::cout << "TerrainPlacement: Searching for flat position near (" 
              << center.x << ", " << center.z << ") within radius " << searchRadius << std::endl;
    
    glm::vec3 bestPosition = center;
    float centerHeight = terrain->GetHeightAt(center.x, center.z);
    bestPosition.y = centerHeight;
    
    float minVariation = GetAreaHeightVariation(center, terrain, 1.5f);
    
    
    if (minVariation <= 0.3f) {
        std::cout << "TerrainPlacement: Center position is already flat (variation: " 
                  << minVariation << ")" << std::endl;
        return bestPosition;
    }
    
    
    for (float radius = 1.0f; radius <= searchRadius; radius += 1.0f) {
        for (float angle = 0.0f; angle < 360.0f; angle += 30.0f) {
            float radians = glm::radians(angle);
            glm::vec3 testPos = center;
            testPos.x += radius * cos(radians);
            testPos.z += radius * sin(radians);
            
            float variation = GetAreaHeightVariation(testPos, terrain, 1.5f);
            
            if (variation < minVariation) {
                minVariation = variation;
                bestPosition = testPos;
                bestPosition.y = terrain->GetHeightAt(testPos.x, testPos.z);
                
                
                if (variation <= 0.3f) {
                    std::cout << "TerrainPlacement: Found flat position at (" 
                              << bestPosition.x << ", " << bestPosition.z 
                              << ") with variation " << variation << std::endl;
                    return bestPosition;
                }
            }
        }
    }
    
    std::cout << "TerrainPlacement: Best position found at (" 
              << bestPosition.x << ", " << bestPosition.z 
              << ") with variation " << minVariation << std::endl;
    
    return bestPosition;
}

float TerrainPlacement::GetAreaHeightVariation(glm::vec3 center, TerrainGenerator* terrain, float checkRadius) {
    if (!terrain) return 0.0f;
    
    float centerHeight = terrain->GetHeightAt(center.x, center.z);
    float maxDiff = 0.0f;
    
    
    int checkPoints = 8;  
    for (int i = 0; i < checkPoints; i++) {
        float angle = (2.0f * M_PI * i) / checkPoints;
        float checkX = center.x + checkRadius * cos(angle);
        float checkZ = center.z + checkRadius * sin(angle);
        
        float checkHeight = terrain->GetHeightAt(checkX, checkZ);
        float heightDiff = abs(checkHeight - centerHeight);
        maxDiff = glm::max(maxDiff, heightDiff);
    }
    
    return maxDiff;
}
