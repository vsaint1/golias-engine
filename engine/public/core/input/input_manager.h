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

    InputManager(SDL_Window* window) : _window(window) {}

    void ProcessEvents(const SDL_Event* pEvent);

    void Update();

    glm::vec2 GetMousePosition();

    bool IsMouseButtonPressed(Uint8 button);

    bool IsMouseButtonReleased(Uint8 button);

    bool IsKeyPressed(SDL_Scancode key);

    bool IsKeyHeld(SDL_Scancode key);

    bool IsKeyReleased(SDL_Scancode key);

    bool IsPositionInRect(glm::vec2 position, ember::Rectangle rect);

    std::string GetTypedText();

    void SetTextInputActive(bool status);

    bool IsTextInputActive();

private:
    SDL_Window* _window;

    std::queue<SDL_Event> events;

    std::unordered_map<SDL_Scancode, bool> keyState;

    std::unordered_map<SDL_Scancode, bool> prevKeyState;

    glm::vec2 mousePosition;

    Joystick joystickData;

    std::unordered_map<int, Joystick> joysticks;

    std::unordered_map<int, TouchPoint> touchPoints;

    TextInputEvent textInputEvt;
};
