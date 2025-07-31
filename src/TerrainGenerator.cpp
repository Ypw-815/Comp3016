#include "TerrainGenerator.h"
#include "Shader.h"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <cfloat>

TerrainGenerator::TerrainGenerator(int chunkSize, float chunkScale)
    : m_chunkSize(chunkSize)
    , m_chunkScale(chunkScale)
    , m_noiseScale(0.05f)
    , m_heightScale(10.0f)
    , m_octaves(4)
    , m_lastCenterPos(-9999.0f, -9999.0f)
    , m_renderDistance(100.0f)
    , m_terrainUpdated(false)
{
    std::cout << "Terrain Generator initialized:" << std::endl;
    std::cout << "  Chunk Size: " << m_chunkSize << "x" << m_chunkSize << std::endl;
    std::cout << "  Chunk Scale: " << m_chunkScale << std::endl;
    std::cout << "  Height Scale: " << m_heightScale << std::endl;
}

TerrainGenerator::~TerrainGenerator() {
    
    for (auto& chunk : m_chunks) {
        if (chunk && chunk->isGenerated) {
            glDeleteVertexArrays(1, &chunk->VAO);
            glDeleteBuffers(1, &chunk->VBO);
            glDeleteBuffers(1, &chunk->EBO);
        }
    }
}

void TerrainGenerator::GenerateTerrainAt(const glm::vec2& centerPos) {
    
    glm::vec2 posDiff = centerPos - m_lastCenterPos;
    if (glm::length(posDiff) < m_chunkScale * 0.5f) {
        return; 
    }
    
    m_lastCenterPos = centerPos;
    bool newChunksGenerated = false;
    
    
    int centerChunkX = static_cast<int>(centerPos.x / m_chunkScale);
    int centerChunkZ = static_cast<int>(centerPos.y / m_chunkScale);
    
    int radius = 3; 
    
    for (int x = centerChunkX - radius; x <= centerChunkX + radius; x++) {
        for (int z = centerChunkZ - radius; z <= centerChunkZ + radius; z++) {
            
            bool exists = false;
            for (const auto& chunk : m_chunks) {
                if (chunk && 
                    abs(static_cast<int>(chunk->vertices[0].position.x / m_chunkScale) - x) < 1 &&
                    abs(static_cast<int>(chunk->vertices[0].position.z / m_chunkScale) - z) < 1) {
                    exists = true;
                    break;
                }
            }
            
            if (!exists) {
                auto newChunk = std::unique_ptr<TerrainChunk>(CreateChunk(x, z));
                if (newChunk) {
                    m_chunks.push_back(std::move(newChunk));
                    newChunksGenerated = true;
                }
            }
        }
    }
    
    
    CleanupDistantChunks(centerPos);
    
    
    if (newChunksGenerated) {
        m_terrainUpdated = true;
    }
    
    std::cout << "Generated terrain at (" << centerPos.x << ", " << centerPos.y 
              << ") - Total chunks: " << m_chunks.size() << std::endl;
}

TerrainChunk* TerrainGenerator::CreateChunk(int chunkX, int chunkZ) {
    auto chunk = new TerrainChunk();
    
    try {
        GenerateChunkVertices(chunk, chunkX, chunkZ);
        CalculateNormals(chunk);
        AssignBiomeColors(chunk);
        SetupChunkBuffers(chunk);
        chunk->isGenerated = true;
        
        return chunk;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create chunk (" << chunkX << ", " << chunkZ << "): " 
                  << e.what() << std::endl;
        delete chunk;
        return nullptr;
    }
}

void TerrainGenerator::GenerateChunkVertices(TerrainChunk* chunk, int chunkX, int chunkZ) {
    chunk->vertices.clear();
    chunk->indices.clear();
    
    float startX = chunkX * m_chunkScale;
    float startZ = chunkZ * m_chunkScale;
    float stepSize = m_chunkScale / (m_chunkSize - 1);
    
    
    for (int z = 0; z < m_chunkSize; z++) {
        for (int x = 0; x < m_chunkSize; x++) {
            TerrainVertex vertex;
            
            float worldX = startX + x * stepSize;
            float worldZ = startZ + z * stepSize;
            float height = GetHeightNoise(worldX, worldZ);
            
            vertex.position = glm::vec3(worldX, height, worldZ);
            vertex.texCoord = glm::vec2(
                static_cast<float>(x) / (m_chunkSize - 1),
                static_cast<float>(z) / (m_chunkSize - 1)
            );
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f); 
            vertex.color = glm::vec3(0.5f); 
            
            chunk->vertices.push_back(vertex);
        }
    }
    
    
    for (int z = 0; z < m_chunkSize - 1; z++) {
        for (int x = 0; x < m_chunkSize - 1; x++) {
            unsigned int topLeft = z * m_chunkSize + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (z + 1) * m_chunkSize + x;
            unsigned int bottomRight = bottomLeft + 1;
            
            
            chunk->indices.push_back(topLeft);
            chunk->indices.push_back(bottomLeft);
            chunk->indices.push_back(topRight);
            
            
            chunk->indices.push_back(topRight);
            chunk->indices.push_back(bottomLeft);
            chunk->indices.push_back(bottomRight);
        }
    }
}

void TerrainGenerator::CalculateNormals(TerrainChunk* chunk) {
    
    for (auto& vertex : chunk->vertices) {
        vertex.normal = glm::vec3(0.0f);
    }
    
    
    for (size_t i = 0; i < chunk->indices.size(); i += 3) {
        unsigned int i0 = chunk->indices[i];
        unsigned int i1 = chunk->indices[i + 1];
        unsigned int i2 = chunk->indices[i + 2];
        
        glm::vec3 v0 = chunk->vertices[i0].position;
        glm::vec3 v1 = chunk->vertices[i1].position;
        glm::vec3 v2 = chunk->vertices[i2].position;
        
        glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        
        chunk->vertices[i0].normal += normal;
        chunk->vertices[i1].normal += normal;
        chunk->vertices[i2].normal += normal;
    }
    
    
    for (auto& vertex : chunk->vertices) {
        vertex.normal = glm::normalize(vertex.normal);
    }
}

void TerrainGenerator::AssignBiomeColors(TerrainChunk* chunk) {
    for (auto& vertex : chunk->vertices) {
        float height = vertex.position.y;
        float moisture = GetMoistureNoise(vertex.position.x, vertex.position.z);
        
        BiomeType biome = DetermineBiome(height, moisture);
        vertex.color = GetBiomeColor(biome, height);
        
        
        if (&vertex == &chunk->vertices[chunk->vertices.size() / 2]) {
            chunk->biome = biome;
        }
    }
}

void TerrainGenerator::SetupChunkBuffers(TerrainChunk* chunk) {
    glGenVertexArrays(1, &chunk->VAO);
    glGenBuffers(1, &chunk->VBO);
    glGenBuffers(1, &chunk->EBO);
    
    glBindVertexArray(chunk->VAO);
    
    
    glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO);
    glBufferData(GL_ARRAY_BUFFER, 
                 chunk->vertices.size() * sizeof(TerrainVertex), 
                 chunk->vertices.data(), 
                 GL_STATIC_DRAW);
    
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                 chunk->indices.size() * sizeof(unsigned int), 
                 chunk->indices.data(), 
                 GL_STATIC_DRAW);
    
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), 
                         (void*)offsetof(TerrainVertex, normal));
    glEnableVertexAttribArray(1);
    
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), 
                         (void*)offsetof(TerrainVertex, texCoord));
    glEnableVertexAttribArray(2);
    
    
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), 
                         (void*)offsetof(TerrainVertex, color));
    glEnableVertexAttribArray(3);
    
    glBindVertexArray(0);
}

float TerrainGenerator::GetHeightNoise(float x, float z) {
    float height = 0.0f;
    float amplitude = m_heightScale;
    float frequency = m_noiseScale;
    
    
    for (int i = 0; i < m_octaves; i++) {
        height += glm::simplex(glm::vec2(x * frequency, z * frequency)) * amplitude;
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }
    
    
    float groundLevel = -0.1f;  
    height = glm::max(height, groundLevel);
    
    return height;
}

float TerrainGenerator::GetMoistureNoise(float x, float z) {
    
    return glm::simplex(glm::vec2((x + 1000.0f) * m_noiseScale * 0.5f, 
                                 (z + 1000.0f) * m_noiseScale * 0.5f));
}

BiomeType TerrainGenerator::DetermineBiome(float height, float moisture) {
    
    if (height > m_heightScale * 0.6f) {
        return BiomeType::MOUNTAIN;
    } else if (moisture < -0.2f) {
        return BiomeType::DESERT;
    } else if (moisture > 0.3f) {
        return BiomeType::FOREST;
    } else {
        return BiomeType::GRASSLAND;
    }
}

glm::vec3 TerrainGenerator::GetBiomeColor(BiomeType biome, float height) {
    switch (biome) {
        case BiomeType::GRASSLAND:
            return glm::vec3(0.4f, 0.8f, 0.2f); 
        case BiomeType::MOUNTAIN:
            return glm::mix(glm::vec3(0.5f, 0.5f, 0.5f), 
                           glm::vec3(1.0f, 1.0f, 1.0f), 
                           std::max(0.0f, (height - m_heightScale * 0.7f) / (m_heightScale * 0.3f)));
        case BiomeType::DESERT:
            return glm::vec3(0.9f, 0.8f, 0.4f); 
        case BiomeType::FOREST:
            return glm::vec3(0.2f, 0.6f, 0.1f); 
        default:
            return glm::vec3(0.5f, 0.5f, 0.5f); 
    }
}

void TerrainGenerator::RenderTerrain(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    shader.Use();
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);
    
    for (const auto& chunk : m_chunks) {
        if (chunk && chunk->isGenerated) {
            glm::mat4 model = glm::mat4(1.0f);
            shader.SetMat4("model", model);
            
            glBindVertexArray(chunk->VAO);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(chunk->indices.size()), GL_UNSIGNED_INT, 0);
        }
    }
    
    glBindVertexArray(0);
}

float TerrainGenerator::GetHeightAt(float x, float z) {
    return GetHeightNoise(x, z);
}

glm::vec3 TerrainGenerator::FindSafeSpawnPoint(float preferredX, float preferredZ) {
    std::cout << "Finding safe spawn point near (" << preferredX << ", " << preferredZ << ")" << std::endl;
    
    
    float height = GetHeightAt(preferredX, preferredZ);
    
    
    float tolerance = 0.5f;  
    bool isFlat = true;
    
    
    for (float x = preferredX - 1.5f; x <= preferredX + 1.5f; x += 0.5f) {
        for (float z = preferredZ - 1.5f; z <= preferredZ + 1.5f; z += 0.5f) {
            float checkHeight = GetHeightAt(x, z);
            if (abs(checkHeight - height) > tolerance) {
                isFlat = false;
                break;
            }
        }
        if (!isFlat) break;
    }
    
    if (isFlat) {
        std::cout << "Preferred position is suitable. Height: " << height << std::endl;
        return glm::vec3(preferredX, height, preferredZ);
    }
    
    
    std::cout << "Preferred position too steep, searching for nearby flat area..." << std::endl;
    
    float searchRadius = 10.0f;  
    float bestX = preferredX, bestZ = preferredZ;
    float bestHeight = height;
    float minSteepness = FLT_MAX;
    
    for (float radius = 2.0f; radius <= searchRadius; radius += 1.0f) {
        for (float angle = 0.0f; angle < 360.0f; angle += 30.0f) {
            float radians = glm::radians(angle);
            float testX = preferredX + radius * cos(radians);
            float testZ = preferredZ + radius * sin(radians);
            
            float testHeight = GetHeightAt(testX, testZ);
            
            
            float maxHeightDiff = 0.0f;
            for (float x = testX - 1.5f; x <= testX + 1.5f; x += 0.5f) {
                for (float z = testZ - 1.5f; z <= testZ + 1.5f; z += 0.5f) {
                    float checkHeight = GetHeightAt(x, z);
                    maxHeightDiff = glm::max(maxHeightDiff, abs(checkHeight - testHeight));
                }
            }
            
            if (maxHeightDiff < minSteepness) {
                minSteepness = maxHeightDiff;
                bestX = testX;
                bestZ = testZ;
                bestHeight = testHeight;
                
                
                if (maxHeightDiff < tolerance) {
                    std::cout << "Found flat area at (" << bestX << ", " << bestZ << ") height: " << bestHeight << std::endl;
                    return glm::vec3(bestX, bestHeight, bestZ);
                }
            }
        }
    }
    
    
    std::cout << "Using best available position at (" << bestX << ", " << bestZ << ") height: " << bestHeight << std::endl;
    return glm::vec3(bestX, bestHeight, bestZ);
}

void TerrainGenerator::UpdateLOD(const glm::vec3& cameraPos) {
    
    
    GenerateTerrainAt(glm::vec2(cameraPos.x, cameraPos.z));
}

bool TerrainGenerator::IsChunkVisible(int chunkX, int chunkZ, const glm::vec3& cameraPos) {
    float chunkCenterX = chunkX * m_chunkScale + m_chunkScale * 0.5f;
    float chunkCenterZ = chunkZ * m_chunkScale + m_chunkScale * 0.5f;
    
    float distance = glm::length(glm::vec2(chunkCenterX - cameraPos.x, chunkCenterZ - cameraPos.z));
    return distance <= m_renderDistance;
}

void TerrainGenerator::CleanupDistantChunks(const glm::vec2& centerPos) {
    auto it = std::remove_if(m_chunks.begin(), m_chunks.end(),
        [this, centerPos](const std::unique_ptr<TerrainChunk>& chunk) {
            if (!chunk || chunk->vertices.empty()) return true;
            
            glm::vec3 chunkCenter = chunk->vertices[chunk->vertices.size() / 2].position;
            float distance = glm::length(glm::vec2(chunkCenter.x - centerPos.x, chunkCenter.z - centerPos.y));
            
            if (distance > m_renderDistance * 1.5f) {
                
                if (chunk->isGenerated) {
                    glDeleteVertexArrays(1, &chunk->VAO);
                    glDeleteBuffers(1, &chunk->VBO);
                    glDeleteBuffers(1, &chunk->EBO);
                }
                return true;
            }
            return false;
        });
    
    m_chunks.erase(it, m_chunks.end());
}

void TerrainGenerator::GetCollisionData(std::vector<glm::vec3>& vertices, std::vector<unsigned int>& indices) {
    vertices.clear();
    indices.clear();
    
    
    unsigned int vertexOffset = 0;
    
    for (const auto& chunk : m_chunks) {
        if (!chunk || !chunk->isGenerated) continue;
        
        
        for (const auto& vertex : chunk->vertices) {
            vertices.push_back(vertex.position);
        }
        
        
        for (const auto& index : chunk->indices) {
            indices.push_back(index + vertexOffset);
        }
        
        vertexOffset += static_cast<unsigned int>(chunk->vertices.size());
    }
    
    std::cout << "Collected collision data: " << vertices.size() << " vertices, " 
              << (indices.size() / 3) << " triangles from " << m_chunks.size() << " chunks" << std::endl;
}

bool TerrainGenerator::HasTerrainUpdated() {
    return m_terrainUpdated;
}

void TerrainGenerator::ResetTerrainUpdateFlag() {
    m_terrainUpdated = false;
}
