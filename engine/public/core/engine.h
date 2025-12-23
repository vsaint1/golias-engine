#pragma once
#include "core/input/input_manager.h"
#include <SDL3/SDL.h>
#include <memory>
#include <spdlog/spdlog.h>

namespace golias {

    class Application;

    class Engine {
    public:

        ~Engine() = default;

        static Engine& GetInstance();

        bool Initialize(const char* title = "Golias Engine", int width = 1280, int height = 720);

        void Run();

        void Destroy();

        void SetApplication(Application* pApplication);

        Application* GetApplication() const;

        InputManager& GetInputManager();

    private:
        Engine()  = default;
        Engine(const Engine&)            = delete;
        Engine& operator=(const Engine&) = delete;
        Engine(Engine&&)                 = delete;
        Engine& operator=(Engine&&)      = delete;

    private:
        InputManager input_manager;
        std::unique_ptr<Application> application;
        Uint64 last_time_point = 0;

        SDL_Window* window = nullptr;
    };

}; // namespace golias
