#include "core/renderer/ember_core.h"


void InitWindow(const char* title, int width, int height, Uint64 flags) {

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
        LOG_CRITICAL("Failed to initialize SDL: %s", SDL_GetError());
        return;
    }

    SDL_Window* _window = SDL_CreateWindow(title, width, height, flags);
    if (!_window) {
        LOG_CRITICAL("Failed to create window: %s", SDL_GetError());
        return;
    }

    renderer = CreateRenderer(_window, width, height);

    LOG_INFO("Created Window and Renderer");
}

void SetTargetFPS(int fps) {
    if (fps < 1) {
        core.Time.target = 0.0f;
    } else {
        core.Time.target = 1.0f / (float)fps;
    }

    core.Time.previous = SDL_GetTicks() / 1000.f;

    LOG_INFO("Target FPS(frames per second) to %02.03f ms", (float)core.Time.target * 1000.f);
}
