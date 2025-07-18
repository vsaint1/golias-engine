#pragma once
#include "core/engine_structs.h"

struct Joystick {
    SDL_Joystick* joystick;
    std::unordered_map<int, bool> buttonState;
    std::vector<float> axisValues;
};

struct TouchPoint {
    bool active;
    glm::vec2 position;
};

struct TextInputEvent {
    std::string text = "";
    bool bIsActive   = false;
};

class InputManager {

public:
    InputManager() = default;

    explicit InputManager(SDL_Window* window) : _window(window) {}

    void process(const SDL_Event* pEvent);

    void update();

    glm::vec2 get_mouse_position();

    bool is_mouse_button_pressed(Uint8 button);

    bool is_mouse_button_released(Uint8 button);

    bool is_key_pressed(SDL_Scancode key);

    bool is_key_held(SDL_Scancode key);

    bool is_key_released(SDL_Scancode key);

    bool position_in_rect(glm::vec2 position, ember::Rectangle rect);

    std::string get_typed_text();

    void set_text_input_active(bool status);

    bool is_text_input_active();

private:
    SDL_Window* _window = nullptr;

    std::queue<SDL_Event> events;

    std::unordered_map<SDL_Scancode, bool> keyState;

    std::unordered_map<SDL_Scancode, bool> prevKeyState;

    glm::vec2 mousePosition = glm::vec2(0.0f);

    Joystick joystickData = {};

    std::unordered_map<int, Joystick> joysticks;

    std::unordered_map<int, TouchPoint> touchPoints;

    TextInputEvent textInputEvt;
};
