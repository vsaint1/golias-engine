#include "core/input/input_manager.h"

#include "core/engine.h"

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

    void InputManager::SetMousePosition(float x, float y) {
        const glm::vec2 position{x, y};
        if (!mHasMouseBaseline) {
            mHasMouseBaseline = true;
        } else if (!mUIFocus) {
            mMouseDelta += position - mMousePosition;
        }

        mMousePosition = position;
    }

    void InputManager::SetMousePosition(const glm::vec2& position) {
        if (!mHasMouseBaseline) {
            mHasMouseBaseline = true;
        } else if (!mUIFocus) {
            mMouseDelta += position - mMousePosition;
        }

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

    bool InputManager::IsCanvasFocused() const {
        return mUIFocus;
    }

    void InputManager::SetCanvasFocus(bool focus) {
        mUIFocus = focus;

        Engine::GetInstance().SetInputMode(focus ? InputMode::Cursor : InputMode::Disabled);

        ResetMouseBaseline();
    }

    void InputManager::ResetMouseBaseline() {
        mHasMouseBaseline = false;
        mMouseDelta       = glm::vec2{0.0f};
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
