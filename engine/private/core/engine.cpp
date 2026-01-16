#include "core/engine.h"

#include "scene/3d/camera_component.h"

#if defined(SDL_PLATFORM_EMSCRIPTEN)
    #include <emscripten/emscripten.h>
#endif

namespace golias {

    Engine& Engine::GetInstance() {
        static Engine instance;
        return instance;
    }

    bool Engine::Initialize() {

        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK)) {
            spdlog::error("Engine::Initialize Failed to initialize SDL : {}", SDL_GetError());
            return false;
        }


        if (!fontManager.Initialize()) {
            spdlog::error("Engine::Initialize Failed to initialize Font Manager.");
            return false;
        }


        fontManager.SetFallbackFonts({
            "fonts/NotoSans.ttf",
            "fonts/NotoSansCJK.otf",
            // "fonts/Twemoji.ttf" // needs plutosvg
        });


        Scene::RegisterTypes();

        if (!audioManager.Initialize()) {
            spdlog::error("Engine::Initialize Failed to initialize Audio Manager.");
            return false;
        }

        // TODO: make possible to swap/recrate the application
        if (application) {


            if (!sceneRenderer.Initialize(application->GetWindowNativeHandle(), application->GetRenderingDeviceType())) {
                spdlog::error("Engine::Initialize Failed to initialize Rendering Canvas.");
                return false;
            }

            sceneRenderer.GetRenderingDevice()->SetViewport({0, 0, application->GetWidth(), application->GetHeight()});


            if (!physicsManager.Initialize(sceneRenderer.GetRenderingDevice()->GetPhysicsDebugDrawer())) {
                spdlog::error("Engine::Initialize Failed to initialize Physics Manager.");
                return false;
            }

            application->RegisterTypes();

            if (!application->Initialize()) {
                spdlog::error("Engine::Initialize Failed to initialize the Application.");
                return false;
            }
        } else {
            spdlog::error("Engine::Initialize No Application instance set before initialization.");
            return false;
        }

        spdlog::info("Engine::Initialize Golias Engine Initialized successfully.");

        return true;
    }

    void engine_core_loop() {

        Engine& engine = Engine::GetInstance();

        Uint64 current_time_point = SDL_GetPerformanceCounter();
        Uint64 time_delta         = current_time_point - engine.GetLastTimePoint();

        float delta_time = static_cast<float>(time_delta) / SDL_GetPerformanceFrequency();

        engine.SetLastTimePoint(current_time_point);

        engine.GetInputManager().Update();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                engine.GetApplication()->Close();
            }

            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                engine.GetApplication()->SetWidth(event.window.data1);
                engine.GetApplication()->SetHeight(event.window.data2);
                engine.GetSceneRenderer().GetRenderingDevice()->SetViewport(
                    {0, 0, engine.GetApplication()->GetWidth(), engine.GetApplication()->GetHeight()});
            }

            engine.GetInputManager().ProcessEvent(event);
        }

        if (engine.GetCanvasInputManager().IsActive()) {
            engine.GetCanvasInputManager().Update(delta_time);
        }

        engine.GetPhysicsManager().StepSimulation(delta_time);

        engine.GetApplication()->Update(delta_time);


        CameraCommand cameraData;
        if (engine.GetScene() && engine.GetScene()->GetMainCamera()) {
            auto pCameraComponent = engine.GetScene()->GetMainCamera()->GetComponent<CameraComponent>();

            if (pCameraComponent) {
                cameraData.viewMatrix = pCameraComponent->GetViewMatrix();

                float aspect =
                    static_cast<float>(engine.GetApplication()->GetWidth()) / static_cast<float>(engine.GetApplication()->GetHeight());
                cameraData.projectionMatrix   = pCameraComponent->GetProjectionMatrix(aspect);
                cameraData.orthographicMatrix = glm::ortho(0.0f,
                                                           static_cast<float>(engine.GetApplication()->GetWidth()),
                                                           0.0f,
                                                           static_cast<float>(engine.GetApplication()->GetHeight()));

                cameraData.position = engine.GetScene()->GetMainCamera()->GetWorldPosition();
            }
        }

        engine.GetSceneRenderer().BeginFrame();
        engine.GetSceneRenderer().Draw(cameraData);

        if (engine.GetPhysicsManager().IsDebugDrawEnabled()) {
            engine.GetPhysicsManager().RenderDebug(cameraData.projectionMatrix * cameraData.viewMatrix);
        }

        engine.GetSceneRenderer().EndFrame();

        engine.GetSceneRenderer().Present();
    }

    void Engine::Run() {

        if (!application) {
            return;
        }

        last_time_point = SDL_GetPerformanceCounter();


#if defined(SDL_PLATFORM_EMSCRIPTEN)
        emscripten_set_main_loop(engine_core_loop, 0, 1);
#else
        while (!application->ShouldClose()) {

            engine_core_loop();
        }
#endif
        application.reset(nullptr);
    }

    void Engine::Destroy() {

        if (application) {
            application->Destroy();
            application.reset(nullptr);
        }

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


    Uint64 Engine::GetLastTimePoint() const {
        return last_time_point;
    }
    void Engine::SetLastTimePoint(Uint64 timePoint) {
        last_time_point = timePoint;
    }


    void Engine::SetScene(const std::shared_ptr<Scene>& pScene) {
        scene = pScene;
    }

    Application* Engine::GetApplication() const {
        return application.get();
    }

    FontManager& Engine::GetFontManager() {
        return fontManager;
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

    CanvasInputManager& Engine::GetCanvasInputManager() {
        return canvasInputManager;
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
