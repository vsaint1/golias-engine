#include "core/engine.h"

#include "core/application.h"
#include "scene/3d/camera_component.h"


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


        SDL_SetWindowRelativeMouseMode(window, true);

        if (!sceneRenderer.Initialize(window, deviceType)) {
            spdlog::error("Engine::Initialize Failed to initialize Rendering Canvas.");
            return false;
        }


        if (!physicsManager.Initialize(sceneRenderer.GetRenderingDevice()->GetPhysicsDebugDrawer())) {
            spdlog::error("Engine::Initialize Failed to initialize Physics Manager.");
            return false;
        }

        Scene::RegisterTypes();

        if (!audioManager.Initialize()) {
            spdlog::error("Engine::Initialize Failed to initialize Audio Manager.");
            return false;
        }

        if (application) {
            application->RegisterTypes();

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

            Uint64 current_time_point = SDL_GetPerformanceCounter();
            Uint64 time_delta         = current_time_point - last_time_point;
            float delta_time          = static_cast<float>(time_delta) / SDL_GetPerformanceFrequency();
            last_time_point           = current_time_point;

            inputManager.Update();

            SDL_Event event;

            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    application->Close();
                }

                if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                    _width  = event.window.data1;
                    _height = event.window.data2;
                }

                inputManager.ProcessEvent(event);
            }


            physicsManager.StepSimulation(delta_time);

            application->Update(delta_time);


            CameraCommand cameraData;
            if (scene && scene->GetMainCamera()) {

                auto pCameraComponent = scene->GetMainCamera()->GetComponent<CameraComponent>();

                if (pCameraComponent) {
                    cameraData.viewMatrix = pCameraComponent->GetViewMatrix();

                    float aspect                  = static_cast<float>(_width) / static_cast<float>(_height);
                    cameraData.projectionMatrix   = pCameraComponent->GetProjectionMatrix(aspect);
                    cameraData.orthographicMatrix = glm::ortho(0.0f, static_cast<float>(_width), 0.0f, static_cast<float>(_height));

                    cameraData.position = scene->GetMainCamera()->GetWorldPosition();
                }
            }

            sceneRenderer.Clear();

            sceneRenderer.Draw(cameraData);

            if (GetPhysicsManager().IsDebugDrawEnabled()) {
                GetPhysicsManager().RenderDebug(cameraData.projectionMatrix * cameraData.viewMatrix);
            }

            sceneRenderer.Present();

            SDL_Delay(16); // HACK for development purposes
        }
    }

    void Engine::Destroy() {

        if (application) {
            application->Destroy();
            application.reset();
        }

        SDL_DestroyWindow(window);
        SDL_Quit();

        spdlog::info("Engine::Destroy Cleanup phase completed, Engine systems destroyed.");
    }

    void Engine::SetApplication(Application* pApplication) {
        spdlog::info("Engine::SetApplication Setting Application for the Engine.");
       
        if (application) {
            spdlog::warn("Engine::SetApplication Overwriting existing Application instance.");
            application->Destroy();
        }

        application.reset(pApplication);
    }

    void Engine::SetScene(const std::shared_ptr<Scene>& pScene) {
        scene = pScene;
    }


    Application* Engine::GetApplication() const {
        return application.get();
    }

    InputManager& Engine::GetInputManager() {
        return inputManager;
    }

    AudioManager& Engine::GetAudioManager() {
        return audioManager;
    }

    SceneRenderer& Engine::GetSceneRenderer() {
        return sceneRenderer;
    }

    FileSystem& Engine::GetFileSystem() {
        return fileSystem;
    }

    Scene* Engine::GetScene() const {
        return scene.get();
    }

    MaterialManager& Engine::GetMaterialManager() {
        return materialManager;
    }

    TextureManager& Engine::GetTextureManager() {
        return textureManager;
    }

    PhysicsManager& Engine::GetPhysicsManager() {
        return physicsManager;
    }
} // namespace golias
