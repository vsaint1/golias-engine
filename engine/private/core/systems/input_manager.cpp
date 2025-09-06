#include "core/systems/input_manager.h"

#include "core/systems/logging_sys.h"
#include "core/engine.h"


GamepadInfo::~GamepadInfo() {
    if (controller) {
        SDL_CloseGamepad(controller);
        controller = nullptr;
        joystick   = nullptr;
    } else if (joystick) {
        SDL_CloseJoystick(joystick);
        joystick = nullptr;
    }
}

float GamepadInfo::get_axis_value(int axis) const {
    if (axis < 0 || axis >= static_cast<int>(axis_values.size())) {
        return 0.0f;
    }

    float value = axis_values[axis];
    return (std::abs(value) < deadzone) ? 0.0f : value;
}

glm::vec2 GamepadInfo::get_stick_value(int stick_index) const {

    if (stick_index == 0) {
        return glm::vec2(get_axis_value(0), get_axis_value(1));
    }

    if (stick_index == 1) {
        return glm::vec2(get_axis_value(2), get_axis_value(3));
    }

    return glm::vec2(0.0f);
}

void TextInputManager::start_input(SDL_Window* window) {
    if (!_active) {
        _active = true;
        SDL_StartTextInput(window);
    }
}

void TextInputManager::stop_input(SDL_Window* window) {
    if (_active) {
        _active = false;
        SDL_StopTextInput(window);
    }
}

void TextInputManager::process_event(const SDL_Event& event) {
    if (!_active) {
        return;
    }

    switch (event.type) {
    case SDL_EVENT_TEXT_INPUT:
        insert_text(event.text.text);
        break;

    case SDL_EVENT_TEXT_EDITING:
        _composition        = event.edit.text;
        _composition_start  = event.edit.start;
        _composition_length = event.edit.length;
        break;
    case SDL_EVENT_KEY_DOWN:
        switch (event.key.key) {
        case SDLK_BACKSPACE:
            delete_char_at_cursor();
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            insert_text("\n");
            break;
        case SDLK_LEFT:
            move_cursor(-1);
            break;
        case SDLK_RIGHT:
            move_cursor(1);
            break;
        case SDLK_HOME:
            _cursor_pos = 0;
            break;
        case SDLK_END:
            _cursor_pos = static_cast<int>(_text.length());
            break;
        }
        break;
    default:;
    }
}

void TextInputManager::clear_text() {
    _text.clear();
    _cursor_pos = 0;
    _composition.clear();
}

void TextInputManager::set_text(const std::string& text) {
    _text       = text;
    _cursor_pos = static_cast<int>(_text.length());
}

void TextInputManager::insert_text(const std::string& text) {
    _text.insert(_cursor_pos, text);
    _cursor_pos += static_cast<int>(text.length());
}

void TextInputManager::delete_char_at_cursor() {
    if (_cursor_pos > 0 && !_text.empty()) {
        _text.erase(_cursor_pos - 1, 1);
        _cursor_pos--;
    }
}

void TextInputManager::move_cursor(int offset) {
    _cursor_pos = std::max(0, std::min(static_cast<int>(_text.length()), _cursor_pos + offset));
}

InputManager::InputManager(SDL_Window* window) : _window(window) {

    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK)) {
        LOG_ERROR("Failed to initialize SDL joystick subsystem: %s", SDL_GetError());
    }

    int num_joysticks;
    SDL_JoystickID* joystick_ids = SDL_GetJoysticks(&num_joysticks);

    if (joystick_ids) {
        for (int i = 0; i < num_joysticks; ++i) {
            SDL_JoystickID instance_id = joystick_ids[i];

            if (SDL_IsGamepad(instance_id)) {
                SDL_Gamepad* gamepad_device = SDL_OpenGamepad(instance_id);
                if (gamepad_device) {
                    auto gamepad         = std::make_unique<GamepadInfo>();
                    gamepad->controller  = gamepad_device;
                    gamepad->joystick    = SDL_GetGamepadJoystick(gamepad_device);
                    gamepad->instance_id = instance_id;
                    gamepad->name = SDL_GetGamepadName(gamepad_device) != nullptr ? SDL_GetGamepadName(gamepad_device) : "UNKNOWN_GAMEPAD";
                    gamepad->is_controller = true;
                    gamepad->axis_values.resize(SDL_GAMEPAD_AXIS_COUNT, 0.0f);

                    _gamepads[_next_gamepad_index++] = std::move(gamepad);
                }
            } else {
                SDL_Joystick* joystick_device = SDL_OpenJoystick(instance_id);
                if (joystick_device) {
                    auto gamepad         = std::make_unique<GamepadInfo>();
                    gamepad->joystick    = joystick_device;
                    gamepad->instance_id = instance_id;
                    gamepad->name        = SDL_GetJoystickName(joystick_device) ? SDL_GetJoystickName(joystick_device) : "UNKNOWN_JOYSTICK";
                    gamepad->is_controller = false;

                    int num_axes = SDL_GetNumJoystickAxes(joystick_device);
                    gamepad->axis_values.resize(num_axes, 0.0f);

                    _gamepads[_next_gamepad_index++] = std::move(gamepad);
                }
            }
        }
        SDL_free(joystick_ids);
    }
}

#if defined(WITH_EDITOR)
glm::vec4 get_editor_viewport_rect()  {

    ImGuiWindow* window = ImGui::FindWindowByName("ViewportPane");
    if (!window) return glm::vec4(0,0,0,0);

    ImVec2 top_left = window->Pos;
    ImVec2 size     = window->Size;

    return {top_left.x, top_left.y, size.x, size.y}; // x, y, width, height
}
#endif


InputManager::~InputManager() {
    _gamepads.clear();
    SDL_QuitSubSystem(SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK);
}

void InputManager::process_event(const SDL_Event& event) {

    _text_input.process_event(event);

    switch (event.type) {
    case SDL_EVENT_QUIT:
        if (GEngine) {
            GEngine->is_running = false;
        }
        break;

    case SDL_EVENT_MOUSE_MOTION:
        glm::vec2 raw_pos(event.motion.x, event.motion.y);

#if defined(WITH_EDITOR)
        glm::vec4 vp = get_editor_viewport_rect();
        _mouse_position = raw_pos - glm::vec2(vp.x, vp.y);

        _mouse_position.x = glm::clamp(_mouse_position.x, 0.0f, vp.z);
        _mouse_position.y = glm::clamp(_mouse_position.y, 0.0f, vp.w);

#else
        _mouse_position = raw_pos;
#endif

        _mouse_delta = glm::vec2(event.motion.xrel, event.motion.yrel) * _mouse_sensitivity;
        break;

    case SDL_EVENT_MOUSE_WHEEL:
        _mouse_wheel = glm::vec2(event.wheel.x, event.wheel.y);
        break;

    case SDL_EVENT_KEY_DOWN:
        _keys_this_frame.insert(event.key.scancode);
        break;

    case SDL_EVENT_FINGER_DOWN:
    case SDL_EVENT_FINGER_MOTION:
    case SDL_EVENT_FINGER_UP:
        {
            SDL_FingerID finger_id = event.tfinger.fingerID;
            glm::vec2 position(event.tfinger.x, event.tfinger.y);
            float pressure = event.tfinger.pressure;

            if (event.type == SDL_EVENT_FINGER_UP) {
                _touch_points.erase(finger_id);
            } else {
                auto it = _touch_points.find(finger_id);
                if (it != _touch_points.end()) {
                    it->second.position   = position;
                    it->second.pressure.x = pressure;
                } else {
                    _touch_points[finger_id] = TouchPoint(finger_id, position, pressure);
                }
            }
            break;
        }

    case SDL_EVENT_GAMEPAD_ADDED:
    case SDL_EVENT_GAMEPAD_REMOVED:
        handle_gamepad_connection(event);
        break;
    default:
        break;
    }
}

void InputManager::update() {
    // Skip input processing if window is minimized
    if (_window && (SDL_GetWindowFlags(_window) & SDL_WINDOW_MINIMIZED)) {
        SDL_Delay(10);
        return;
    }

    update_key_states();
    update_mouse_states();
    update_gamepad_states();

    // Clear per-frame data
    _mouse_wheel = glm::vec2(0.0f);
    _keys_this_frame.clear();
}

void InputManager::late_update() {
    // Store previous states for next frame
    _prev_mouse_position = _mouse_position;
    _mouse_delta         = glm::vec2(0.0f);

    // Update previous key states
    for (const auto& [key, state] : _key_states) {
        _prev_key_states[key] = (state == InputState::PRESSED || state == InputState::HELD);
    }

    // Update previous mouse button states
    for (const auto& [button, state] : _mouse_button_states) {
        _prev_mouse_button_states[button] = (state == InputState::PRESSED || state == InputState::HELD);
    }

    // Update gamepad previous states
    for (auto& [index, gamepad] : _gamepads) {
        for (const auto& [button, state] : gamepad->button_states) {
            gamepad->prev_button_states[button] = (state == InputState::PRESSED || state == InputState::HELD);
        }
    }
}

// Keyboard functions
bool InputManager::is_key_pressed(SDL_Scancode key) const {
    if (_input_blocked) {
        return false;
    }
    auto it = _key_states.find(key);
    return it != _key_states.end() && it->second == InputState::PRESSED;
}

bool InputManager::is_key_held(SDL_Scancode key) const {
    if (_input_blocked) {
        return false;
    }
    auto it = _key_states.find(key);
    return it != _key_states.end() && (it->second == InputState::PRESSED || it->second == InputState::HELD);
}

bool InputManager::is_key_released(SDL_Scancode key) const {
    if (_input_blocked) {
        return false;
    }
    auto it = _key_states.find(key);
    return it != _key_states.end() && it->second == InputState::JUST_RELEASED;
}

InputState InputManager::get_key_state(SDL_Scancode key) const {
    if (_input_blocked) {
        return InputState::RELEASED;
    }
    auto it = _key_states.find(key);
    return it != _key_states.end() ? it->second : InputState::RELEASED;
}

bool InputManager::are_keys_held(const std::vector<SDL_Scancode>& keys) const {
    for (SDL_Scancode key : keys) {
        if (!is_key_held(key)) {
            return false;
        }
    }
    return !keys.empty();
}

bool InputManager::any_key_pressed(const std::vector<SDL_Scancode>& keys) const {
    for (SDL_Scancode key : keys) {
        if (is_key_pressed(key)) {
            return true;
        }
    }
    return false;
}

// Mouse functions
bool InputManager::is_mouse_button_pressed(MouseButton button) const {
    if (_input_blocked) {
        return false;
    }
    auto it = _mouse_button_states.find(button);
    return it != _mouse_button_states.end() && it->second == InputState::PRESSED;
}

bool InputManager::is_mouse_button_held(MouseButton button) const {
    if (_input_blocked) {
        return false;
    }
    auto it = _mouse_button_states.find(button);
    return it != _mouse_button_states.end() && (it->second == InputState::PRESSED || it->second == InputState::HELD);
}

bool InputManager::is_mouse_button_released(MouseButton button) const {
    if (_input_blocked) {
        return false;
    }
    auto it = _mouse_button_states.find(button);
    return it != _mouse_button_states.end() && it->second == InputState::JUST_RELEASED;
}

InputState InputManager::get_mouse_button_state(MouseButton button) const {
    if (_input_blocked) {
        return InputState::RELEASED;
    }
    auto it = _mouse_button_states.find(button);
    return it != _mouse_button_states.end() ? it->second : InputState::RELEASED;
}

void InputManager::set_mouse_position(int x, int y) {
    if (_window) {
        SDL_WarpMouseInWindow(_window, x, y);
        _mouse_position = glm::vec2(x, y);
    }
}

void InputManager::set_relative_mouse_mode(bool enabled) {
    return;
}

bool InputManager::is_relative_mouse_mode() const {
    return true;
}

glm::vec2 InputManager::screen_to_world(const glm::vec2& screen_pos) const {
    int w, h;
    SDL_GetWindowSize(_window, &w, &h);

    float vw = GEngine->Config.get_viewport().width;
    float vh = GEngine->Config.get_viewport().height;

    return {screen_pos.x * (vw / (float) w), screen_pos.y * (vh / (float) h)};
}

glm::vec2 InputManager::world_to_screen(const glm::vec2& world_pos) const {
    int w, h;
    SDL_GetWindowSize(_window, &w, &h);

    float vw = GEngine->Config.get_viewport().width;
    float vh = GEngine->Config.get_viewport().height;

    return {world_pos.x * ((float) w / vw), world_pos.y * ((float) h / vh)};
}

// Gamepad functions
const GamepadInfo* InputManager::get_gamepad(int index) const {
    auto it = _gamepads.find(index);
    return it != _gamepads.end() ? it->second.get() : nullptr;
}

bool InputManager::is_gamepad_connected(int index) const {
    const GamepadInfo* gamepad = get_gamepad(index);
    return gamepad && gamepad->is_connected();
}

bool InputManager::is_gamepad_button_pressed(int gamepad_index, int button) const {
    if (_input_blocked) {
        return false;
    }
    const GamepadInfo* gamepad = get_gamepad(gamepad_index);
    if (!gamepad) {
        return false;
    }

    auto it = gamepad->button_states.find(button);
    return it != gamepad->button_states.end() && it->second == InputState::PRESSED;
}

bool InputManager::is_gamepad_button_held(int gamepad_index, int button) const {
    if (_input_blocked) {
        return false;
    }
    const GamepadInfo* gamepad = get_gamepad(gamepad_index);
    if (!gamepad) {
        return false;
    }

    auto it = gamepad->button_states.find(button);
    return it != gamepad->button_states.end() && (it->second == InputState::PRESSED || it->second == InputState::HELD);
}

bool InputManager::is_gamepad_button_released(int gamepad_index, int button) const {
    if (_input_blocked) {
        return false;
    }
    const GamepadInfo* gamepad = get_gamepad(gamepad_index);
    if (!gamepad) {
        return false;
    }

    auto it = gamepad->button_states.find(button);
    return it != gamepad->button_states.end() && it->second == InputState::JUST_RELEASED;
}

float InputManager::get_gamepad_axis(int gamepad_index, int axis) const {
    const GamepadInfo* gamepad = get_gamepad(gamepad_index);
    return gamepad ? gamepad->get_axis_value(axis) : 0.0f;
}

glm::vec2 InputManager::get_gamepad_stick(int gamepad_index, int stick_index) const {
    const GamepadInfo* gamepad = get_gamepad(gamepad_index);
    return gamepad ? gamepad->get_stick_value(stick_index) : glm::vec2(0.0f);
}

void InputManager::set_gamepad_deadzone(int gamepad_index, float deadzone) {
    auto it = _gamepads.find(gamepad_index);
    if (it != _gamepads.end()) {
        it->second->deadzone = std::max(0.0f, std::min(1.0f, deadzone));
    }
}

// Touch functions
bool InputManager::is_touch_active(SDL_FingerID finger_id) const {
    auto it = _touch_points.find(finger_id);
    return it != _touch_points.end() && it->second.active;
}

glm::vec2 InputManager::get_touch_position(SDL_FingerID finger_id) const {
    auto it = _touch_points.find(finger_id);
    return it != _touch_points.end() ? it->second.position : glm::vec2(0.0f);
}

size_t InputManager::get_active_touch_count() const {
    size_t count = 0;
    for (const auto& [id, touch] : _touch_points) {
        if (touch.active) {
            count++;
        }
    }
    return count;
}

// Input Actions
void InputManager::register_action(const InputAction& action) {
    _input_actions[action.name] = action;
    LOG_INFO("Registered input action: %s", action.name.c_str());
}

void InputManager::unregister_action(const std::string& action_name) {
    _input_actions.erase(action_name);
}

bool InputManager::is_action_pressed(const std::string& action_name) const {
    auto it = _input_actions.find(action_name);
    if (it == _input_actions.end()) {
        return false;
    }

    const InputAction& action = it->second;

    // Check keys
    for (SDL_Scancode key : action.keys) {
        if (is_key_pressed(key)) {
            return true;
        }
    }

    // Check mouse buttons
    for (MouseButton button : action.mouse_buttons) {
        if (is_mouse_button_pressed(button)) {
            return true;
        }
    }

    // Check gamepad buttons (check all connected gamepads)
    for (const auto& [index, gamepad] : _gamepads) {
        if (gamepad->is_connected()) {
            for (int button : action.gamepad_buttons) {
                if (is_gamepad_button_pressed(index, button)) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool InputManager::is_action_held(const std::string& action_name) const {
    auto it = _input_actions.find(action_name);
    if (it == _input_actions.end()) {
        return false;
    }

    const InputAction& action = it->second;

    // Check keys
    for (SDL_Scancode key : action.keys) {
        if (is_key_held(key)) {
            return true;
        }
    }

    // Check mouse buttons
    for (MouseButton button : action.mouse_buttons) {
        if (is_mouse_button_held(button)) {
            return true;
        }
    }

    // Check gamepad buttons
    for (const auto& [index, gamepad] : _gamepads) {
        if (gamepad->is_connected()) {
            for (int button : action.gamepad_buttons) {
                if (is_gamepad_button_held(index, button)) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool InputManager::is_action_released(const std::string& action_name) const {
    auto it = _input_actions.find(action_name);
    if (it == _input_actions.end()) {
        return false;
    }

    const InputAction& action = it->second;

    for (SDL_Scancode key : action.keys) {
        if (is_key_released(key)) {
            return true;
        }
    }

    for (MouseButton button : action.mouse_buttons) {
        if (is_mouse_button_released(button)) {
            return true;
        }
    }

    for (const auto& [index, gamepad] : _gamepads) {
        if (gamepad->is_connected()) {
            for (int button : action.gamepad_buttons) {
                if (is_gamepad_button_released(index, button)) {
                    return true;
                }
            }
        }
    }

    return false;
}

// Utility functions
// Private helper functions
void InputManager::update_key_states() {
    const bool* keyboard_state = SDL_GetKeyboardState(nullptr);

    // Update all tracked keys
    for (auto& [scancode, state] : _key_states) {
        bool current          = keyboard_state[scancode];
        bool previous         = _prev_key_states[scancode];
        _key_states[scancode] = calculate_input_state(current, previous);
    }

    // Add new keys that were pressed this frame
    for (SDL_Scancode key : _keys_this_frame) {
        if (!_key_states.contains(key)) {
            _key_states[key]      = InputState::PRESSED;
            _prev_key_states[key] = false;
        }
    }
}

void InputManager::update_mouse_states() {
    Uint32 mouse_state = SDL_GetMouseState(&_mouse_position.x, &_mouse_position.y);


    // Update all mouse button states
    std::vector<MouseButton> buttons = {MouseButton::LEFT, MouseButton::MIDDLE, MouseButton::RIGHT, MouseButton::BUTTON_X1,
                                        MouseButton::BUTTON_X2};

    for (MouseButton button : buttons) {
        bool current                 = (mouse_state & SDL_BUTTON_MASK(static_cast<int>(button))) != 0;
        bool previous                = _prev_mouse_button_states[button];
        _mouse_button_states[button] = calculate_input_state(current, previous);
    }
}

void InputManager::update_gamepad_states() {
    for (auto& [index, gamepad] : _gamepads) {
        if (!gamepad->is_connected()) {
            continue;
        }

        if (gamepad->is_controller && gamepad->controller) {
            // Update gamepad button states
            for (int button = 0; button < SDL_GAMEPAD_BUTTON_COUNT; ++button) {
                bool current                   = SDL_GetGamepadButton(gamepad->controller, static_cast<SDL_GamepadButton>(button));
                bool previous                  = gamepad->prev_button_states[button];
                gamepad->button_states[button] = calculate_input_state(current, previous);
            }

            // Update axis values
            for (int axis = 0; axis < SDL_GAMEPAD_AXIS_COUNT; ++axis) {
                Sint16 value               = SDL_GetGamepadAxis(gamepad->controller, static_cast<SDL_GamepadAxis>(axis));
                gamepad->axis_values[axis] = value / 32767.0f; // Normalize to -1.0 to 1.0
            }
        } else if (gamepad->joystick) {
            // Update joystick button states
            int num_buttons = SDL_GetNumJoystickButtons(gamepad->joystick);
            for (int button = 0; button < num_buttons; ++button) {
                bool current                   = SDL_GetJoystickButton(gamepad->joystick, button);
                bool previous                  = gamepad->prev_button_states[button];
                gamepad->button_states[button] = calculate_input_state(current, previous);
            }

            // Update axis values
            int num_axes = SDL_GetNumJoystickAxes(gamepad->joystick);
            for (int axis = 0; axis < num_axes && axis < static_cast<int>(gamepad->axis_values.size()); ++axis) {
                Sint16 value               = SDL_GetJoystickAxis(gamepad->joystick, axis);
                gamepad->axis_values[axis] = value / 32767.0f;
            }
        }
    }
}

void InputManager::handle_gamepad_connection(const SDL_Event& event) {
    if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
        SDL_JoystickID instance_id = event.gdevice.which;

        if (SDL_IsGamepad(instance_id)) {
            SDL_Gamepad* gamepad_device = SDL_OpenGamepad(instance_id);
            if (gamepad_device) {
                auto gamepad           = std::make_unique<GamepadInfo>();
                gamepad->controller    = gamepad_device;
                gamepad->joystick      = SDL_GetGamepadJoystick(gamepad_device);
                gamepad->instance_id   = instance_id;
                gamepad->name          = SDL_GetGamepadName(gamepad_device) ? SDL_GetGamepadName(gamepad_device) : "UNKNOWN_GAMEPAD";
                gamepad->is_controller = true;
                gamepad->axis_values.resize(SDL_GAMEPAD_AXIS_COUNT, 0.0f);

                _gamepads[_next_gamepad_index++] = std::move(gamepad);
            }
        }
    } else if (event.type == SDL_EVENT_GAMEPAD_REMOVED) {
        SDL_JoystickID instance_id = event.gdevice.which;

        for (auto it = _gamepads.begin(); it != _gamepads.end(); ++it) {
            if (it->second->instance_id == instance_id) {
                _gamepads.erase(it);
                break;
            }
        }
    }
}

void InputManager::handle_gamepad_disconnection(const SDL_Event& event) {
    // This function is now handled in handle_gamepad_connection for SDL3
}

InputState InputManager::calculate_input_state(bool current, bool previous) const {
    if (current && !previous) {
        return InputState::PRESSED;
    } else if (current && previous) {
        return InputState::HELD;
    } else if (!current && previous) {
        return InputState::JUST_RELEASED;
    } else {
        return InputState::RELEASED;
    }
}

bool InputManager::position_in_rect(glm::vec2 position, const Rect2& rect) const {
    return position.x >= rect.x && position.x <= rect.x + rect.width && position.y >= rect.y && position.y <= rect.y + rect.height;
}

bool InputManager::mouse_in_rect(const Rect2& rect) const {
    return position_in_rect(_mouse_position, rect);
}

void InputManager::print_debug_info() const {
    LOG_INFO("=== InputManager Debug Info ===");
    LOG_INFO("Mouse Position: (%.1f, %.1f)", _mouse_position.x, _mouse_position.y);
    LOG_INFO("Mouse Delta: (%.1f, %.1f)", _mouse_delta.x, _mouse_delta.y);
    LOG_INFO("Connected Gamepads: %zu", _gamepads.size());
    LOG_INFO("Active Touch Points: %zu", get_active_touch_count());
    LOG_INFO("Text Input Active: %s", _text_input.is_active() ? "Yes" : "No");
    LOG_INFO("Input Blocked: %s", _input_blocked ? "Yes" : "No");

    std::vector<SDL_Scancode> pressed_keys = get_pressed_keys();
    if (!pressed_keys.empty()) {
        LOG_INFO("Pressed Keys: %zu", pressed_keys.size());
    }
}

std::vector<SDL_Scancode> InputManager::get_pressed_keys() const {
    std::vector<SDL_Scancode> pressed_keys;
    for (const auto& [scancode, state] : _key_states) {
        if (state == InputState::PRESSED || state == InputState::HELD) {
            pressed_keys.push_back(scancode);
        }
    }
    return pressed_keys;
}
