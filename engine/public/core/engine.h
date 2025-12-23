#pragma once
#include "core/graphics/rendering_canvas.h"
#include "core/graphics/rendering_device.h"
#include "core/input/input_manager.h"

namespace golias {

    class Application;

    class Engine {
    public:
        ~Engine() = default;

        static Engine& GetInstance();

        bool Initialize(const char* pTitle = "Golias Engine", int width = 1280, int height = 720,
                        ERenderingDeviceType deviceType = ERenderingDeviceType::COMPATIBILITY);

        void Run();

        void Destroy();

        void SetApplication(Application* pApplication);

        Application* GetApplication() const;

        InputManager& GetInputManager();

        RenderingDevice* GetRenderingDevice();

        RenderingCanvas& GetRenderingCanvas();

    private:
        Engine()                         = default;
        Engine(const Engine&)            = delete;
        Engine& operator=(const Engine&) = delete;
        Engine(Engine&&)                 = delete;
        Engine& operator=(Engine&&)      = delete;

    private:
        InputManager input_manager;

        std::unique_ptr<Application> application;
        Uint64 last_time_point = 0;

        SDL_Window* window = nullptr;

    private:
        RenderingDevice* rendering_device = nullptr;
        RenderingCanvas rendering_canvas;
    };

}; // namespace golias
