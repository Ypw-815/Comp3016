#pragma once

#include "Camera.h"
#include "PhysicsManager.h"
#include <glm/glm.hpp>

class PhysicsCamera {
public:
    PhysicsCamera(Camera* camera, PhysicsManager* physics);
    ~PhysicsCamera();

    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);
    
    void Update(float deltaTime);
    
    
    void SetMovementSpeed(float speed) { m_movementSpeed = speed; }
    void SetGravity(bool enabled) { m_gravityEnabled = enabled; }
    void SetJumpPower(float power) { m_jumpPower = power; }
    
    
    glm::vec3 GetPosition() const { return m_camera->Position; }
    
    
    void Jump();
    
    
    bool IsOnGround() const { return m_isOnGround; }

private:
    Camera* m_camera;
    PhysicsManager* m_physics;
    
    
    float m_movementSpeed;
    float m_jumpPower;
    float m_gravity;
    bool m_gravityEnabled;
    bool m_isOnGround;
    
    
    glm::vec3 m_velocity;
    
    
    float m_cameraHeight;
    
    
    bool CheckCollision(const glm::vec3& position);
    glm::vec3 ResolveCollision(const glm::vec3& desiredPosition);
    void UpdateGroundCheck();
};
