/**
 * @file Camera.h
 * @brief First-person camera system for 3D navigation
 * 
 * Provides a flexible first-person camera implementation with:
 * - WASD movement controls
 * - Mouse look functionality
 * - Configurable movement speed and mouse sensitivity
 * - View and projection matrix generation
 */

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

/**
 * @brief Camera movement directions
 * 
 * Defines abstract movement directions that can be mapped
 * to keyboard keys (typically WASD).
 */
enum Camera_Movement {
    FORWARD,    // Move forward relative to camera direction
    BACKWARD,   // Move backward relative to camera direction
    LEFT,       // Strafe left relative to camera direction
    RIGHT       // Strafe right relative to camera direction
};

// Default camera configuration constants
const float YAW = -90.0f;        // Default horizontal rotation (facing negative Z)
const float PITCH = 0.0f;        // Default vertical rotation (level horizon)
const float SPEED = 2.5f;        // Default movement speed (units per second)
const float SENSITIVITY = 0.2f;  // Mouse sensitivity for look around
const float ZOOM = 45.0f;        // Default field of view (degrees)

/**
 * @brief First-person camera class
 * 
 * Implements a first-person camera system suitable for 3D games and applications.
 * Supports keyboard movement, mouse look, and automatic view matrix calculation.
 * 
 * Features:
 * - Euler angle-based orientation (yaw and pitch)
 * - Smooth movement with delta-time integration
 * - Configurable speed and sensitivity
 * - View frustum controls (zoom/FOV)
 */
class Camera
{
public:
    // Camera coordinate system vectors
    glm::vec3 Position;  // Camera world position
    glm::vec3 Front;     // Forward direction vector
    glm::vec3 Up;        // Up direction vector
    glm::vec3 Right;     // Right direction vector
    glm::vec3 WorldUp;   // World up vector (usually +Y)

    // Euler angle rotations
    float Yaw;    // Horizontal rotation around Y-axis
    float Pitch;  // Vertical rotation around X-axis

    // Camera movement and control parameters
    float MovementSpeed;    // Movement speed in units per second
    float MouseSensitivity; // Mouse sensitivity multiplier
    float Zoom;            // Field of view in degrees

    /**
     * @brief Constructor with position and orientation vectors
     * 
     * @param position Initial camera world position
     * @param up World up vector (default: +Y axis)
     * @param yaw Initial horizontal rotation in degrees
     * @param pitch Initial vertical rotation in degrees
     */
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
           float yaw = YAW, 
           float pitch = PITCH);

    /**
     * @brief Constructor with individual position components
     * 
     * @param posX Initial X position
     * @param posY Initial Y position  
     * @param posZ Initial Z position
     * @param upX World up X component
     * @param upY World up Y component
     * @param upZ World up Z component
     * @param yaw Initial horizontal rotation
     * @param pitch Initial vertical rotation
     */
    Camera(float posX, float posY, float posZ, 
           float upX, float upY, float upZ, 
           float yaw, float pitch);

    /**
     * @brief Get the view matrix for rendering
     * @return 4x4 view transformation matrix
     */
    glm::mat4 GetViewMatrix();

    /**
     * @brief Process keyboard input for camera movement
     * @param direction Movement direction (WASD)
     * @param deltaTime Time elapsed since last frame
     */
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);

    /**
     * @brief Process mouse movement for camera rotation
     * @param xoffset Mouse X movement
     * @param yoffset Mouse Y movement
     * @param constrainPitch Whether to limit vertical rotation
     */
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

    /**
     * @brief Process mouse scroll for zoom control
     * @param yoffset Scroll wheel movement
     */
    void ProcessMouseScroll(float yoffset);

private:
    /**
     * @brief Recalculate camera direction vectors from Euler angles
     * 
     * Updates the Front, Right, and Up vectors based on the current
     * Yaw and Pitch values. This method is called automatically whenever
     * the camera orientation changes through mouse input.
     * 
     * The calculation uses spherical coordinates to convert Euler angles
     * to Cartesian direction vectors, ensuring all vectors remain
     * normalized and orthogonal.
     */
    void UpdateCameraVectors();
}; 