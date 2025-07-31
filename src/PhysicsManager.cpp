#include "PhysicsManager.h"
#include <iostream>
#include <algorithm>

PhysicsManager::PhysicsManager()
    : m_foundation(nullptr)
    , m_physics(nullptr)
    , m_dispatcher(nullptr)
    , m_scene(nullptr)
    , m_material(nullptr)
    , m_pvd(nullptr)
{
}

PhysicsManager::~PhysicsManager() {
    Cleanup();
}

bool PhysicsManager::Initialize() {
    
    m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_allocator, m_errorCallback);
    if (!m_foundation) {
        std::cerr << "PxCreateFoundation failed!" << std::endl;
        return false;
    }

    
    m_pvd = PxCreatePvd(*m_foundation);
    if (m_pvd) {
        PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
        if (transport) {
            m_pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
        }
    }

    
    m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, PxTolerancesScale(), true, m_pvd);
    if (!m_physics) {
        std::cerr << "PxCreatePhysics failed!" << std::endl;
        return false;
    }

    
    m_dispatcher = PxDefaultCpuDispatcherCreate(2);
    if (!m_dispatcher) {
        std::cerr << "PxDefaultCpuDispatcherCreate failed!" << std::endl;
        return false;
    }

    
    PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    sceneDesc.cpuDispatcher = m_dispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;

    m_scene = m_physics->createScene(sceneDesc);
    if (!m_scene) {
        std::cerr << "createScene failed!" << std::endl;
        return false;
    }

    
    m_material = m_physics->createMaterial(0.5f, 0.5f, 0.6f);

    
    CreateGroundPlane();

    std::cout << "PhysX Physics System Initialized!" << std::endl;
    std::cout << "Gravity: -9.81 m/s^2" << std::endl;
    std::cout << "Ground collision enabled" << std::endl;
    std::cout << "Realistic movement constraints" << std::endl;

    return true;
}

void PhysicsManager::Update(float deltaTime) {
    if (m_scene) {
        m_scene->simulate(deltaTime);
        m_scene->fetchResults(true);
    }
}

void PhysicsManager::Cleanup() {
    if (m_scene) {
        m_scene->release();
        m_scene = nullptr;
    }
    if (m_dispatcher) {
        m_dispatcher->release();
        m_dispatcher = nullptr;
    }
    if (m_physics) {
        m_physics->release();
        m_physics = nullptr;
    }
    if (m_pvd) {
        if (m_pvd->isConnected()) {
            m_pvd->getTransport()->release();
        }
        m_pvd->release();
        m_pvd = nullptr;
    }
    if (m_foundation) {
        m_foundation->release();
        m_foundation = nullptr;
    }
}

PxRigidStatic* PhysicsManager::CreateStaticBody(const glm::vec3& position, const glm::vec3& size) {
    PxTransform transform(PxVec3(position.x, position.y, position.z));
    PxRigidStatic* staticActor = m_physics->createRigidStatic(transform);
    
    PxShape* shape = PxRigidActorExt::createExclusiveShape(*staticActor,
        PxBoxGeometry(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f), *m_material);
    
    m_scene->addActor(*staticActor);
    return staticActor;
}

PxRigidDynamic* PhysicsManager::CreateDynamicBody(const glm::vec3& position, const glm::vec3& size, float mass) {
    PxTransform transform(PxVec3(position.x, position.y, position.z));
    PxRigidDynamic* dynamicActor = m_physics->createRigidDynamic(transform);
    
    PxShape* shape = PxRigidActorExt::createExclusiveShape(*dynamicActor,
        PxBoxGeometry(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f), *m_material);
    
    PxRigidBodyExt::updateMassAndInertia(*dynamicActor, mass);
    
    m_scene->addActor(*dynamicActor);
    return dynamicActor;
}

PxRigidDynamic* PhysicsManager::CreateDynamicActor(const glm::vec3& position, const PxGeometry& geometry, float mass) {
    PxTransform transform(PxVec3(position.x, position.y, position.z));
    PxRigidDynamic* dynamicActor = m_physics->createRigidDynamic(transform);
    
    PxShape* shape = PxRigidActorExt::createExclusiveShape(*dynamicActor, geometry, *m_material);
    
    PxRigidBodyExt::updateMassAndInertia(*dynamicActor, mass);
    
    m_scene->addActor(*dynamicActor);
    return dynamicActor;
}

void PhysicsManager::CreateGroundPlane(float y) {
    PxTransform transform(PxVec3(0.0f, y, 0.0f));
    PxRigidStatic* groundPlane = m_physics->createRigidStatic(transform);
    
    
    PxShape* shape = PxRigidActorExt::createExclusiveShape(*groundPlane,
        PxBoxGeometry(100.0f, 0.1f, 100.0f), *m_material);
    
    m_scene->addActor(*groundPlane);
}

bool PhysicsManager::Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, glm::vec3& hitPoint) {
    PxVec3 pxOrigin(origin.x, origin.y, origin.z);
    PxVec3 pxDirection(direction.x, direction.y, direction.z);
    
    PxRaycastBuffer hit;
    bool hasHit = m_scene->raycast(pxOrigin, pxDirection, maxDistance, hit);
    
    if (hasHit && hit.hasBlock) {
        hitPoint.x = hit.block.position.x;
        hitPoint.y = hit.block.position.y;
        hitPoint.z = hit.block.position.z;
        return true;
    }
    
    return false;
}

float PhysicsManager::GetGroundHeight(const glm::vec3& position) {
    glm::vec3 rayOrigin(position.x, position.y + 10.0f, position.z);
    glm::vec3 rayDirection(0.0f, -1.0f, 0.0f);
    glm::vec3 hitPoint;
    
    if (Raycast(rayOrigin, rayDirection, 20.0f, hitPoint)) {
        return hitPoint.y;
    }
    
    return -0.1f; 
}

void PhysicsManager::CreateTerrainCollision(const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices) {
    if (vertices.empty() || indices.empty()) {
        std::cout << "Warning: Empty terrain data provided for collision creation" << std::endl;
        return;
    }

    
    std::vector<PxVec3> pxVertices;
    pxVertices.reserve(vertices.size());
    for (const auto& vertex : vertices) {
        pxVertices.emplace_back(vertex.x, vertex.y, vertex.z);
    }

    
    PxTriangleMeshDesc meshDesc;
    meshDesc.points.count = static_cast<PxU32>(pxVertices.size());
    meshDesc.points.stride = sizeof(PxVec3);
    meshDesc.points.data = pxVertices.data();

    meshDesc.triangles.count = static_cast<PxU32>(indices.size() / 3);
    meshDesc.triangles.stride = 3 * sizeof(PxU32);
    meshDesc.triangles.data = indices.data();

    
    PxDefaultMemoryOutputStream writeBuffer;
    PxTriangleMeshCookingResult::Enum result;
    PxCookingParams cookingParams(m_physics->getTolerancesScale());
    
    bool status = PxCookTriangleMesh(cookingParams, meshDesc, writeBuffer, &result);
    if (!status) {
        std::cout << "Error: Failed to cook terrain triangle mesh" << std::endl;
        return;
    }

    PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
    PxTriangleMesh* triangleMesh = m_physics->createTriangleMesh(readBuffer);

    if (!triangleMesh) {
        std::cout << "Error: Failed to create terrain triangle mesh" << std::endl;
        return;
    }

    
    PxTransform transform(PxVec3(0.0f, 0.0f, 0.0f));
    PxRigidStatic* terrainActor = m_physics->createRigidStatic(transform);
    
    PxTriangleMeshGeometry triGeom(triangleMesh);
    PxShape* terrainShape = PxRigidActorExt::createExclusiveShape(*terrainActor, triGeom, *m_material);
    
    m_scene->addActor(*terrainActor);
    
    
    m_terrainBodies.push_back(terrainActor);
    
    std::cout << "Terrain collision mesh created with " << vertices.size() << " vertices and " 
              << (indices.size() / 3) << " triangles" << std::endl;
}

void PhysicsManager::ClearTerrainCollision() {
    
    for (auto* body : m_terrainBodies) {
        if (body) {
            m_scene->removeActor(*body);
            body->release();
        }
    }
    m_terrainBodies.clear();
    std::cout << "Cleared all terrain collision bodies" << std::endl;
}
