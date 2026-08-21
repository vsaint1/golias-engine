#include "core/input/input_manager.h"

#include <glfw/glfw3.h>

namespace golias {


    bool InputManager::IsKeyPressed(KeyCode key) const {
        if (static_cast<size_t>(key) < mKeyStates.size()) {
            return mKeyStates[static_cast<size_t>(key)];
        }

        return false;
    }

    bool InputManager::IsKeyJustPressed(KeyCode key) const {
        if (static_cast<size_t>(key) < mKeyJustPressed.size()) {
            return mKeyJustPressed[static_cast<size_t>(key)];
        }

        return false;
    }

    bool InputManager::IsKeyJustReleased(KeyCode key) const {
        if (static_cast<size_t>(key) < mKeyJustReleased.size()) {
            return mKeyJustReleased[static_cast<size_t>(key)];
        }

        return false;
    }

    void InputManager::SetKeyPressed(KeyCode key, bool pressed) {
        if (static_cast<size_t>(key) < mKeyStates.size()) {
            const size_t index      = static_cast<size_t>(key);
            const bool wasPressed   = mKeyStates[index];
            mKeyStates[index]       = pressed;
            mKeyJustPressed[index]  = mKeyJustPressed[index] || (pressed && !wasPressed);
            mKeyJustReleased[index] = mKeyJustReleased[index] || (!pressed && wasPressed);
        }
    }

    int InputManager::GetKeyCount() const {
        return static_cast<int>(KeyCode::Count);

    }
    
    KeyCode InputManager::TranslateKeyCode(int key) const {
        switch (key) {

            // Letters

        case GLFW_KEY_A:
            return KeyCode::A;
        case GLFW_KEY_B:
            return KeyCode::B;
        case GLFW_KEY_C:
            return KeyCode::C;
        case GLFW_KEY_D:
            return KeyCode::D;
        case GLFW_KEY_E:
            return KeyCode::E;
        case GLFW_KEY_F:
            return KeyCode::F;
        case GLFW_KEY_G:
            return KeyCode::G;
        case GLFW_KEY_H:
            return KeyCode::H;
        case GLFW_KEY_I:
            return KeyCode::I;
        case GLFW_KEY_J:
            return KeyCode::J;
        case GLFW_KEY_K:
            return KeyCode::K;
        case GLFW_KEY_L:
            return KeyCode::L;
        case GLFW_KEY_M:
            return KeyCode::M;
        case GLFW_KEY_N:
            return KeyCode::N;
        case GLFW_KEY_O:
            return KeyCode::O;
        case GLFW_KEY_P:
            return KeyCode::P;
        case GLFW_KEY_Q:
            return KeyCode::Q;
        case GLFW_KEY_R:
            return KeyCode::R;
        case GLFW_KEY_S:
            return KeyCode::S;
        case GLFW_KEY_T:
            return KeyCode::T;
        case GLFW_KEY_U:
            return KeyCode::U;
        case GLFW_KEY_V:
            return KeyCode::V;
        case GLFW_KEY_W:
            return KeyCode::W;
        case GLFW_KEY_X:
            return KeyCode::X;
        case GLFW_KEY_Y:
            return KeyCode::Y;
        case GLFW_KEY_Z:
            return KeyCode::Z;

            // Number row

        case GLFW_KEY_0:
            return KeyCode::Num0;
        case GLFW_KEY_1:
            return KeyCode::Num1;
        case GLFW_KEY_2:
            return KeyCode::Num2;
        case GLFW_KEY_3:
            return KeyCode::Num3;
        case GLFW_KEY_4:
            return KeyCode::Num4;
        case GLFW_KEY_5:
            return KeyCode::Num5;
        case GLFW_KEY_6:
            return KeyCode::Num6;
        case GLFW_KEY_7:
            return KeyCode::Num7;
        case GLFW_KEY_8:
            return KeyCode::Num8;
        case GLFW_KEY_9:
            return KeyCode::Num9;

            // Function keys

        case GLFW_KEY_F1:
            return KeyCode::F1;
        case GLFW_KEY_F2:
            return KeyCode::F2;
        case GLFW_KEY_F3:
            return KeyCode::F3;
        case GLFW_KEY_F4:
            return KeyCode::F4;
        case GLFW_KEY_F5:
            return KeyCode::F5;
        case GLFW_KEY_F6:
            return KeyCode::F6;
        case GLFW_KEY_F7:
            return KeyCode::F7;
        case GLFW_KEY_F8:
            return KeyCode::F8;
        case GLFW_KEY_F9:
            return KeyCode::F9;
        case GLFW_KEY_F10:
            return KeyCode::F10;
        case GLFW_KEY_F11:
            return KeyCode::F11;
        case GLFW_KEY_F12:
            return KeyCode::F12;
        case GLFW_KEY_F13:
            return KeyCode::F13;
        case GLFW_KEY_F14:
            return KeyCode::F14;
        case GLFW_KEY_F15:
            return KeyCode::F15;
        case GLFW_KEY_F16:
            return KeyCode::F16;
        case GLFW_KEY_F17:
            return KeyCode::F17;
        case GLFW_KEY_F18:
            return KeyCode::F18;
        case GLFW_KEY_F19:
            return KeyCode::F19;
        case GLFW_KEY_F20:
            return KeyCode::F20;
        case GLFW_KEY_F21:
            return KeyCode::F21;
        case GLFW_KEY_F22:
            return KeyCode::F22;
        case GLFW_KEY_F23:
            return KeyCode::F23;
        case GLFW_KEY_F24:
            return KeyCode::F24;
        case GLFW_KEY_F25:
            return KeyCode::F25;

            // Modifiers

        case GLFW_KEY_LEFT_SHIFT:
            return KeyCode::LeftShift;

        case GLFW_KEY_RIGHT_SHIFT:
            return KeyCode::RightShift;

        case GLFW_KEY_LEFT_CONTROL:
            return KeyCode::LeftControl;

        case GLFW_KEY_RIGHT_CONTROL:
            return KeyCode::RightControl;

        case GLFW_KEY_LEFT_ALT:
            return KeyCode::LeftAlt;

        case GLFW_KEY_RIGHT_ALT:
            return KeyCode::RightAlt;

        case GLFW_KEY_LEFT_SUPER:
            return KeyCode::LeftSuper;

        case GLFW_KEY_RIGHT_SUPER:
            return KeyCode::RightSuper;

            // Navigation / control

        case GLFW_KEY_ESCAPE:
            return KeyCode::Escape;

        case GLFW_KEY_ENTER:
            return KeyCode::Enter;

        case GLFW_KEY_TAB:
            return KeyCode::Tab;

        case GLFW_KEY_BACKSPACE:
            return KeyCode::Backspace;

        case GLFW_KEY_INSERT:
            return KeyCode::Insert;

        case GLFW_KEY_DELETE:
            return KeyCode::Delete;

        case GLFW_KEY_HOME:
            return KeyCode::Home;

        case GLFW_KEY_END:
            return KeyCode::End;

        case GLFW_KEY_PAGE_UP:
            return KeyCode::PageUp;

        case GLFW_KEY_PAGE_DOWN:
            return KeyCode::PageDown;

            // Arrows

        case GLFW_KEY_LEFT:
            return KeyCode::Left;

        case GLFW_KEY_RIGHT:
            return KeyCode::Right;

        case GLFW_KEY_UP:
            return KeyCode::Up;

        case GLFW_KEY_DOWN:
            return KeyCode::Down;

            // Lock / system

        case GLFW_KEY_CAPS_LOCK:
            return KeyCode::CapsLock;

        case GLFW_KEY_NUM_LOCK:
            return KeyCode::NumLock;

        case GLFW_KEY_SCROLL_LOCK:
            return KeyCode::ScrollLock;

        case GLFW_KEY_PRINT_SCREEN:
            return KeyCode::PrintScreen;

        case GLFW_KEY_PAUSE:
            return KeyCode::Pause;

        case GLFW_KEY_MENU:
            return KeyCode::Menu;

            // Punctuation

        case GLFW_KEY_SPACE:
            return KeyCode::Space;

        case GLFW_KEY_APOSTROPHE:
            return KeyCode::Apostrophe;

        case GLFW_KEY_COMMA:
            return KeyCode::Comma;

        case GLFW_KEY_MINUS:
            return KeyCode::Minus;

        case GLFW_KEY_PERIOD:
            return KeyCode::Period;

        case GLFW_KEY_SLASH:
            return KeyCode::Slash;

        case GLFW_KEY_SEMICOLON:
            return KeyCode::Semicolon;

        case GLFW_KEY_EQUAL:
            return KeyCode::Equal;

        case GLFW_KEY_LEFT_BRACKET:
            return KeyCode::LeftBracket;

        case GLFW_KEY_BACKSLASH:
            return KeyCode::Backslash;

        case GLFW_KEY_RIGHT_BRACKET:
            return KeyCode::RightBracket;

        case GLFW_KEY_GRAVE_ACCENT:
            return KeyCode::GraveAccent;

            // Keypad

        case GLFW_KEY_KP_0:
            return KeyCode::KP0;
        case GLFW_KEY_KP_1:
            return KeyCode::KP1;
        case GLFW_KEY_KP_2:
            return KeyCode::KP2;
        case GLFW_KEY_KP_3:
            return KeyCode::KP3;
        case GLFW_KEY_KP_4:
            return KeyCode::KP4;
        case GLFW_KEY_KP_5:
            return KeyCode::KP5;
        case GLFW_KEY_KP_6:
            return KeyCode::KP6;
        case GLFW_KEY_KP_7:
            return KeyCode::KP7;
        case GLFW_KEY_KP_8:
            return KeyCode::KP8;
        case GLFW_KEY_KP_9:
            return KeyCode::KP9;

        case GLFW_KEY_KP_DECIMAL:
            return KeyCode::KPDecimal;

        case GLFW_KEY_KP_DIVIDE:
            return KeyCode::KPDivide;

        case GLFW_KEY_KP_MULTIPLY:
            return KeyCode::KPMultiply;

        case GLFW_KEY_KP_SUBTRACT:
            return KeyCode::KPSubtract;

        case GLFW_KEY_KP_ADD:
            return KeyCode::KPAdd;

        case GLFW_KEY_KP_ENTER:
            return KeyCode::KPEnter;

        case GLFW_KEY_KP_EQUAL:
            return KeyCode::KPEqual;

            // International

        case GLFW_KEY_WORLD_1:
            return KeyCode::World1;

        case GLFW_KEY_WORLD_2:
            return KeyCode::World2;

        default:
            return KeyCode::Unknown;
        }
    }


    void InputManager::SetMouseButtonPressed(MouseButton button, bool pressed) {
        if (static_cast<size_t>(button) < mMouseButtonStates.size()) {
            const size_t index              = static_cast<size_t>(button);
            const bool wasPressed           = mMouseButtonStates[index];
            mMouseButtonStates[index]       = pressed;
            mMouseButtonJustPressed[index]  = mMouseButtonJustPressed[index] || (pressed && !wasPressed);
            mMouseButtonJustReleased[index] = mMouseButtonJustReleased[index] || (!pressed && wasPressed);
        }
    }

    bool InputManager::IsMouseButtonPressed(MouseButton button) const {
        if (static_cast<size_t>(button) < mMouseButtonStates.size()) {
            return mMouseButtonStates[static_cast<size_t>(button)];
        }

        return false;
    }

    bool InputManager::IsMouseButtonReleased(MouseButton button) const {
        return !IsMouseButtonPressed(button);
    }

    bool InputManager::IsMouseButtonJustPressed(MouseButton button) const {
        if (static_cast<size_t>(button) < mMouseButtonJustPressed.size()) {
            return mMouseButtonJustPressed[static_cast<size_t>(button)];
        }

        return false;
    }

    bool InputManager::IsMouseButtonJustReleased(MouseButton button) const {
        if (static_cast<size_t>(button) < mMouseButtonJustReleased.size()) {
            return mMouseButtonJustReleased[static_cast<size_t>(button)];
        }

        return false;
    }

    MouseButton InputManager::TranslateMouseButton(int button) const {
        switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            return MouseButton::Left;
        case GLFW_MOUSE_BUTTON_RIGHT:
            return MouseButton::Right;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            return MouseButton::Middle;
        case GLFW_MOUSE_BUTTON_4:
            return MouseButton::Button4;
        case GLFW_MOUSE_BUTTON_5:
            return MouseButton::Button5;
        case GLFW_MOUSE_BUTTON_6:
            return MouseButton::Button6;
        case GLFW_MOUSE_BUTTON_7:
            return MouseButton::Button7;
        case GLFW_MOUSE_BUTTON_8:
            return MouseButton::Button8;
        default:
            return MouseButton::Count;
        }
    }

    void InputManager::SetMousePosition(float x, float y) {
        const glm::vec2 position{x, y};
        mMouseDelta    = position - mMousePosition;
        mMousePosition = position;
    }

    glm::vec2 InputManager::GetMousePosition() const {
        return mMousePosition;
    }

    glm::vec2 InputManager::GetMouseDelta() const {
        return mMouseDelta;
    }

    void InputManager::SetScrollOffset(float x, float y) {
        mScrollOffset += glm::vec2{x, y};
    }

    glm::vec2 InputManager::GetScrollOffset() const {
        return mScrollOffset;
    }

    void InputManager::ResetTransientState() {
        mKeyJustPressed.fill(false);
        mKeyJustReleased.fill(false);
        mMouseButtonJustPressed.fill(false);
        mMouseButtonJustReleased.fill(false);
        mMouseDelta   = glm::vec2{0.0f};
        mScrollOffset = glm::vec2{0.0f};
    }

} // namespace golias
