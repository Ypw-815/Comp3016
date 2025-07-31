#pragma once

#include <glm/glm.hpp>
#include "TerrainGenerator.h"


class TerrainPlacement {
public:

    static glm::vec3 PlaceOnTerrain(glm::vec3 position, TerrainGenerator* terrain, float objectHeight = 0.0f);
    

    static bool IsPositionFlat(glm::vec3 position, TerrainGenerator* terrain, float tolerance = 0.5f);
    

    static glm::vec3 FindNearbyFlatPosition(glm::vec3 center, TerrainGenerator* terrain, float searchRadius = 5.0f);

private:
    
    static float GetAreaHeightVariation(glm::vec3 center, TerrainGenerator* terrain, float checkRadius = 1.0f);
};
