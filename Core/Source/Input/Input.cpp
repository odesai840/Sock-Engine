#include "Input.h"

namespace SockEngine {

Input::Input() {
    // Initialize input system
}

void Input::UpdateKeyState(int key, int action) {
    if (key < 0 || key > GLFW_KEY_LAST) return;
    switch (action) {
    case GLFW_PRESS: keyStates[key] = PRESSED; break;
    case GLFW_RELEASE: keyStates[key] = RELEASED; break;
    case GLFW_REPEAT: keyStates[key] = HELD; break;
    default: keyStates[key] = NONE; break;
    }
}

bool Input::GetKeyPressed(int key) {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    if (keyStates[key] == PRESSED) {
        keyStates[key] = HELD;
        return true;
    }
    return false;
}

bool Input::GetKeyHeld(int key) {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return keyStates[key] == PRESSED || keyStates[key] == HELD;
}

bool Input::GetKeyReleased(int key) {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    if (keyStates[key] == RELEASED) {
        keyStates[key] = NONE;
        return true;
    }
    return false;
}

void Input::UpdateMouseButtonState(int button, int action) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return;
    switch (action) {
    case GLFW_PRESS: mouseButtonStates[button] = PRESSED; break;
    case GLFW_RELEASE: mouseButtonStates[button] = RELEASED; break;
    default: mouseButtonStates[button] = NONE; break;
    }
}

bool Input::GetMouseButtonPressed(int button) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    if (mouseButtonStates[button] == PRESSED) {
        mouseButtonStates[button] = HELD;
        return true;
    }
    return false;
}

bool Input::GetMouseButtonHeld(int button) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return mouseButtonStates[button] == PRESSED || mouseButtonStates[button] == HELD;
}

bool Input::GetMouseButtonReleased(int button) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    if (mouseButtonStates[button] == RELEASED) {
        mouseButtonStates[button] = NONE;
        return true;
    }
    return false;
}

void Input::UpdateMousePosition(float x, float y) {
    m_LastMousePosition = m_MousePosition;
    m_MousePosition = {x, y};
    
    if (m_FirstMouse) {
        m_MouseDelta = {0.0f, 0.0f};
        m_FirstMouse = false;
        return;
    }
    
    // Calculate delta
    m_MouseDelta = {
        m_MousePosition.x - m_LastMousePosition.x,
        m_MousePosition.y - m_LastMousePosition.y
    };
    
    // If mouse is captured, constrain it to viewport bounds
    if (m_MouseCaptured && m_CaptureWindow) {
        glm::vec2 center = (m_ViewportMin + m_ViewportMax) * 0.5f;
        glm::vec2 size = m_ViewportMax - m_ViewportMin;
        
        // Check if mouse is getting close to viewport edges
        bool needsRecenter = false;
        
        if (m_MousePosition.x <= m_ViewportMin.x + 10.0f || 
            m_MousePosition.x >= m_ViewportMax.x - 10.0f ||
            m_MousePosition.y <= m_ViewportMin.y + 10.0f || 
            m_MousePosition.y >= m_ViewportMax.y - 10.0f) {
            needsRecenter = true;
            }
        
        if (needsRecenter) {
            // Move cursor back to center of viewport
            glfwSetCursorPos(m_CaptureWindow, center.x, center.y);
            m_MousePosition = center;
            m_LastMousePosition = center;
        }
    }
}

void Input::UpdateMouseScroll(float xOffset, float yOffset) {
    m_MouseScroll = {xOffset, yOffset};
}

void Input::StartMouseCapture(GLFWwindow* window, const glm::vec2& viewportMin, const glm::vec2& viewportMax) {
    if (m_MouseCaptured) return;
    
    m_MouseCaptured = true;
    m_CaptureWindow = window;
    m_ViewportMin = viewportMin;
    m_ViewportMax = viewportMax;
    m_CaptureStartPosition = m_MousePosition;
    
    // Disable cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Reset delta to prevent initial jump
    m_MouseDelta = {0.0f, 0.0f};
}

void Input::EndMouseCapture(GLFWwindow* window) {
    if (!m_MouseCaptured) return;
    
    m_MouseCaptured = false;
    m_CaptureWindow = nullptr;
    
    // Show cursor again
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    
    // Restore cursor to original position
    glfwSetCursorPos(window, m_CaptureStartPosition.x, m_CaptureStartPosition.y);
    m_MousePosition = m_CaptureStartPosition;
    m_LastMousePosition = m_CaptureStartPosition;
    
    // Reset delta
    m_MouseDelta = {0.0f, 0.0f};
}

void Input::ResetDeltas()
{
    m_MouseDelta = {0.0f, 0.0f};
    m_MouseScroll = {0.0f, 0.0f};
}

}