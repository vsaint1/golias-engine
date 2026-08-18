#pragma once
#include "core/input/input_manager.h"
#include "stdafx.h"

struct GLFWwindow;

namespace golias {

    class Application;

    class Engine {

    public:
        static Engine& GetInstance();

        bool Initialize(int width = 800, int height = 600, const String& title = "Golias Engine");

        void Run();

        void Shutdown();

        void SetApplication(Application* app);

        Application* GetApplication() const;

        InputManager& GetInputManager();

    private:
        Engine()                         = default;
        Engine(const Engine&)            = delete;
        Engine& operator=(const Engine&) = delete;
        Engine(Engine&&)                 = delete;
        Engine& operator=(Engine&&)      = delete;

    private:
        Scope<Application> mApplication = nullptr;

        std::chrono::high_resolution_clock::time_point mLastTime;

        GLFWwindow* mWindow = nullptr;

        InputManager mInputManager;
    };
} // namespace golias
