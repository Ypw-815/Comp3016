/**
 * @file PhysicsManager.h
 * @brief PhysX physics engine integration and management
 * 
 * Provides a comprehensive physics simulation system using NVIDIA PhysX.
 * Manages rigid body dynamics, collision detection, terrain physics,
 * and raycasting for interactive 3D environments.
 * 
 * Features:
 * - Rigid body creation (static and dynamic)
 * - Terrain collision mesh generation
 * - Ground plane and basic collision shapes
 * - Raycasting for ground height and object detection
 * - Automatic physics simulation stepping
 * - Resource management and cleanup
 */

#pragma once

#include <PxPhysicsAPI.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

using namespace physx;

/**
 * @brief Central physics simulation manager using PhysX
 * 
 * Encapsulates the PhysX physics engine and provides a simplified interface
 * for common physics operations in a 3D game environment. Handles the entire
 * physics simulation lifecycle from initialization to cleanup.
 * 
 * Key capabilities:
 * - Physics world simulation with automatic time stepping
 * - Creation of static and dynamic rigid bodies
 * - Complex terrain collision mesh support
 * - Raycasting for various gameplay mechanics
 * - Collision filtering and material properties
 * - Visual debugger integration for development
 */
class PhysicsManager {
public:
    /**
     * @brief Constructor - prepares physics manager
     */
    PhysicsManager();
    
    /**
     * @brief Destructor - ensures proper cleanup
     */
    ~PhysicsManager();

    /**
     * @brief Initialize PhysX physics engine
     * 
     * Sets up the complete PhysX simulation environment including:
     * - Foundation and physics SDK initialization
     * - Scene creation with gravity and simulation parameters
     * - Material setup for surface properties
     * - Visual debugger connection for development
     * 
     * @return True if initialization successful, false otherwise
     */
    bool Initialize();
    
    /**
     * @brief Update physics simulation
     * 
     * Steps the physics simulation forward by the specified time delta.
     * Should be called every frame with the elapsed time to maintain
     * consistent physics behavior regardless of framerate.
     * 
     * @param deltaTime Time elapsed since last update (in seconds)
     */
    void Update(float deltaTime);
    
    /**
     * @brief Clean up all physics resources
     * 
     * Properly releases all PhysX objects and memory to prevent leaks.
     * Should be called before application shutdown.
     */
    void Cleanup();

    /**
     * @brief Create static rigid body (immovable objects)
     * 
     * Creates a static physics body that doesn't move but can collide
     * with dynamic objects. Useful for walls, floors, and static geometry.
     * 
     * @param position World position for the body
     * @param size Dimensions of the box collision shape
     * @return Pointer to created static rigid body
     */
    PxRigidStatic* CreateStaticBody(const glm::vec3& position, const glm::vec3& size);
    
    /**
     * @brief Create dynamic rigid body (movable objects)
     * 
     * Creates a dynamic physics body that responds to forces, gravity,
     * and collisions. Suitable for projectiles, movable objects, etc.
     * 
     * @param position Initial world position
     * @param size Dimensions of the box collision shape
     * @param mass Mass of the object (affects physics response)
     * @return Pointer to created dynamic rigid body
     */
    PxRigidDynamic* CreateDynamicBody(const glm::vec3& position, const glm::vec3& size, float mass = 1.0f);
    
    /**
     * @brief Create dynamic actor with custom geometry
     * 
     * Creates a dynamic rigid body with custom collision geometry
     * rather than just a simple box shape.
     * 
     * @param position Initial world position
     * @param geometry Custom PhysX geometry for collision shape
     * @param mass Mass of the object
     * @return Pointer to created dynamic actor
     */
    PxRigidDynamic* CreateDynamicActor(const glm::vec3& position, const PxGeometry& geometry, float mass = 1.0f);

    /**
     * @brief Create collision mesh from terrain geometry
     * 
     * Generates static collision geometry from terrain mesh data.
     * Allows for accurate collision detection with complex terrain
     * generated from heightmaps or procedural algorithms.
     * 
     * @param vertices Array of terrain vertex positions
     * @param indices Array of triangle indices defining terrain faces
     */
    void CreateTerrainCollision(const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices);
    
    /**
     * @brief Remove all terrain collision geometry
     * 
     * Clears existing terrain collision bodies, useful for
     * regenerating terrain or switching levels.
     */
    void ClearTerrainCollision();
    
    /**
     * @brief Create infinite ground plane
     * 
     * Creates an infinite flat collision plane useful as a basic
     * ground surface that objects can't fall through.
     * 
     * @param y Y-coordinate height of the ground plane
     */
    void CreateGroundPlane(float y = -0.1f);
    
    /**
     * @brief Perform raycast query
     * 
     * Casts a ray through the physics world to detect collisions.
     * Useful for mouse picking, line-of-sight checks, ground detection, etc.
     * 
     * @param origin Starting point of the ray
     * @param direction Direction vector of the ray (should be normalized)
     * @param maxDistance Maximum distance to check along the ray
     * @param hitPoint Output parameter for collision point (if hit occurs)
     * @return True if ray hit something, false otherwise
     */
    bool Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, glm::vec3& hitPoint);
    
    /**
     * @brief Get ground height at position
     * 
     * Performs a downward raycast to find the ground height at a given
     * position. Useful for placing objects on terrain or character movement.
     * 
     * @param position Horizontal position to check
     * @return Y-coordinate of ground height, or large negative value if no ground found
     */
    float GetGroundHeight(const glm::vec3& position);
    
    /**
     * @brief Access to core PhysX objects
     * 
     * Provides direct access to PhysX objects for advanced usage
     * that isn't covered by the simplified interface.
     */
    PxPhysics* GetPhysics() { return m_physics; }
    PxScene* GetScene() { return m_scene; }

private:
    // Core PhysX objects for physics simulation
    PxDefaultAllocator m_allocator;        // Memory allocation handler
    PxDefaultErrorCallback m_errorCallback; // Error reporting handler
    PxFoundation* m_foundation;            // PhysX foundation (base system)
    PxPhysics* m_physics;                  // Main PhysX physics SDK
    PxDefaultCpuDispatcher* m_dispatcher;  // CPU task dispatcher for threading
    PxScene* m_scene;                      // Physics simulation scene
    PxMaterial* m_material;                // Default surface material properties
    PxPvd* m_pvd;                         // PhysX Visual Debugger connection
    
    // Terrain collision management
    std::vector<PxRigidStatic*> m_terrainBodies; // Collection of terrain collision bodies

    /**
     * @brief Setup collision filtering system
     * 
     * Configures collision filtering to control which objects
     * can collide with each other. Useful for gameplay mechanics
     * like trigger volumes or different object layers.
     */
    void SetupFiltering();
};
