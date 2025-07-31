#include "PhysicsCamera.h"
#include <algorithm>
#include <iostream>

PhysicsCamera::PhysicsCamera(Camera* camera, PhysicsManager* physics)
    : m_camera(camera)
    , m_physics(physics)
    , m_movementSpeed(2.5f)
    , m_jumpPower(8.0f)
    , m_gravity(9.81f)
    , m_gravityEnabled(true)
    , m_isOnGround(false)
    , m_velocity(0.0f)
    , m_cameraHeight(0.5f)  
{
    
    float groundHeight = m_physics->GetGroundHeight(m_camera->Position);
    m_camera->Position.y = groundHeight + m_cameraHeight;
}

PhysicsCamera::~PhysicsCamera() {
}

void PhysicsCamera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = m_movementSpeed * deltaTime;
    glm::vec3 desiredMovement(0.0f);
    
    
    glm::vec3 forward = glm::normalize(glm::vec3(m_camera->Front.x, 0.0f, m_camera->Front.z));
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    
    if (direction == FORWARD)
        desiredMovement += forward;
    if (direction == BACKWARD)
        desiredMovement -= forward;
    if (direction == LEFT)
        desiredMovement -= right;
    if (direction == RIGHT)
        desiredMovement += right;
    
    
    if (glm::length(desiredMovement) > 0.0f) {
        desiredMovement = glm::normalize(desiredMovement) * velocity;
        glm::vec3 newPosition = m_camera->Position + desiredMovement;
        
        
        glm::vec3 resolvedPosition = ResolveCollision(newPosition);
        m_camera->Position.x = resolvedPosition.x;
        m_camera->Position.z = resolvedPosition.z;
    }
}

void PhysicsCamera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    m_camera->ProcessMouseMovement(xoffset, yoffset, constrainPitch);
}

void PhysicsCamera::ProcessMouseScroll(float yoffset) {
    m_camera->ProcessMouseScroll(yoffset);
}

void PhysicsCamera::Update(float deltaTime) {
    if (m_gravityEnabled) {
        
        if (!m_isOnGround) {
            m_velocity.y -= m_gravity * deltaTime;
        } else {
            
            if (m_velocity.y < 0.0f) {
                m_velocity.y = 0.0f;
            }
        }
        
        
        float newY = m_camera->Position.y + m_velocity.y * deltaTime;
        
        
        glm::vec3 rayOrigin = m_camera->Position;
        rayOrigin.y += 1.0f; 
        glm::vec3 rayDirection(0.0f, -1.0f, 0.0f);
        glm::vec3 hitPoint;
        
        float groundHeight = -0.1f; 
        if (m_physics->Raycast(rayOrigin, rayDirection, 5.0f, hitPoint)) {
            groundHeight = hitPoint.y;
        }
        
        
        bool shouldBeOnGround = (newY <= groundHeight + m_cameraHeight + 0.1f);
        
        if (shouldBeOnGround && m_velocity.y <= 0.0f) {
            newY = groundHeight + m_cameraHeight;
            m_velocity.y = 0.0f;
            m_isOnGround = true;
        } else {
            m_isOnGround = false;
        }
        
        m_camera->Position.y = newY;
    }
}

void PhysicsCamera::Jump() {
    
    glm::vec3 rayOrigin = m_camera->Position;
    glm::vec3 rayDirection(0.0f, -1.0f, 0.0f);
    glm::vec3 hitPoint;
    
    
    bool canJump = m_physics->Raycast(rayOrigin, rayDirection, m_cameraHeight + 0.2f, hitPoint);
    
    if (canJump && m_gravityEnabled && m_velocity.y <= 1.0f) { 
        m_velocity.y = m_jumpPower;
        m_isOnGround = false;
        std::cout << "Jump executed! Height: " << m_camera->Position.y << std::endl;
    } else {
        std::cout << "Cannot jump - not on ground or already jumping" << std::endl;
    }
}

bool PhysicsCamera::CheckCollision(const glm::vec3& position) {
    
    return false;
}

glm::vec3 PhysicsCamera::ResolveCollision(const glm::vec3& desiredPosition) {
    
    glm::vec3 movement = desiredPosition - m_camera->Position;
    if (glm::length(movement) == 0.0f) {
        return desiredPosition;
    }
    
    glm::vec3 direction = glm::normalize(movement);
    glm::vec3 hitPoint;
    
    
    glm::vec3 rayOrigin = m_camera->Position;
    if (m_physics->Raycast(rayOrigin, direction, glm::length(movement) + 0.5f, hitPoint)) {
        
        float safeDistance = glm::distance(rayOrigin, hitPoint) - 0.5f;
        if (safeDistance > 0.0f) {
            return rayOrigin + direction * safeDistance;
        } else {
            return rayOrigin; 
        }
    }
    
    return desiredPosition; 
}

void PhysicsCamera::UpdateGroundCheck() {
    
    glm::vec3 rayOrigin = m_camera->Position;
    glm::vec3 rayDirection(0.0f, -1.0f, 0.0f);
    glm::vec3 hitPoint;
    
    if (m_physics->Raycast(rayOrigin, rayDirection, m_cameraHeight + 0.1f, hitPoint)) {
        float distanceToGround = rayOrigin.y - hitPoint.y;
        m_isOnGround = (distanceToGround <= m_cameraHeight + 0.05f);
    } else {
        m_isOnGround = false;
    }
}
