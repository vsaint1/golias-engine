#include "core/engine.h"

#include "core/application.h"
#include "scene/3d/camera_component.h"

bool create_renderer_internal(golias::ERenderingDeviceType deviceType, golias::RenderingDevice** pOutDevice) {
    switch (deviceType) {
    case golias::ERenderingDeviceType::COMPATIBILITY:
        *pOutDevice = new golias::RenderingDeviceGLES3();
        return true;
    case golias::ERenderingDeviceType::FORWARD_PLUS:
        spdlog::error("FORWARD_PLUS rendering device not implemented yet.");
        return false;
    default:
        spdlog::error("Unknown rendering device type.");
        return false;
    }
}

namespace golias {

    Engine& Engine::GetInstance() {
        static Engine instance;
        return instance;
    }

    bool Engine::Initialize(const char* pTitle, int width, int height, ERenderingDeviceType deviceType) {

        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK)) {
            spdlog::error("Engine::Initialize Failed to initialize SDL : {}", SDL_GetError());
            return false;
        }

        _width  = width;
        _height = height;
        window  = SDL_CreateWindow(pTitle, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

        if (!window) {
            spdlog::error("Engine::Initialize Failed to create SDL Window : {}", SDL_GetError());
            return false;
        }


        // SDL_SetWindowRelativeMouseMode(window, true);

        if (!create_renderer_internal(deviceType, &rendering_device)) {
            spdlog::error("Engine::Initialize Failed to create Rendering Device.");
            return false;
        }

        if (!rendering_device->Initialize(window)) {
            spdlog::error("Engine::InitializeFailed to initialize Rendering Device.");
            return false;
        }

        if (application) {
            if (!application->Initialize()) {
                spdlog::error("Engine::Initialize Failed to initialize the Application.");
                return false;
            }
        }

        spdlog::info("Engine::Initialize Golias Engine Initialized successfully.");

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

                if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                    _width  = event.window.data1;
                    _height = event.window.data2;
                }

                input_manager.ProcessEvent(event);
            }


            Uint64 current_time_point = SDL_GetPerformanceCounter();
            Uint64 time_delta         = current_time_point - last_time_point;
            float delta_time          = static_cast<float>(time_delta) / SDL_GetPerformanceFrequency();
            last_time_point           = current_time_point;

            application->Update(delta_time);

            CameraData camera_data;
            if (scene && scene->GetMainCamera()) {
                auto camera_component = scene->GetMainCamera()->GetComponent<CameraComponent>();
                
                if (camera_component) {
                    camera_data.viewMatrix = camera_component->GetViewMatrix();

                    float aspect            = static_cast<float>(_width) / static_cast<float>(_height);
                    camera_data.projectionMatrix = camera_component->GetProjectionMatrix(aspect);
                }
            }

            rendering_device->Clear();

            rendering_canvas.Draw(rendering_device, camera_data);
            rendering_device->Present();
        }
    }

    void Engine::Destroy() {

        if (application) {
            application->Destroy();
            application.reset();
        }

        delete rendering_device;
        rendering_device = nullptr;

        SDL_DestroyWindow(window);
        SDL_Quit();

        spdlog::info("Engine::Destroy Golias Engine Destroyed successfully.");
    }

    void Engine::SetApplication(Application* pApplication) {
        spdlog::info("Engine::SetApplication Setting Application for the Engine.");
        application.reset(pApplication);
    }

    Application* Engine::GetApplication() const {
        return application.get();
    }

    InputManager& Engine::GetInputManager() {
        return input_manager;
    }

    RenderingDevice* Engine::GetRenderingDevice() {
        return rendering_device;
    }


    RenderingCanvas& Engine::GetRenderingCanvas() {
        return rendering_canvas;
    }

    Scene* Engine::GetScene() const {
        return scene.get();
    }

    void Engine::SetScene(Scene* pScene) {
        scene.reset(pScene);
    }
} // namespace golias
