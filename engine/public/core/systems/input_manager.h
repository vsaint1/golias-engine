#pragma once
#include "core/engine_structs.h"

enum class InputState {
    RELEASED = 0,
    PRESSED  = 1,
    HELD     = 2,
    JUST_RELEASED = 3
};

enum class MouseButton {
    LEFT   = SDL_BUTTON_LEFT,
    MIDDLE = SDL_BUTTON_MIDDLE,
    RIGHT  = SDL_BUTTON_RIGHT,
    BUTTON_X1     = SDL_BUTTON_X1,
    BUTTON_X2     = SDL_BUTTON_X2
};

struct InputAction {
    std::string name;
    std::vector<SDL_Scancode> keys;
    std::vector<MouseButton> mouse_buttons;
    std::vector<int> gamepad_buttons;

    InputAction() = default;

    explicit InputAction(const std::string& action_name) : name(action_name) {}

    InputAction& bind_key(SDL_Scancode key) {
        keys.push_back(key);
        return *this;
    }

    InputAction& bind_mouse(MouseButton button) {
        mouse_buttons.push_back(button);
        return *this;
    }

    InputAction& bind_gamepad_button(int button) {
        gamepad_buttons.push_back(button);
        return *this;
    }
};

struct GamepadInfo {
    SDL_Joystick* joystick = nullptr;
    SDL_Gamepad* controller = nullptr;
    SDL_JoystickID instance_id;
    std::string name;
    bool is_controller = false;

    // Button states (current and previous frame)
    std::unordered_map<int, InputState> button_states;
    std::unordered_map<int, bool> prev_button_states;

    // Axis values (-1.0 to 1.0)
    std::vector<float> axis_values;

    // Deadzone for analog sticks
    float deadzone = 0.15f;

    GamepadInfo() = default;
    ~GamepadInfo();

    bool is_connected() const { return joystick != nullptr; }
    float get_axis_value(int axis) const;
    glm::vec2 get_stick_value(int stick_index) const; // 0 = left stick, 1 = right stick
};

struct TouchPoint {
    SDL_FingerID finger_id;
    bool active = false;
    glm::vec2 position{0.0f};
    glm::vec2 pressure{0.0f}; // x = pressure, y = unused
    glm::vec2 start_position{0.0f};
    float start_time = 0.0f;

    TouchPoint() = default;
    TouchPoint(SDL_FingerID id, glm::vec2 pos, float press = 1.0f)
        : finger_id(id), active(true), position(pos), pressure(press, 0.0f), start_position(pos) {}
};


/* !
 * @brief InputManager class
 *
 * @details Manages keyboard, mouse, gamepad, and touch input.
 *
 * @version 1.0.0
 */
class InputManager {
public:
    InputManager() = default;
    explicit InputManager(SDL_Window* window);
    ~InputManager();

    // Core update functions
    void process_event(const SDL_Event& event);
    void update(); // Call once per frame after processing all events
    void late_update(); // Call at end of frame to transition states

    // === KEYBOARD INPUT ===
    [[nodiscard]] bool is_key_pressed(SDL_Scancode key) const;
    [[nodiscard]] bool is_key_held(SDL_Scancode key) const;
    [[nodiscard]] bool is_key_released(SDL_Scancode key) const;
    [[nodiscard]] InputState get_key_state(SDL_Scancode key) const;

    // Multiple key combinations
    [[nodiscard]] bool are_keys_held(const std::vector<SDL_Scancode>& keys) const;
    [[nodiscard]] bool any_key_pressed(const std::vector<SDL_Scancode>& keys) const;

    // === MOUSE INPUT ===
    [[nodiscard]] glm::vec2 get_mouse_position() const { return _mouse_position; }
    [[nodiscard]] glm::vec2 get_mouse_delta() const { return _mouse_delta; }
    [[nodiscard]] glm::vec2 get_mouse_wheel() const { return _mouse_wheel; }

    [[nodiscard]] bool is_mouse_button_pressed(MouseButton button) const;
    [[nodiscard]] bool is_mouse_button_held(MouseButton button) const;
    [[nodiscard]] bool is_mouse_button_released(MouseButton button) const;
    [[nodiscard]] InputState get_mouse_button_state(MouseButton button) const;

    // Mouse utilities
    void set_mouse_position(int x, int y);
    void set_relative_mouse_mode(bool enabled);
    [[nodiscard]] bool is_relative_mouse_mode() const;
    glm::vec2 screen_to_world(const glm::vec2& screen_pos) const;
    glm::vec2 world_to_screen(const glm::vec2& world_pos) const;


    // === GAMEPAD INPUT ===
    [[nodiscard]] size_t get_gamepad_count() const { return _gamepads.size(); }
    [[nodiscard]] const GamepadInfo* get_gamepad(int index) const;
    [[nodiscard]] bool is_gamepad_connected(int index) const;

    [[nodiscard]] bool is_gamepad_button_pressed(int gamepad_index, int button) const;
    [[nodiscard]] bool is_gamepad_button_held(int gamepad_index, int button) const;
    [[nodiscard]] bool is_gamepad_button_released(int gamepad_index, int button) const;

    [[nodiscard]] float get_gamepad_axis(int gamepad_index, int axis) const;
    [[nodiscard]] glm::vec2 get_gamepad_stick(int gamepad_index, int stick_index) const;

    void set_gamepad_deadzone(int gamepad_index, float deadzone);

    // === TOUCH INPUT ===
    [[nodiscard]] const std::unordered_map<SDL_FingerID, TouchPoint>& get_touch_points() const { return _touch_points; }
    [[nodiscard]] bool is_touch_active(SDL_FingerID finger_id) const;
    [[nodiscard]] glm::vec2 get_touch_position(SDL_FingerID finger_id) const;
    [[nodiscard]] size_t get_active_touch_count() const;

    // === INPUT ACTIONS ===
    void register_action(const InputAction& action);
    void unregister_action(const std::string& action_name);
    [[nodiscard]] bool is_action_pressed(const std::string& action_name) const;
    [[nodiscard]] bool is_action_held(const std::string& action_name) const;
    [[nodiscard]] bool is_action_released(const std::string& action_name) const;

    // === UTILITIES ===
    [[nodiscard]] bool position_in_rect(glm::vec2 position, const Rect2& rect) const;
    [[nodiscard]] bool mouse_in_rect(const Rect2& rect) const;

    // Input blocking (useful for UI)
    void set_input_blocked(bool blocked) { _input_blocked = blocked; }
    [[nodiscard]] bool is_input_blocked() const { return _input_blocked; }

    // Debug
    [[nodiscard]] std::vector<SDL_Scancode> get_pressed_keys() const;

    // Configuration
    void set_mouse_sensitivity(float sensitivity) { _mouse_sensitivity = sensitivity; }
    [[nodiscard]] float get_mouse_sensitivity() const { return _mouse_sensitivity; }

     SDL_Event get_last_event() const { return _last_event; }

     std::string get_last_text_input()  const;

private:
    SDL_Event _last_event;
    std::string _last_text_input;

    mutable  bool _has_fresh_text_input = false;

    // Internal helper functions
    void update_key_states();
    void update_mouse_states();
    void update_gamepad_states();
    void handle_gamepad_connection(const SDL_Event& event);
    void handle_gamepad_disconnection(const SDL_Event& event);
    InputState calculate_input_state(bool current, bool previous) const;

    // Core data
    SDL_Window* _window = nullptr;
    bool _input_blocked = false;
    float _mouse_sensitivity = 1.0f;

    // Keyboard state
    std::unordered_map<SDL_Scancode, InputState> _key_states;
    std::unordered_map<SDL_Scancode, bool> _prev_key_states;
    std::unordered_set<SDL_Scancode> _keys_this_frame;

    // Mouse state
    glm::vec2 _mouse_position{0.0f};
    glm::vec2 _prev_mouse_position{0.0f};
    glm::vec2 _mouse_delta{0.0f};
    glm::vec2 _mouse_wheel{0.0f};
    std::unordered_map<MouseButton, InputState> _mouse_button_states;
    std::unordered_map<MouseButton, bool> _prev_mouse_button_states;

    // Gamepad state
    std::unordered_map<int, std::unique_ptr<GamepadInfo>> _gamepads;
    int _next_gamepad_index = 0;

    // Touch state
    std::unordered_map<SDL_FingerID, TouchPoint> _touch_points;

    // Input actions
    std::unordered_map<std::string, InputAction> _input_actions;
};
