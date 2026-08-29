#pragma once

#include "input.h"
#include <glm/vec2.hpp>
#include <array>

namespace golias {


    class InputManager {

    public:
        void SetKeyPressed(KeyCode key, bool pressed);

        bool IsKeyPressed(KeyCode key) const;

        bool IsKeyJustPressed(KeyCode key) const;

        bool IsKeyJustReleased(KeyCode key) const;

        int GetKeyCount() const;

        void SetMouseButtonPressed(MouseButton button, bool pressed);

        bool IsMouseButtonPressed(MouseButton button) const;

        bool IsMouseButtonReleased(MouseButton button) const;

        bool IsMouseButtonJustPressed(MouseButton button) const;

        bool IsMouseButtonJustReleased(MouseButton button) const;

        void SetMousePosition(float x, float y);
        void SetMousePosition(const glm::vec2& position);

        glm::vec2 GetMousePosition() const;

        glm::vec2 GetMouseDelta() const;

        void SetScrollOffset(float x, float y);

        glm::vec2 GetScrollOffset() const;

        void ResetTransientState();


    private:
        InputManager()                               = default;
        InputManager(const InputManager&)            = delete;
        InputManager& operator=(const InputManager&) = delete;
        InputManager(InputManager&&)                 = delete;
        InputManager& operator=(InputManager&&)      = delete;

    private:
        std::array<bool, static_cast<size_t>(KeyCode::Count)> mKeyStates                   = {};
        std::array<bool, static_cast<size_t>(KeyCode::Count)> mKeyJustPressed              = {};
        std::array<bool, static_cast<size_t>(KeyCode::Count)> mKeyJustReleased             = {};
        std::array<bool, static_cast<size_t>(MouseButton::Count)> mMouseButtonStates       = {};
        std::array<bool, static_cast<size_t>(MouseButton::Count)> mMouseButtonJustPressed  = {};
        std::array<bool, static_cast<size_t>(MouseButton::Count)> mMouseButtonJustReleased = {};

        glm::vec2 mMousePosition = glm::vec2(0.0f, 0.0f);
        glm::vec2 mMouseDelta    = glm::vec2(0.0f, 0.0f);
        glm::vec2 mScrollOffset  = glm::vec2(0.0f, 0.0f);

        friend class Engine;
    };
} // namespace golias
