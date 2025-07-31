/**
 * @file TerrainGenerator.h
 * @brief Procedural terrain generation and rendering system
 * 
 * Provides a comprehensive system for generating and rendering infinite
 * procedural terrain using Perlin noise. Features dynamic chunk loading,
 * multiple biome types, level-of-detail optimization, and physics collision
 * mesh generation.
 * 
 * Key Features:
 * - Infinite terrain generation using chunk-based streaming
 * - Multiple biome types with distinct visual characteristics
 * - Perlin noise-based height and moisture generation
 * - Automatic normal calculation for realistic lighting
 * - Physics collision mesh integration
 * - Level-of-detail (LOD) management for performance
 * - Safe spawn point detection for gameplay
 */

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>

#include <vector>
#include <memory>

/**
 * @brief Terrain vertex structure with complete rendering data
 * 
 * Contains all necessary data for rendering terrain vertices including
 * position, lighting normals, texture coordinates, and vertex colors
 * for biome-based terrain visualization.
 */
struct TerrainVertex {
    glm::vec3 position;  // 3D world position of the vertex
    glm::vec3 normal;    // Normal vector for lighting calculations
    glm::vec2 texCoord;  // Texture coordinates for texture mapping
    glm::vec3 color;     // Vertex color for biome identification
};

/**
 * @brief Biome types for terrain variety
 * 
 * Defines different environmental zones that affect terrain
 * appearance, color schemes, and potential gameplay mechanics.
 */
enum class BiomeType {
    GRASSLAND,  // Green plains and meadows
    MOUNTAIN,   // Rocky high-altitude terrain
    DESERT,     // Sandy/rocky arid regions
    FOREST      // Dense woodland areas
};

/**
 * @brief Terrain chunk data structure
 * 
 * Represents a single chunk of terrain geometry with all necessary
 * rendering data and metadata. Chunks are the basic unit of terrain
 * generation and management for infinite world streaming.
 */
struct TerrainChunk {
    std::vector<TerrainVertex> vertices;  // All vertices in this chunk
    std::vector<unsigned int> indices;    // Triangle indices for rendering
    unsigned int VAO, VBO, EBO;          // OpenGL buffer objects
    BiomeType biome;                     // Dominant biome type for this chunk
    bool isGenerated;                    // Whether chunk geometry is ready
    
    /**
     * @brief Default constructor initializing chunk to safe state
     */
    TerrainChunk() : VAO(0), VBO(0), EBO(0), biome(BiomeType::GRASSLAND), isGenerated(false) {}
};

/**
 * @brief Procedural terrain generation and management system
 * 
 * Generates infinite terrain using Perlin noise algorithms and manages
 * terrain chunks dynamically based on camera position. Provides seamless
 * terrain streaming, multiple biome support, and integration with physics
 * systems for collision detection.
 * 
 * Technical Details:
 * - Uses multi-octave Perlin noise for realistic height variation
 * - Implements chunk-based LOD system for performance optimization
 * - Generates collision meshes compatible with PhysX physics engine
 * - Supports real-time parameter adjustment for terrain customization
 * - Automatic cleanup of distant chunks to manage memory usage
 */
class TerrainGenerator {
public:
    /**
     * @brief Constructor with terrain generation parameters
     * 
     * @param chunkSize Number of vertices per chunk side (default: 64)
     * @param chunkScale World-space size of each chunk (default: 1.0)
     */
    TerrainGenerator(int chunkSize = 64, float chunkScale = 1.0f);
    
    /**
     * @brief Destructor - cleanup OpenGL resources
     */
    ~TerrainGenerator();

    /**
     * @brief Generate terrain chunks around specified position
     * 
     * Creates new terrain chunks around the given center position and
     * removes chunks that are too far away. This enables infinite terrain
     * by only maintaining chunks near the camera/player.
     * 
     * @param centerPos World position to center terrain generation around
     */
    void GenerateTerrainAt(const glm::vec2& centerPos);
    
    /**
     * @brief Render all visible terrain chunks
     * 
     * Renders all currently loaded terrain chunks using the provided
     * shader and camera matrices. Performs frustum culling and LOD
     * selection for optimal performance.
     * 
     * @param shader Shader program for terrain rendering
     * @param view Camera view transformation matrix
     * @param projection Camera projection matrix
     */
    void RenderTerrain(class Shader& shader, const glm::mat4& view, const glm::mat4& projection);
    
    /**
     * @brief Update level-of-detail based on camera position
     * 
     * Adjusts terrain detail levels based on distance from camera
     * to optimize rendering performance. Distant chunks use lower
     * polygon counts while near chunks maintain full detail.
     * 
     * @param cameraPos Current camera world position
     */
    void UpdateLOD(const glm::vec3& cameraPos);
    
    /**
     * @brief Get terrain height at specific world coordinates
     * 
     * Samples the terrain height function at the specified position.
     * Useful for placing objects on terrain surface or character movement.
     * 
     * @param x World X coordinate
     * @param z World Z coordinate
     * @return Terrain height (Y coordinate) at the specified position
     */
    float GetHeightAt(float x, float z);
    
    /**
     * @brief Find safe spawn location near preferred coordinates
     * 
     * Searches for a suitable spawn point near the preferred location
     * by checking terrain height, slope, and potentially biome restrictions.
     * Ensures objects/characters spawn on stable ground.
     * 
     * @param preferredX Preferred X world coordinate
     * @param preferredZ Preferred Z world coordinate
     * @return Safe spawn position (includes Y coordinate)
     */
    glm::vec3 FindSafeSpawnPoint(float preferredX, float preferredZ);
    
    /**
     * @brief Extract collision mesh data for physics engine
     * 
     * Generates simplified collision geometry from current terrain chunks
     * suitable for use with physics engines like PhysX. Optimizes triangle
     * count while maintaining collision accuracy.
     * 
     * @param vertices Output vector for collision mesh vertices
     * @param indices Output vector for collision mesh triangle indices
     */
    void GetCollisionData(std::vector<glm::vec3>& vertices, std::vector<unsigned int>& indices);
    
    /**
     * @brief Check if terrain has been modified since last query
     * 
     * Useful for systems that need to know when to update cached
     * terrain data (like physics collision meshes).
     * 
     * @return True if terrain has been updated recently
     */
    bool HasTerrainUpdated();
    
    /**
     * @brief Reset the terrain update flag
     * 
     * Call after handling terrain updates to reset the flag
     * for next update detection cycle.
     */
    void ResetTerrainUpdateFlag();
    
    /**
     * @brief Runtime parameter adjustment methods
     * 
     * Allows real-time modification of terrain generation parameters
     * for testing and customization. Changes affect newly generated chunks.
     */
    void SetNoiseScale(float scale) { m_noiseScale = scale; }    // Frequency of height variation
    void SetHeightScale(float scale) { m_heightScale = scale; }  // Amplitude of height variation
    void SetOctaves(int octaves) { m_octaves = octaves; }        // Detail levels in noise

private:
    // Core terrain parameters
    int m_chunkSize;           // Vertices per chunk side (e.g., 64x64)
    float m_chunkScale;        // World-space size of each chunk
    float m_noiseScale;        // Frequency scale for Perlin noise
    float m_heightScale;       // Amplitude scale for terrain height
    int m_octaves;             // Number of noise octaves for detail layers
    
    // Chunk management
    std::vector<std::unique_ptr<TerrainChunk>> m_chunks;  // Active terrain chunks
    glm::vec2 m_lastCenterPos;   // Last center position for change detection
    float m_renderDistance;      // Maximum distance for chunk visibility
    bool m_terrainUpdated;       // Flag indicating recent terrain changes
    
    // Core chunk generation pipeline
    /**
     * @brief Create new terrain chunk at specified grid coordinates
     * @param chunkX Chunk grid X coordinate
     * @param chunkZ Chunk grid Z coordinate
     * @return Pointer to newly created terrain chunk
     */
    TerrainChunk* CreateChunk(int chunkX, int chunkZ);
    
    /**
     * @brief Generate vertices and geometry for a terrain chunk
     * @param chunk Target chunk to populate with geometry
     * @param chunkX Chunk grid X coordinate
     * @param chunkZ Chunk grid Z coordinate
     */
    void GenerateChunkVertices(TerrainChunk* chunk, int chunkX, int chunkZ);
    
    /**
     * @brief Calculate normal vectors for proper lighting
     * @param chunk Chunk to calculate normals for
     */
    void CalculateNormals(TerrainChunk* chunk);
    
    /**
     * @brief Assign colors based on biome and height
     * @param chunk Chunk to apply biome coloring to
     */
    void AssignBiomeColors(TerrainChunk* chunk);
    
    /**
     * @brief Create OpenGL buffers for chunk rendering
     * @param chunk Chunk to setup rendering buffers for
     */
    void SetupChunkBuffers(TerrainChunk* chunk);
    
    // Noise generation and biome determination
    /**
     * @brief Generate height value using Perlin noise
     * @param x World X coordinate
     * @param z World Z coordinate
     * @return Height value at specified coordinates
     */
    float GetHeightNoise(float x, float z);
    
    /**
     * @brief Generate moisture value for biome determination
     * @param x World X coordinate
     * @param z World Z coordinate
     * @return Moisture value (0-1) at specified coordinates
     */
    float GetMoistureNoise(float x, float z);
    
    /**
     * @brief Determine biome type based on height and moisture
     * @param height Terrain height value
     * @param moisture Moisture level (0-1)
     * @return Appropriate biome type for the conditions
     */
    BiomeType DetermineBiome(float height, float moisture);
    
    /**
     * @brief Get color for specific biome and height
     * @param biome Target biome type
     * @param height Terrain height for color variation
     * @return RGB color vector for the biome
     */
    glm::vec3 GetBiomeColor(BiomeType biome, float height);
    
    // Performance optimization
    /**
     * @brief Check if chunk should be rendered based on camera position
     * @param chunkX Chunk grid X coordinate
     * @param chunkZ Chunk grid Z coordinate
     * @param cameraPos Current camera world position
     * @return True if chunk is within visible range
     */
    bool IsChunkVisible(int chunkX, int chunkZ, const glm::vec3& cameraPos);
    
    /**
     * @brief Remove chunks that are too far from center position
     * @param centerPos Current center position for distance calculation
     */
    void CleanupDistantChunks(const glm::vec2& centerPos);
};
