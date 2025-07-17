#include "core/input/input_manager.h"

#include "core/engine.h"

void InputManager::process(const SDL_Event* pEvent) {


    // Simple check but this may never fail
    if (!pEvent ) {
        return;
    }

    const SDL_Event& event = *pEvent;
    ImGui_ImplSDL3_ProcessEvent(&event);

    events.push(event);
    switch (event.type) {
    case SDL_EVENT_MOUSE_MOTION:
        mousePosition = {event.motion.x, event.motion.y};
        break;
    case SDL_EVENT_TEXT_INPUT:

        if (textInputEvt.bIsActive) {
            textInputEvt.text += event.text.text;
        }

        break;
    case SDL_EVENT_KEY_DOWN:
        if (textInputEvt.bIsActive) {
            switch (event.key.key) {
            case SDLK_BACKSPACE:
                if (!textInputEvt.text.empty()) {
                    textInputEvt.text.pop_back();
                }
                break;
            case SDLK_RETURN:
                textInputEvt.text += '\n';
                break;
            default:;
            }
        } else {
            if (!textInputEvt.text.empty()) {
                textInputEvt.text.clear();
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

void InputManager::update() {

    // Saving resources
    if (SDL_GetWindowFlags(_window) & SDL_WINDOW_MINIMIZED) {
        SDL_Delay(10);
        return;
    }

    prevKeyState = keyState;

    while (!events.empty()) {
        SDL_Event event = events.front();
        events.pop();

        if (event.type == SDL_EVENT_QUIT) {

            GEngine->bIsRunning = false;

        }

        if (event.type == SDL_EVENT_KEY_DOWN) {
            keyState[event.key.scancode] = true;
        } else if (event.type == SDL_EVENT_KEY_UP) {
            keyState[event.key.scancode] = false;
        }
    }
}

void InputManager::set_text_input_active(bool active) {
    if (active && !textInputEvt.bIsActive) {
        textInputEvt.bIsActive = true;
        SDL_StartTextInput(_window);
    } else {
        textInputEvt.bIsActive = false;
        SDL_StopTextInput(_window);
    }
}

bool InputManager::is_text_input_active() {
    return textInputEvt.bIsActive;
}

std::string InputManager::get_typed_text() {
    return textInputEvt.text;
}

bool InputManager::position_in_rect(glm::vec2 position, ember::Rectangle rect) {
    return position.x > rect.x && position.x < rect.x + rect.width && position.y > rect.y
        && position.y < rect.y + rect.height;
}

glm::vec2 InputManager::get_mouse_position() {

    return mousePosition;
}

bool InputManager::is_mouse_button_pressed(Uint8 button) {
    return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MASK(button);
}

bool InputManager::is_mouse_button_released(Uint8 button) {
    return !(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MASK(button));
}


bool InputManager::is_key_pressed(SDL_Scancode key) {
    return keyState[key] && !prevKeyState[key];
}

bool InputManager::is_key_released(SDL_Scancode key) {
    return !keyState[key] && prevKeyState[key];
}

bool InputManager::is_key_held(SDL_Scancode key) {
    return keyState[key];
}
