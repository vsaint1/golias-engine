#include "core/input/input_manager.h"

#include <cmath>

namespace golias {

    InputManager::KeyState InputManager::GetKeyState(int keycode) const {
        return IsValidKeycode(keycode) ? key_states_[keycode] : KeyState::UP;
    }

    void InputManager::Update() {
        for (auto& state : key_states_) {
            if (state == KeyState::JUST_PRESSED) {
                state = KeyState::PRESSED;
            } else if (state == KeyState::JUST_RELEASED) {
                state = KeyState::UP;
            }
        }
    }

    void InputManager::ProcessEvent(const SDL_Event& event) {
        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.repeat == 0 && IsValidKeycode(event.key.key)) {
                key_states_[event.key.key] = KeyState::JUST_PRESSED;
            }
        } else if (event.type == SDL_EVENT_KEY_UP) {
            if (IsValidKeycode(event.key.key)) {
                key_states_[event.key.key] = KeyState::JUST_RELEASED;
            }
        }
    }

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

    void InputManager::BindAction(const std::string& action_name, int keycode) {
        if (!IsValidKeycode(keycode)) {
            return;
        }
        action_bindings_[action_name].push_back(keycode);
    }

    void InputManager::UnbindAction(const std::string& action_name) {
        action_bindings_.erase(action_name);
    }

    bool InputManager::IsActionPressed(const std::string& action_name) const {
        auto it = action_bindings_.find(action_name);
        if (it == action_bindings_.end()) {
            return false;
        }

        for (int keycode : it->second) {
            if (IsKeyPressed(keycode)) {
                return true;
            }
        }
        return false;
    }

    bool InputManager::IsActionJustPressed(const std::string& action_name) const {
        auto it = action_bindings_.find(action_name);
        if (it == action_bindings_.end()) {
            return false;
        }

        for (int keycode : it->second) {
            if (IsKeyJustPressed(keycode)) {
                return true;
            }
        }
        return false;
    }

    bool InputManager::IsActionJustReleased(const std::string& action_name) const {
        auto it = action_bindings_.find(action_name);
        if (it == action_bindings_.end()) {
            return false;
        }

        for (int keycode : it->second) {
            if (IsKeyJustReleased(keycode)) {
                return true;
            }
        }
        return false;
    }

    void InputManager::BindAxis(const std::string& axis_name, int positive_key, int negative_key) {
        if (!IsValidKeycode(positive_key) || !IsValidKeycode(negative_key)) {
            return;
        }
        axis_bindings_[axis_name] = {positive_key, negative_key};
    }

    float InputManager::GetAxis(const std::string& axis_name) const {
        auto it = axis_bindings_.find(axis_name);
        if (it == axis_bindings_.end()) {
            return 0.0f;
        }

        float value = 0.0f;
        if (IsKeyPressed(it->second.positive_key)) {
            value += 1.0f;
        }
        if (IsKeyPressed(it->second.negative_key)) {
            value -= 1.0f;
        }
        return value;
    }

    glm::vec2 InputManager::GetAxis2D(const std::string& horizontal_axis, const std::string& vertical_axis) const {
        float x = GetAxis(horizontal_axis);
        float y = GetAxis(vertical_axis);

        glm::vec2 result(x, y);
        float length_sq = x * x + y * y;
        if (length_sq > 1.0f) {
            result /= std::sqrt(length_sq);
        }

        return result;
    }

    void InputManager::Reset() {
        key_states_.fill(KeyState::UP);
        action_bindings_.clear();
        axis_bindings_.clear();
    }

} // namespace golias
