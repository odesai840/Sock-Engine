// Input.cpp
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
    // Skip first movement to avoid a large jump
    if (m_FirstMouse) {
        m_LastMousePosition = {x, y};
        m_MousePosition = {x, y};
        m_FirstMouse = false;
        return;
    }
    
    // Store last position before updating
    m_LastMousePosition = m_MousePosition;
    m_MousePosition = {x, y};
    
    // Calculate delta
    m_MouseDelta.x = m_MousePosition.x - m_LastMousePosition.x;
    m_MouseDelta.y = m_MousePosition.y - m_LastMousePosition.y;
}

void Input::UpdateMouseScroll(float xOffset, float yOffset) {
    m_MouseScroll = {xOffset, yOffset};
}

void Input::ResetDeltas()
{
    m_MouseDelta = {0.0f, 0.0f};
    m_MouseScroll = {0.0f, 0.0f};
}

}