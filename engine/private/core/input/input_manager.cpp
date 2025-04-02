#include "core/input/input_manager.h"

void InputManager::ProcessEvents(const SDL_Event& event) {

    ImGui_ImplSDL3_ProcessEvent(&event);

    events.push(event);
    switch (event.type) {
    case SDL_EVENT_MOUSE_MOTION:
        mousePosition = {event.motion.x, event.motion.y};
        break;
    case SDL_EVENT_TEXT_INPUT:

        if (isTextInputActive) {
            textInput += event.text.text;
        }

        break;
    case SDL_EVENT_KEY_DOWN:
        if (isTextInputActive) {
            switch (event.key.key) {
            case SDLK_BACKSPACE:
                if (!textInput.empty()) {
                    textInput.pop_back();
                }
                break;
            case SDLK_RETURN:
                textInput += '\n';
                break;
            }
        } else {
            if (!textInput.empty()) {
                textInput.clear();
            }
        }
        break;
    case SDL_EVENT_FINGER_DOWN:
    case SDL_EVENT_FINGER_MOTION:
    case SDL_EVENT_FINGER_UP:
        {
            int fingerID          = event.tfinger.fingerID;
            glm::vec2 pos         = {event.tfinger.x, event.tfinger.y};
            touchPoints[fingerID] = {event.type != SDL_EVENT_FINGER_UP, pos};
            break;
        }
    default:

        break;
    }
}

void InputManager::Update() {

    // Saving resources
    if (SDL_GetWindowFlags(_window) & SDL_WINDOW_MINIMIZED) {
        SDL_Delay(10);
        return;
    }

    prevKeyState = keyState;

    while (!events.empty()) {
        SDL_Event event = events.front();
        events.pop();

        if (event.type == SDL_EVENT_KEY_DOWN) {
            keyState[event.key.scancode] = true;
        } else if (event.type == SDL_EVENT_KEY_UP) {
            keyState[event.key.scancode] = false;
        }
    }
}

void InputManager::SetWindow(SDL_Window* window) {
    _window = window;
}

void InputManager::SetTextInputActive(bool active) {
    if (active && !isTextInputActive) {
        isTextInputActive = true;
        SDL_StartTextInput(_window);
    } else {
        isTextInputActive = false;
        SDL_StopTextInput(_window);
    }
}

std::string InputManager::GetTypedText() {
    return textInput;
}

bool InputManager::IsPositionInRect(glm::vec2 position, ember::Rectangle rect) {
    return position.x > rect.x && position.x < rect.x + rect.width && position.y > rect.y
        && position.y < rect.y + rect.height;
}

glm::vec2 InputManager::GetMousePosition() {

    return mousePosition;
}

bool InputManager::IsMouseButtonPressed(Uint8 button) {
    return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MASK(button);
}

bool InputManager::IsMouseButtonReleased(Uint8 button) {
    return !(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MASK(button));
}


bool InputManager::IsKeyPressed(SDL_Scancode key) {
    return keyState[key] && !prevKeyState[key];
}

bool InputManager::IsKeyReleased(SDL_Scancode key) {
    return !keyState[key] && prevKeyState[key];
}

bool InputManager::IsKeyHeld(SDL_Scancode key) {
    return keyState[key];
}
