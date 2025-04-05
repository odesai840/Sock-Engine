#include "Camera.h"

namespace SockEngine {

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

// returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(Position, Position + Front, Up);
}

// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD) {
        Position += Front * velocity;
    }
    if (direction == BACKWARD) {
        Position -= Front * velocity;
    }
    if (direction == LEFT) {
        Position -= Right * velocity;
    }
    if (direction == RIGHT) {
        Position += Right * velocity;
    }
}

// Apply smoothing to rotation values
void Camera::applySmoothing(float& yawOffset, float& pitchOffset) {
    // Initialize buffers if they're empty
    if (yawBuffer.size() == 0) {
        yawBuffer.assign(bufferSize, 0.0f);
        pitchBuffer.assign(bufferSize, 0.0f);
    }
    
    // Shift values in the buffer
    for (int i = bufferSize - 1; i > 0; i--) {
        yawBuffer[i] = yawBuffer[i-1];
        pitchBuffer[i] = pitchBuffer[i-1];
    }
    
    // Add new values
    yawBuffer[0] = yawOffset;
    pitchBuffer[0] = pitchOffset;
    
    // Compute smoothed values using weighted average
    float yawSum = 0.0f;
    float pitchSum = 0.0f;
    float weightSum = 0.0f;
    
    for (int i = 0; i < bufferSize; i++) {
        float weight = 1.0f / (i + 1); // More weight to recent values
        yawSum += yawBuffer[i] * weight;
        pitchSum += pitchBuffer[i] * weight;
        weightSum += weight;
    }
    
    // Update the offsets with smoothed values
    yawOffset = yawSum / weightSum;
    pitchOffset = pitchSum / weightSum;
}

// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;
    
    // Apply smoothing if enabled
    if (EnableSmoothing) {
        applySmoothing(xoffset, yoffset);
    }

    Yaw += xoffset;
    Pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (Pitch > 89.0f) {
            Pitch = 89.0f;
        }
        if (Pitch < -89.0f) {
            Pitch = -89.0f;
        }
    }

    // update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
}

// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void Camera::ProcessMouseScroll(float yoffset)
{
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f) {
        Zoom = 1.0f;
    }
    if (Zoom > 45.0f) {
        Zoom = 45.0f;
    }
}

void Camera::updateCameraVectors()
{
    // calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    // also re-calculate the Right and Up vector
    Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Up = glm::normalize(glm::cross(Right, Front));
}

}