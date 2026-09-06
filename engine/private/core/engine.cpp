#include "core/engine.h"

#include "core/application.h"
#include "core/wsi/glfw_window.h"
#include "scene/components/camera_component.h"

#if defined(GOLIAS_PLATFORM_EMSCRIPTEN)
    #include <emscripten/emscripten.h>
#endif

namespace golias {

    bool Engine::Initialize(int width, int height, const String& title) {

        if (!mApplication) {
            GOLIAS_LOG_ERROR("Application is not set. Please set an application before initializing the engine.");
            return false;
        }

        Scene::RegisterTypes();
        mApplication->RegisterTypes();

        mWindow = new GlfwWindow(width, height, title.c_str());

        if (!mWindow) {
            GOLIAS_LOG_ERROR("Failed to create Window");
            return false;
        }

        if (!mGraphicsDevice.Initialize()) {
            GOLIAS_LOG_ERROR("Failed to initialize GraphicsDevice");
            return false;
        }

        if (!mCommandQueue.Initialize()) {
            GOLIAS_LOG_ERROR("Failed to initialize CommandQueue");
            return false;
        }

        if (!mPhysicsManager.Initialize()) {
            GOLIAS_LOG_ERROR("Failed to initialize PhysicsManager");
            return false;
        }

        if (!mAudioManager.Initialize()) {
            GOLIAS_LOG_ERROR("Failed to initialize AudioManager");
            return false;
        }

        mGraphicsDevice.SetViewport({0, 0, width, height});

        mWindow->OnKey = [this](KeyCode key, KeyAction action, int /*mods*/) {
            if (action == KeyAction::Repeat) {
                return;
            }

            mInputManager.SetKeyPressed(key, action == KeyAction::Press);
        };

        mWindow->OnMouseButton = [this](MouseButton button, bool pressed, int /*mods*/) {
            mInputManager.SetMouseButtonPressed(button, pressed);
        };

        mWindow->OnCursorPos = [this](double xpos, double ypos) {
            mInputManager.SetMousePosition(static_cast<float>(xpos), static_cast<float>(ypos));
        };

        mWindow->OnScroll = [this](double xoffset, double yoffset) {
            mInputManager.SetScrollOffset(static_cast<float>(xoffset), static_cast<float>(yoffset));
        };

        if (!mApplication->Initialize()) {
            GOLIAS_LOG_ERROR("Failed to initialize the application");
            return false;
        }

        return true;
    }

    void engine_core_loop() {

        Engine& engine = Engine::GetInstance();

        Time::Start();

        static Clock::time_point lastMemorySample = Clock::now();
        engine.mMemoryStats                       = GetMemoryStats();

        while (!engine.mWindow->ShouldClose()) {

            Time::Tick();

            engine.mWindow->PollEvents();

            const float deltaTime = Time::GetDeltaTime();

            const Clock::time_point cpuStart = Clock::now();

            engine.mApplication->Update(deltaTime);

            engine.mInputManager.ResetTransientState();

            engine.mPhysicsManager.Update(deltaTime);

            int width, height;
            engine.mWindow->GetDrawableSize(&width, &height);

            CameraCommand cameraCommand;
            if (GameObject* camera = Engine::GetInstance().GetScene()->GetMainCamera()) {
                if (CameraComponent* cameraComponent = camera->GetComponent<CameraComponent>()) {

                    if (width <= 0 || height <= 0) {
                        continue;
                    }

                    cameraComponent->SetAspectRatio(static_cast<float>(width) / static_cast<float>(height));

                    cameraCommand.View           = cameraComponent->GetViewMatrix();
                    cameraCommand.Projection     = cameraComponent->GetProjectionMatrix();
                    cameraCommand.CameraPosition = camera->GetWorldPosition();
                    cameraCommand.Ortho          = cameraComponent->GetOrthoMatrix(width, height);
                    cameraCommand.NearPlane      = cameraComponent->GetNearPlane();
                    cameraCommand.FarPlane       = cameraComponent->GetFarPlane();
                    cameraCommand.Viewport       = {.X = 0, .Y = 0, .Width = width, .Height = height};

                    engine.mCommandQueue.Submit(cameraCommand);
                }
            }

            engine.mCommandQueue.BeginFrame();
            engine.mCommandQueue.Execute();
            engine.mCommandQueue.EndFrame();

            const Clock::time_point cpuEnd = Clock::now();

            engine.mWindow->SwapBuffers();

            const Clock::time_point frameEnd = Clock::now();

            const float frameMs = deltaTime * 1000.0f;
            const float cpuMs   = std::chrono::duration<float, std::milli>(cpuEnd - cpuStart).count();

            FrameStats::RecordFrame(frameMs, cpuMs);
            FrameStats::NextFrame();

            static constexpr std::chrono::duration<float, std::milli> kMemoryInterval = std::chrono::duration<float, std::milli>(500.0f);
            if (frameEnd - lastMemorySample >= kMemoryInterval) {
                engine.mMemoryStats = GetMemoryStats();
                lastMemorySample    = frameEnd;
            }
        }
    }

    void Engine::Run() {
        if (!mApplication || !mWindow) {
            return;
        }

#if defined(GOLIAS_PLATFORM_EMSCRIPTEN)
        emscripten_set_main_loop(engine_core_loop, 0, true);
#else
        engine_core_loop();
#endif
    }

    void Engine::Shutdown() {

        if (mApplication) {
            mApplication->Shutdown();

            delete mWindow;
            mWindow = nullptr;
        }


        mApplication.reset();
    }

    void Engine::SetInputMode(InputMode mode) {
        if (mWindow) {
            mWindow->SetInputMode(mode);
        }
    }

    InputMode Engine::GetInputMode() const {
        if (mWindow) {
            return mWindow->GetInputMode();
        }

        return InputMode::Cursor;
    }

    void Engine::SetApplication(Application* app) {
        mApplication.reset(app);
    }

    Engine& Engine::GetInstance() {
        static Engine instance;
        return instance;
    }

    Scene* Engine::GetScene() const {
        return mScene.get();
    }

    void Engine::SetScene(const Ref<Scene>& scene) {
        mScene = scene;
    }

    void Engine::Quit() {
        if (mWindow) {
            mWindow->Close();
        }
    }

    InputManager& Engine::GetInputManager() {
        return mInputManager;
    }

    GraphicsDevice& Engine::GetGraphicsDevice() {
        return mGraphicsDevice;
    }

    CommandQueue& Engine::GetCommandQueue() {
        return mCommandQueue;
    }

    FileSystem& Engine::GetFileSystem() {
        return mFileSystem;
    }

    AssetManager& Engine::GetAssetManager() {
        return mAssetManager;
    }

    PhysicsManager& Engine::GetPhysicsManager() {
        return mPhysicsManager;
    }

    AudioManager& Engine::GetAudioManager() {
        return mAudioManager;
    }

    const RenderStats& Engine::GetRenderStats() const {
        return FrameStats::Get();
    }

    const MemoryStats& Engine::GetMemoryStats() const {
        return mMemoryStats;
    }

} // namespace golias
