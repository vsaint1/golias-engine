#include "core/engine.h"

#include "core/application.h"
#include "core/wsi/glfw_window.h"
#include "scene/components/camera_component.h"

namespace golias {

    bool Engine::Initialize(int width, int height, const String& title) {

        if (!mApplication) {
            GOLIAS_LOG_ERROR("Application is not set. Please set an application before initializing the engine.");
            return false;
        }

        Scene::RegisterTypes();
        mApplication->RegisterTypes();

        if (!glfwInit()) {
            GOLIAS_LOG_ERROR("Failed to initialize GLFW");
            return false;
        }


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

        mWindow->SetInputMode(InputMode::Disabled);

        if (!mApplication->Initialize()) {
            GOLIAS_LOG_ERROR("Failed to initialize the application");
            return false;
        }

        return true;
    }

    void Engine::Run() {
        if (!mApplication || !mWindow) {
            return;
        }

        mLastTime = std::chrono::high_resolution_clock::now();

        while (!mWindow->ShouldClose()) {

            mWindow->PollEvents();


            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime  = std::chrono::duration<float>(currentTime - mLastTime).count();
            mLastTime        = currentTime;

            mApplication->Update(deltaTime);

            mInputManager.ResetTransientState();

            mPhysicsManager.Update(deltaTime);

            mGraphicsDevice.SetClearColor();
            mGraphicsDevice.ClearBuffers();

            int width, height;
            mWindow->GetDrawableSize(&width, &height);

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

                    mCommandQueue.Submit(cameraCommand);
                }
            }

            mCommandQueue.BeginFrame();
            mCommandQueue.Execute();
            mCommandQueue.EndFrame();

            mWindow->SwapBuffers();
        }
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

} // namespace golias
