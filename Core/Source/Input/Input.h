#ifndef INPUT_H
#define INPUT_H

#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace SockEngine {

enum State {
    NONE,
    PRESSED,
    HELD,
    RELEASED
};

class Input {
public:
    Input();

    // Keyboard methods
    void UpdateKeyState(int key, int action);
    bool GetKeyPressed(int key);
    bool GetKeyHeld(int key);
    bool GetKeyReleased(int key);

    // Mouse methods
    void UpdateMouseButtonState(int button, int action);
    bool GetMouseButtonPressed(int button);
    bool GetMouseButtonHeld(int button);
    bool GetMouseButtonReleased(int button);
    
    void UpdateMousePosition(float x, float y);
    glm::vec2 GetMousePosition() const { return m_MousePosition; }
    glm::vec2 GetMouseDelta() const { return m_MouseDelta; }
    
    void UpdateMouseScroll(float xOffset, float yOffset);
    glm::vec2 GetMouseScroll() const { return m_MouseScroll; }

    void ResetDeltas();

private:
    std::vector<State> keyStates = std::vector<State>(GLFW_KEY_LAST + 1, NONE);
    std::vector<State> mouseButtonStates = std::vector<State>(GLFW_MOUSE_BUTTON_LAST + 1, NONE);
    
    glm::vec2 m_MousePosition = {0.0f, 0.0f};
    glm::vec2 m_LastMousePosition = {0.0f, 0.0f};
    glm::vec2 m_MouseDelta = {0.0f, 0.0f};
    glm::vec2 m_MouseScroll = {0.0f, 0.0f};

    bool m_FirstMouse = true;
};

}

#endif