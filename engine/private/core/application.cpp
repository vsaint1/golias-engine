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


    SDL_Window* Application::GetWindowNativeHandle() const {
        return window;

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

    void Application::SetTitle(const std::string_view pTitle) {
        title = pTitle;
        if (window) {
            SDL_SetWindowTitle(window, title.data());
        }
    }

    const std::string& Application::GetTitle() const {
        return title;
    }

    int Application::GetWidth() const {
        return width;
    }

    int Application::GetHeight() const {
        return height;
    }

    void Application::SetWidth(int w) {
        width = w;
    }

    void Application::SetHeight(int h) {
        height = h;
    }

    ERenderingDeviceType Application::GetRenderingDeviceType() const {
        return renderingDeviceType;
    }

} // namespace golias
