#ifndef EVENTS_H
#define EVENTS_H

namespace SockEngine {

enum class EventType {
    None,
    WindowResize, WindowClose,
    KeyPressed, KeyReleased, KeyRepeat,
    MouseButtonPressed, MouseButtonReleased,
    MouseMoved, MouseScrolled
};

class Event {
public:
    virtual ~Event() = default;
    virtual EventType GetType() const = 0;
};

// Window Events
class WindowResizeEvent : public Event {
public:
    WindowResizeEvent(unsigned int width, unsigned int height)
        : m_Width(width), m_Height(height) {}

    unsigned int GetWidth() const { return m_Width; }
    unsigned int GetHeight() const { return m_Height; }

    static EventType GetStaticType() { return EventType::WindowResize; }
    EventType GetType() const override { return GetStaticType(); }

private:
    unsigned int m_Width, m_Height;
};

class WindowCloseEvent : public Event {
public:
    WindowCloseEvent() {}

    static EventType GetStaticType() { return EventType::WindowClose; }
    EventType GetType() const override { return GetStaticType(); }
};

// Keyboard Events
class KeyEvent : public Event {
public:
    int GetKeyCode() const { return m_KeyCode; }
    
protected:
    KeyEvent(int keyCode) : m_KeyCode(keyCode) {}
    int m_KeyCode;
};

class KeyPressedEvent : public KeyEvent {
public:
    KeyPressedEvent(int keyCode) : KeyEvent(keyCode) {}
    
    static EventType GetStaticType() { return EventType::KeyPressed; }
    EventType GetType() const override { return GetStaticType(); }
};

class KeyReleasedEvent : public KeyEvent {
public:
    KeyReleasedEvent(int keyCode) : KeyEvent(keyCode) {}
    
    static EventType GetStaticType() { return EventType::KeyReleased; }
    EventType GetType() const override { return GetStaticType(); }
};

class KeyRepeatEvent : public KeyEvent {
public:
    KeyRepeatEvent(int keyCode) : KeyEvent(keyCode) {}
    
    static EventType GetStaticType() { return EventType::KeyRepeat; }
    EventType GetType() const override { return GetStaticType(); }
};

// Mouse Events
class MouseMovedEvent : public Event {
public:
    MouseMovedEvent(float x, float y, float deltaX = 0.0f, float deltaY = 0.0f)
        : m_MouseX(x), m_MouseY(y), m_DeltaX(deltaX), m_DeltaY(deltaY) {}

    float GetX() const { return m_MouseX; }
    float GetY() const { return m_MouseY; }
    float GetDeltaX() const { return m_DeltaX; }
    float GetDeltaY() const { return m_DeltaY; }

    static EventType GetStaticType() { return EventType::MouseMoved; }
    EventType GetType() const override { return GetStaticType(); }

private:
    float m_MouseX, m_MouseY;
    float m_DeltaX, m_DeltaY;
};

class MouseScrolledEvent : public Event {
public:
    MouseScrolledEvent(float xOffset, float yOffset)
        : m_XOffset(xOffset), m_YOffset(yOffset) {}

    float GetXOffset() const { return m_XOffset; }
    float GetYOffset() const { return m_YOffset; }

    static EventType GetStaticType() { return EventType::MouseScrolled; }
    EventType GetType() const override { return GetStaticType(); }

private:
    float m_XOffset, m_YOffset;
};

class MouseButtonEvent : public Event {
public:
    int GetMouseButton() const { return m_Button; }

protected:
    MouseButtonEvent(int button)
        : m_Button(button) {}

    int m_Button;
};

class MouseButtonPressedEvent : public MouseButtonEvent {
public:
    MouseButtonPressedEvent(int button)
        : MouseButtonEvent(button) {}

    static EventType GetStaticType() { return EventType::MouseButtonPressed; }
    EventType GetType() const override { return GetStaticType(); }
};

class MouseButtonReleasedEvent : public MouseButtonEvent {
public:
    MouseButtonReleasedEvent(int button)
        : MouseButtonEvent(button) {}

    static EventType GetStaticType() { return EventType::MouseButtonReleased; }
    EventType GetType() const override { return GetStaticType(); }
};

}

#endif