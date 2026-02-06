#include "core/input/input_manager.h"

#include <algorithm>
#include <cmath>

namespace golias {

    // ================================================================
    //  Keyboard
    // ================================================================
    bool InputManager::IsKeyPressed(SDL_Keycode keycode) const {
        auto state = GetKeyState(keycode);
        return state == KeyState::PRESSED || state == KeyState::JUST_PRESSED;
    }

    bool InputManager::IsKeyJustPressed(SDL_Keycode keycode) const {
        return GetKeyState(keycode) == KeyState::JUST_PRESSED;
    }

    bool InputManager::IsKeyJustReleased(SDL_Keycode keycode) const {
        return GetKeyState(keycode) == KeyState::JUST_RELEASED;
    }

    InputManager::KeyState InputManager::GetKeyState(SDL_Keycode keycode) const {
        auto it = key_states_.find(keycode);
        if (it == key_states_.end()) {
            return KeyState::UP;
        }
        return it->second;
    }

    // ================================================================
    //  Mouse
    // ================================================================
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

    float InputManager::GetMouseWheelDelta() const {
        return mouse_wheel_delta_;
    }

    const std::string& InputManager::GetTextInput() const {
        return text_input_;
    }

    InputManager::KeyState InputManager::GetMouseState(int button) const {
        if (!IsValidMouseButton(button)) {
            return KeyState::UP;
        }
        return mouse_states_[button];
    }

    // ================================================================
    //  Controller
    // ================================================================
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

    // ================================================================
    //  Actions
    // ================================================================
    void InputManager::BindAction(const std::string& action, SDL_Keycode keycode) {
        actions_[action].push_back({BindingType::KEYBOARD, static_cast<int>(keycode)});
    }

    void InputManager::BindActionMouse(const std::string& action, int button) {
        actions_[action].push_back({BindingType::MOUSE, button});
    }

    void InputManager::BindActionController(const std::string& action, int button) {
        actions_[action].push_back({BindingType::CONTROLLER, button});
    }

    bool InputManager::IsActionPressed(const std::string& action) const {
        auto it = actions_.find(action);
        if (it == actions_.end()) return false;
        for (const auto& binding : it->second) {
            if (CheckBinding(binding)) return true;
        }
        return false;
    }

    bool InputManager::IsActionJustPressed(const std::string& action) const {
        auto it = actions_.find(action);
        if (it == actions_.end()) return false;
        for (const auto& binding : it->second) {
            if (CheckBindingJustPressed(binding)) return true;
        }
        return false;
    }

    bool InputManager::IsActionJustReleased(const std::string& action) const {
        auto it = actions_.find(action);
        if (it == actions_.end()) return false;
        for (const auto& binding : it->second) {
            if (CheckBindingJustReleased(binding)) return true;
        }
        return false;
    }

    bool InputManager::CheckBinding(const Binding& b) const {
        switch (b.type) {
        case BindingType::KEYBOARD:   return IsKeyPressed(static_cast<SDL_Keycode>(b.code));
        case BindingType::MOUSE:      return IsMouseButtonPressed(b.code);
        case BindingType::CONTROLLER: return IsControllerButtonPressed(b.code);
        }
        return false;
    }

    bool InputManager::CheckBindingJustPressed(const Binding& b) const {
        switch (b.type) {
        case BindingType::KEYBOARD:   return IsKeyJustPressed(static_cast<SDL_Keycode>(b.code));
        case BindingType::MOUSE:      return IsMouseButtonJustPressed(b.code);
        case BindingType::CONTROLLER: return IsControllerButtonJustPressed(b.code);
        }
        return false;
    }

    bool InputManager::CheckBindingJustReleased(const Binding& b) const {
        switch (b.type) {
        case BindingType::KEYBOARD:   return IsKeyJustReleased(static_cast<SDL_Keycode>(b.code));
        case BindingType::MOUSE:      return IsMouseButtonJustReleased(b.code);
        case BindingType::CONTROLLER: return IsControllerButtonJustReleased(b.code);
        }
        return false;
    }

    // ================================================================
    //  Virtual axes  (Unity-style "Horizontal", "Vertical", "Mouse X", "Mouse Y")
    // ================================================================
    float InputManager::GetAxis(const std::string& axisName) const {
        // Built-in axis names
        if (axisName == "Horizontal") {
            float raw = 0.0f;
            if (IsKeyPressed(SDLK_D) || IsKeyPressed(SDLK_RIGHT)) raw += 1.0f;
            if (IsKeyPressed(SDLK_A) || IsKeyPressed(SDLK_LEFT))  raw -= 1.0f;
            // Smooth toward target
            float& val = axis_values_[axisName];
            float speed = 6.0f; // units per second  approximate
            if (std::abs(raw - val) < 0.01f) {
                val = raw;
            } else {
                val += (raw - val) * std::min(1.0f, speed * 0.016f);
            }
            if (gamepad_) {
                float stick = GetControllerAxis(SDL_GAMEPAD_AXIS_LEFTX);
                if (std::abs(stick) > std::abs(val)) val = stick;
            }
            return val;
        }

        if (axisName == "Vertical") {
            float raw = 0.0f;
            if (IsKeyPressed(SDLK_W) || IsKeyPressed(SDLK_UP))   raw += 1.0f;
            if (IsKeyPressed(SDLK_S) || IsKeyPressed(SDLK_DOWN))  raw -= 1.0f;
            float& val = axis_values_[axisName];
            float speed = 6.0f;
            if (std::abs(raw - val) < 0.01f) {
                val = raw;
            } else {
                val += (raw - val) * std::min(1.0f, speed * 0.016f);
            }
            if (gamepad_) {
                float stick = -GetControllerAxis(SDL_GAMEPAD_AXIS_LEFTY);
                if (std::abs(stick) > std::abs(val)) val = stick;
            }
            return val;
        }

        if (axisName == "Mouse X") return mouse_delta_.x;
        if (axisName == "Mouse Y") return mouse_delta_.y;
        if (axisName == "Mouse ScrollWheel") return mouse_wheel_delta_;

        // Controller axes
        if (axisName == "RightStickHorizontal" && gamepad_) return GetControllerAxis(SDL_GAMEPAD_AXIS_RIGHTX);
        if (axisName == "RightStickVertical"   && gamepad_) return -GetControllerAxis(SDL_GAMEPAD_AXIS_RIGHTY);

        return 0.0f;
    }

    float InputManager::GetAxisRaw(const std::string& axisName) const {
        if (axisName == "Horizontal") {
            float val = 0.0f;
            if (IsKeyPressed(SDLK_D) || IsKeyPressed(SDLK_RIGHT)) val += 1.0f;
            if (IsKeyPressed(SDLK_A) || IsKeyPressed(SDLK_LEFT))  val -= 1.0f;
            if (gamepad_) {
                float stick = GetControllerAxis(SDL_GAMEPAD_AXIS_LEFTX);
                if (std::abs(stick) > std::abs(val)) val = stick;
            }
            return val;
        }
        if (axisName == "Vertical") {
            float val = 0.0f;
            if (IsKeyPressed(SDLK_W) || IsKeyPressed(SDLK_UP))   val += 1.0f;
            if (IsKeyPressed(SDLK_S) || IsKeyPressed(SDLK_DOWN))  val -= 1.0f;
            if (gamepad_) {
                float stick = -GetControllerAxis(SDL_GAMEPAD_AXIS_LEFTY);
                if (std::abs(stick) > std::abs(val)) val = stick;
            }
            return val;
        }
        if (axisName == "Mouse X") return mouse_delta_.x;
        if (axisName == "Mouse Y") return mouse_delta_.y;
        if (axisName == "Mouse ScrollWheel") return mouse_wheel_delta_;
        return 0.0f;
    }

    // ================================================================
    //  Any key
    // ================================================================
    bool InputManager::AnyKey() const {
        return anyKey_;
    }

    bool InputManager::AnyKeyDown() const {
        return anyKeyDown_;
    }

    // ================================================================
    //  Per-frame update
    // ================================================================
    void InputManager::Update() {
        anyKeyDown_ = false;
        bool hasAnyPressed = false;

        // Keyboard  iterate the map and advance states
        for (auto it = key_states_.begin(); it != key_states_.end(); ) {
            auto& state = it->second;
            if (state == KeyState::JUST_PRESSED) {
                state = KeyState::PRESSED;
            } else if (state == KeyState::JUST_RELEASED) {
                // Remove entries that are UP to keep the map lean
                it = key_states_.erase(it);
                continue;
            }
            if (state == KeyState::PRESSED || state == KeyState::JUST_PRESSED) {
                hasAnyPressed = true;
            }
            ++it;
        }

        for (auto& state : mouse_states_) {
            if (state == KeyState::JUST_PRESSED)  state = KeyState::PRESSED;
            else if (state == KeyState::JUST_RELEASED) state = KeyState::UP;
        }
        mouse_delta_       = glm::vec2(0.0f);
        mouse_wheel_delta_ = 0.0f;
        text_input_.clear();

        if (gamepad_) {
            for (auto& state : controller_states_) {
                if (state == KeyState::JUST_PRESSED)  state = KeyState::PRESSED;
                else if (state == KeyState::JUST_RELEASED) state = KeyState::UP;
            }
        }

        anyKey_ = hasAnyPressed || IsMouseButtonPressed(SDL_BUTTON_LEFT) || 
                  IsMouseButtonPressed(SDL_BUTTON_RIGHT);
    }

    // ================================================================
    //  Event processing
    // ================================================================
    void InputManager::ProcessEvent(const SDL_Event& event) {
        switch (event.type) {
        case SDL_EVENT_KEY_DOWN:
            {
                SDL_Keycode k = event.key.key;
                auto it = key_states_.find(k);
                if (it == key_states_.end() || it->second == KeyState::UP) {
                    key_states_[k] = KeyState::JUST_PRESSED;
                    anyKeyDown_ = true;
                }
                break;
            }
        case SDL_EVENT_KEY_UP:
            {
                SDL_Keycode k = event.key.key;
                key_states_[k] = KeyState::JUST_RELEASED;
                break;
            }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                int b = event.button.button;
                if (IsValidMouseButton(b) && mouse_states_[b] == KeyState::UP) {
                    mouse_states_[b] = KeyState::JUST_PRESSED;
                    anyKeyDown_ = true;
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
        case SDL_EVENT_MOUSE_WHEEL:
            {
                mouse_wheel_delta_ = event.wheel.y;
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
                        anyKeyDown_ = true;
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
        case SDL_EVENT_TEXT_INPUT:
            {
                text_input_ = event.text.text;
                break;
            }
        }
    }

    // ================================================================
    //  Reset
    // ================================================================
    void InputManager::Reset() {
        key_states_.clear();
        mouse_states_.fill(KeyState::UP);
        mouse_pos_         = glm::vec2(0.0f);
        mouse_delta_       = glm::vec2(0.0f);
        mouse_wheel_delta_ = 0.0f;
        text_input_.clear();

        if (gamepad_) {
            SDL_CloseGamepad(gamepad_);
            gamepad_ = nullptr;
        }
        controller_states_.fill(KeyState::UP);
        controller_axes_.fill(0.0f);

        actions_.clear();
        axis_values_.clear();
        anyKeyDown_ = false;
        anyKey_     = false;
    }

} // namespace golias