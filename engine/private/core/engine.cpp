#include "core/engine.h"

#include "core/application.h"


namespace golias {

    Engine& Engine::GetInstance() {
        static Engine instance;
        return instance;
    }

    bool Engine::Initialize(const char* title, int width, int height) {

        if (application) {
            if (!application->Initialize()) {
                spdlog::error("Failed to initialize the Application.");
                return false;
            }
        }

        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK)) {
            spdlog::error("Failed to initialize SDL : {}", SDL_GetError());
            return false;
        }

        window = SDL_CreateWindow(title, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

        if (!window) {
            spdlog::error("Failed to create SDL Window : {}", SDL_GetError());
            return false;
        }


        spdlog::info("Golias Engine Initialized successfully.");

        return true;
    }


    void Engine::Run() {

        if (!application) {
            return;
        }

        last_time_point = SDL_GetPerformanceCounter();

        while (!application->ShouldClose()) {

            input_manager.Update();

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    application->Close();
                }

                input_manager.ProcessEvent(event);
            }


            Uint64 current_time_point = SDL_GetPerformanceCounter();
            Uint64 time_delta         = current_time_point - last_time_point;
            float delta_time          = static_cast<float>(time_delta) / SDL_GetPerformanceFrequency();
            last_time_point           = current_time_point;

            application->Update(delta_time);

            // SDL_GL_SwapWindow(window);
        }
    }

    void Engine::Destroy() {

        if (application) {
            application->Destroy();
            application.reset();
        }

        SDL_DestroyWindow(window);
        SDL_Quit();

        spdlog::info("Golias Engine Destroyed successfully.");
    }

    void Engine::SetApplication(Application* pApplication) {
        spdlog::info("Setting Application for the Engine.");
        application.reset(pApplication);
    }

    Application* Engine::GetApplication() const {
        return application.get();
    }

    InputManager& Engine::GetInputManager() {
        return input_manager;
    }


} // namespace golias
