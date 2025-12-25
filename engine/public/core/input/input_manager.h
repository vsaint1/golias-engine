#pragma once
#include <SDL3/SDL_events.h>
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

namespace golias {

    class InputManager {
    public:
        ~InputManager() = default;

        // Keyboard
        bool IsKeyPressed(int keycode) const;
        bool IsKeyJustPressed(int keycode) const;
        bool IsKeyJustReleased(int keycode) const;

        // Mouse
        bool IsMouseButtonPressed(int button) const;
        bool IsMouseButtonJustPressed(int button) const;
        bool IsMouseButtonJustReleased(int button) const;
        glm::vec2 GetMousePosition() const;
        glm::vec2 GetMouseDelta() const;

        // Controller
        bool IsControllerButtonPressed(int button) const;
        bool IsControllerButtonJustPressed(int button) const;
        bool IsControllerButtonJustReleased(int button) const;
        float GetControllerAxis(int axis) const;

        // Actions (can bind keyboard, mouse, or controller)
        void BindAction(const std::string& action, int keycode);
        void BindActionMouse(const std::string& action, int button);
        void BindActionController(const std::string& action, int button);
        bool IsActionPressed(const std::string& action) const;
        bool IsActionJustPressed(const std::string& action) const;
        bool IsActionJustReleased(const std::string& action) const;

        void Update();
        void ProcessEvent(const SDL_Event& event);
        void Reset();

    private:
        InputManager()                               = default;
        InputManager(const InputManager&)            = delete;
        InputManager& operator=(const InputManager&) = delete;

        enum class KeyState : uint8_t { UP = 0, JUST_PRESSED = 1, PRESSED = 2, JUST_RELEASED = 3 };

        enum class BindingType : uint8_t { KEYBOARD, MOUSE, CONTROLLER };

        struct Binding {
            BindingType type;
            int code;
        };

        // Keyboard
        static constexpr size_t MAX_KEYS = 512;
        std::array<KeyState, MAX_KEYS> key_states_{};

        // Mouse
        static constexpr size_t MAX_MOUSE_BUTTONS = 8;
        std::array<KeyState, MAX_MOUSE_BUTTONS> mouse_states_{};
        glm::vec2 mouse_pos_{0.0f};
        glm::vec2 mouse_delta_{0.0f};

        SDL_Gamepad* gamepad_ = nullptr;
        std::array<KeyState, SDL_GAMEPAD_BUTTON_COUNT> controller_states_{};
        std::array<float, SDL_GAMEPAD_AXIS_COUNT> controller_axes_{};

        std::unordered_map<std::string, std::vector<Binding>> actions_;

        bool IsValidKeycode(int k) const {
            return k >= 0 && k < MAX_KEYS;
        }
        
        bool IsValidMouseButton(int b) const {
            return b >= 0 && b < MAX_MOUSE_BUTTONS;
        }
        bool IsValidControllerButton(int b) const {
            return b >= 0 && b < SDL_GAMEPAD_BUTTON_COUNT;
        }

        KeyState GetKeyState(int keycode) const;
        KeyState GetMouseState(int button) const;
        KeyState GetControllerState(int button) const;
        bool CheckBinding(const Binding& b) const;
        bool CheckBindingJustPressed(const Binding& b) const;
        bool CheckBindingJustReleased(const Binding& b) const;

        friend class Engine;
    };

} // namespace golias
