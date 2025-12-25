#include "core/input/input_manager.h"

#include <algorithm>
#include <cmath>

namespace golias {

    bool InputManager::IsKeyPressed(int keycode) const {
        auto state = GetKeyState(keycode);
        return state == KeyState::PRESSED || state == KeyState::JUST_PRESSED;
    }

    bool InputManager::IsKeyJustPressed(int keycode) const {
        return GetKeyState(keycode) == KeyState::JUST_PRESSED;
    }

    bool InputManager::IsKeyJustReleased(int keycode) const {
        return GetKeyState(keycode) == KeyState::JUST_RELEASED;
    }

    InputManager::KeyState InputManager::GetKeyState(int keycode) const {
        if (!IsValidKeycode(keycode)) {
            return KeyState::UP;
        }
        return key_states_[keycode];
    }

    bool InputManager::IsMouseButtonPressed(int button) const {
        auto state = GetMouseState(button);
        return state == KeyState::PRESSED || state == KeyState::JUST_PRESSED;
    }

    bool InputManager::IsMouseButtonJustPressed(int button) const {
        return GetMouseState(button) == KeyState::JUST_PRESSED;
    }

    bool InputManager::IsMouseButtonJustReleased(int button) const {
        return GetMouseState(button) == KeyState::JUST_RELEASED;
    }

    glm::vec2 InputManager::GetMousePosition() const {
        return mouse_pos_;
    }

    glm::vec2 InputManager::GetMouseDelta() const {
        return mouse_delta_;
    }

    InputManager::KeyState InputManager::GetMouseState(int button) const {
        if (!IsValidMouseButton(button)) {
            return KeyState::UP;
        }
        return mouse_states_[button];
    }

    bool InputManager::IsControllerButtonPressed(int button) const {
        auto state = GetControllerState(button);
        return state == KeyState::PRESSED || state == KeyState::JUST_PRESSED;
    }

    bool InputManager::IsControllerButtonJustPressed(int button) const {
        return GetControllerState(button) == KeyState::JUST_PRESSED;
    }

    bool InputManager::IsControllerButtonJustReleased(int button) const {
        return GetControllerState(button) == KeyState::JUST_RELEASED;
    }

    float InputManager::GetControllerAxis(int axis) const {
        if (!gamepad_ || axis < 0 || axis >= SDL_GAMEPAD_AXIS_COUNT) {
            return 0.0f;
        }
        return controller_axes_[axis];
    }

    InputManager::KeyState InputManager::GetControllerState(int button) const {
        if (!gamepad_ || !IsValidControllerButton(button)) {
            return KeyState::UP;
        }
        return controller_states_[button];
    }

    void InputManager::BindAction(const std::string& action, int keycode) {
        actions_[action].push_back({BindingType::KEYBOARD, keycode});
    }

    void InputManager::BindActionMouse(const std::string& action, int button) {
        actions_[action].push_back({BindingType::MOUSE, button});
    }

    void InputManager::BindActionController(const std::string& action, int button) {
        actions_[action].push_back({BindingType::CONTROLLER, button});
    }

    bool InputManager::IsActionPressed(const std::string& action) const {
        auto it = actions_.find(action);
        if (it == actions_.end()) {
            return false;
        }

        for (const auto& binding : it->second) {
            if (CheckBinding(binding)) {
                return true;
            }
        }
        return false;
    }

    bool InputManager::IsActionJustPressed(const std::string& action) const {
        auto it = actions_.find(action);
        if (it == actions_.end()) {
            return false;
        }

        for (const auto& binding : it->second) {
            if (CheckBindingJustPressed(binding)) {
                return true;
            }
        }
        return false;
    }

    bool InputManager::IsActionJustReleased(const std::string& action) const {
        auto it = actions_.find(action);
        if (it == actions_.end()) {
            return false;
        }

        for (const auto& binding : it->second) {
            if (CheckBindingJustReleased(binding)) {
                return true;
            }
        }
        return false;
    }

    bool InputManager::CheckBinding(const Binding& b) const {
        switch (b.type) {
        case BindingType::KEYBOARD:
            return IsKeyPressed(b.code);
        case BindingType::MOUSE:
            return IsMouseButtonPressed(b.code);
        case BindingType::CONTROLLER:
            return IsControllerButtonPressed(b.code);
        }
        return false;
    }

    bool InputManager::CheckBindingJustPressed(const Binding& b) const {
        switch (b.type) {
        case BindingType::KEYBOARD:
            return IsKeyJustPressed(b.code);
        case BindingType::MOUSE:
            return IsMouseButtonJustPressed(b.code);
        case BindingType::CONTROLLER:
            return IsControllerButtonJustPressed(b.code);
        }
        return false;
    }

    bool InputManager::CheckBindingJustReleased(const Binding& b) const {
        switch (b.type) {
        case BindingType::KEYBOARD:
            return IsKeyJustReleased(b.code);
        case BindingType::MOUSE:
            return IsMouseButtonJustReleased(b.code);
        case BindingType::CONTROLLER:
            return IsControllerButtonJustReleased(b.code);
        }
        return false;
    }

    // Update
    void InputManager::Update() {
        // Update keyboard
        for (auto& state : key_states_) {
            if (state == KeyState::JUST_PRESSED) {
                state = KeyState::PRESSED;
            } else if (state == KeyState::JUST_RELEASED) {
                state = KeyState::UP;
            }
        }

        for (auto& state : mouse_states_) {
            if (state == KeyState::JUST_PRESSED) {
                state = KeyState::PRESSED;
            } else if (state == KeyState::JUST_RELEASED) {
                state = KeyState::UP;
            }
        }
        mouse_delta_ = glm::vec2(0.0f);

        if (gamepad_) {
            for (auto& state : controller_states_) {
                if (state == KeyState::JUST_PRESSED) {
                    state = KeyState::PRESSED;
                } else if (state == KeyState::JUST_RELEASED) {
                    state = KeyState::UP;
                }
            }
        }
    }

    void InputManager::ProcessEvent(const SDL_Event& event) {
        switch (event.type) {
        case SDL_EVENT_KEY_DOWN:
            {
                int k = static_cast<int>(event.key.key);
                if (IsValidKeycode(k) && key_states_[k] == KeyState::UP) {
                    key_states_[k] = KeyState::JUST_PRESSED;
                }
                break;
            }
        case SDL_EVENT_KEY_UP:
            {
                int k = static_cast<int>(event.key.key);
                if (IsValidKeycode(k)) {
                    key_states_[k] = KeyState::JUST_RELEASED;
                }
                break;
            }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                int b = event.button.button;
                if (IsValidMouseButton(b) && mouse_states_[b] == KeyState::UP) {
                    mouse_states_[b] = KeyState::JUST_PRESSED;
                }
                break;
            }
        case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                int b = event.button.button;
                if (IsValidMouseButton(b)) {
                    mouse_states_[b] = KeyState::JUST_RELEASED;
                }
                break;
            }
        case SDL_EVENT_MOUSE_MOTION:
            {
                mouse_pos_   = glm::vec2(event.motion.x, event.motion.y);
                mouse_delta_ = glm::vec2(event.motion.xrel, event.motion.yrel);
                break;
            }
        case SDL_EVENT_GAMEPAD_ADDED:
            {
                if (!gamepad_) {
                    gamepad_ = SDL_OpenGamepad(event.gdevice.which);
                }
                break;
            }
        case SDL_EVENT_GAMEPAD_REMOVED:
            {
                if (gamepad_ && SDL_GetGamepadID(gamepad_) == event.gdevice.which) {
                    SDL_CloseGamepad(gamepad_);
                    gamepad_ = nullptr;
                    controller_states_.fill(KeyState::UP);
                    controller_axes_.fill(0.0f);
                }
                break;
            }
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            {
                if (gamepad_ && SDL_GetGamepadID(gamepad_) == event.gbutton.which) {
                    int b = event.gbutton.button;
                    if (IsValidControllerButton(b) && controller_states_[b] == KeyState::UP) {
                        controller_states_[b] = KeyState::JUST_PRESSED;
                    }
                }
                break;
            }
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            {
                if (gamepad_ && SDL_GetGamepadID(gamepad_) == event.gbutton.which) {
                    int b = event.gbutton.button;
                    if (IsValidControllerButton(b)) {
                        controller_states_[b] = KeyState::JUST_RELEASED;
                    }
                }
                break;
            }
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            {
                if (gamepad_ && SDL_GetGamepadID(gamepad_) == event.gaxis.which) {
                    int a = event.gaxis.axis;
                    if (a >= 0 && a < SDL_GAMEPAD_AXIS_COUNT) {
                        controller_axes_[a] = event.gaxis.value / 32767.0f;
                    }
                }
                break;
            }
        }
    }

    void InputManager::Reset() {
        key_states_.fill(KeyState::UP);
        mouse_states_.fill(KeyState::UP);
        mouse_pos_   = glm::vec2(0.0f);
        mouse_delta_ = glm::vec2(0.0f);

        if (gamepad_) {
            SDL_CloseGamepad(gamepad_);
            gamepad_ = nullptr;
        }
        controller_states_.fill(KeyState::UP);
        controller_axes_.fill(0.0f);

        actions_.clear();
    }

} // namespace golias
