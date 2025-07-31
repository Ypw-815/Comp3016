/**
 * @file Camera.cpp
 * @brief Implementation of the first-person camera system
 */

#include "Camera.h"

/**
 * @brief Constructor with position and up vector
 * 
 * Initializes camera with specified position and orientation.
 * Automatically calculates camera coordinate system vectors.
 */
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) 
    : Front(glm::vec3(0.0f, 0.0f, -1.0f))  // Default forward direction
    , MovementSpeed(SPEED)                  // Default movement speed
    , MouseSensitivity(SENSITIVITY)         // Default mouse sensitivity
    , Zoom(ZOOM)                           // Default field of view
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    UpdateCameraVectors();  // Calculate initial camera vectors
}

/**
 * @brief Constructor with individual components
 * 
 * Alternative constructor accepting individual float values
 * for position and up vector components.
 */
Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) 
    : Front(glm::vec3(0.0f, 0.0f, -1.0f))
    , MovementSpeed(SPEED)
    , MouseSensitivity(SENSITIVITY)
    , Zoom(ZOOM)
{
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    UpdateCameraVectors();
}

/**
 * @brief Generate view matrix for rendering
 * 
 * Creates a view matrix using the camera's current position and orientation.
 * This matrix transforms world coordinates to camera/view coordinates.
 * 
 * @return 4x4 view transformation matrix
 */
glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
}

/**
 * @brief Process keyboard input for camera movement
 * 
 * Moves the camera based on WASD-style input. Movement is relative
 * to the camera's current orientation and scaled by delta time.
 * 
 * @param direction Movement direction (FORWARD, BACKWARD, LEFT, RIGHT)
 * @param deltaTime Time elapsed since last frame (for frame-rate independence)
 */
void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;    // Move forward along camera's front vector
    if (direction == BACKWARD)
        Position -= Front * velocity;    // Move backward opposite to front vector
    if (direction == LEFT)
        Position -= Right * velocity;    // Strafe left opposite to right vector
    if (direction == RIGHT)
        Position += Right * velocity;    // Strafe right along right vector
}

/**
 * @brief Process mouse movement for camera look-around
 * 
 * Updates camera orientation based on mouse movement offsets.
 * Implements pitch constraining to prevent camera flipping.
 * 
 * @param xoffset Mouse X-axis movement offset
 * @param yoffset Mouse Y-axis movement offset  
 * @param constrainPitch Whether to limit vertical rotation (prevents flipping)
 */
void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // Prevent camera from flipping over
    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // Update camera vectors based on new rotation
    UpdateCameraVectors();
}

/**
 * @brief Process mouse scroll wheel for zoom control
 * 
 * Adjusts camera zoom (field of view) based on scroll input.
 * Zoom is clamped between 1 and 45 degrees.
 * 
 * @param yoffset Scroll wheel movement offset
 */
void Camera::ProcessMouseScroll(float yoffset) {
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}

/**
 * @brief Recalculate camera direction vectors from Euler angles
 * 
 * Updates Front, Right, and Up vectors based on current Yaw and Pitch.
 * This creates an orthogonal coordinate system for the camera.
 * 
 * The calculation:
 * 1. Converts Euler angles to direction vector using spherical coordinates
 * 2. Calculates right vector via cross product with world up
 * 3. Calculates camera up vector via cross product of right and front
 */
void Camera::UpdateCameraVectors() {
    // Calculate new Front vector from Euler angles
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    
    // Calculate Right and Up vectors
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}