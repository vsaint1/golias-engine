#include "core/application.h"

#include <spdlog/spdlog.h>

namespace golias {

    Application::Application(const char* pTitle, int width, int height, ERenderingDeviceType deviceType)
        : title(pTitle), width(width), height(height), renderingDeviceType(deviceType) {

        window = SDL_CreateWindow(pTitle, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

        if (!window) {
            spdlog::error("Engine::Initialize Failed to create SDL Window : {}", SDL_GetError());
            return;
        }
    }

    void Application::RegisterTypes() {
    }

    bool Application::ShouldClose() const {
        return !is_running;
    }

    void Application::Close() {
        is_running = false;
    }

    Application::~Application() {
        if (window) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
    }
} // namespace golias
