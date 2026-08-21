#include "core/engine.h"

#include "core/application.h"

#define GLFW_INCLUDE_NONE
#include "scene/components/camera_component.h"
#include <glad.h>
#include <glfw/glfw3.h>

namespace golias {

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Engine& engine             = Engine::GetInstance();
        InputManager& inputManager = engine.GetInputManager();

        KeyCode keyCode = inputManager.TranslateKeyCode(key);

        if (action == GLFW_PRESS) {
            inputManager.SetKeyPressed(keyCode, true);
        } else if (action == GLFW_RELEASE) {
            inputManager.SetKeyPressed(keyCode, false);
        } else if (action == GLFW_REPEAT) {
            // TODO: Handle key repeat if needed
        }
    }

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
        Engine& engine             = Engine::GetInstance();
        InputManager& inputManager = engine.GetInputManager();

        MouseButton mouseButton = inputManager.TranslateMouseButton(button);

        if (action == GLFW_PRESS) {
            inputManager.SetMouseButtonPressed(mouseButton, true);
        } else if (action == GLFW_RELEASE) {
            inputManager.SetMouseButtonPressed(mouseButton, false);
        }
    }

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        Engine& engine             = Engine::GetInstance();
        InputManager& inputManager = engine.GetInputManager();

        inputManager.SetScrollOffset(static_cast<float>(xoffset), static_cast<float>(yoffset));
    }

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    }

    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
        Engine& engine             = Engine::GetInstance();
        InputManager& inputManager = engine.GetInputManager();

        inputManager.SetMousePosition(static_cast<float>(xpos), static_cast<float>(ypos));
    }

    bool Engine::Initialize(int width, int height, const String& title) {

        if (!mApplication) {
            GOLIAS_LOG_ERROR("Application is not set. Please set an application before initializing the engine.");
            return false;
        }

        if (!glfwInit()) {
            GOLIAS_LOG_ERROR("Failed to initialize GLFW");
            return false;
        }


        mWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!mWindow) {
            GOLIAS_LOG_ERROR("Failed to create GLFW window");
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(mWindow);

#if defined(GOLIAS_PLATFORM_WINDOWS) || defined(GOLIAS_PLATFORM_LINUX) || defined(GOLIAS_PLATFORM_OSX)


        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            GOLIAS_LOG_ERROR("Failed to initialize GLAD");
            return false;
        }
#else
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

        if (!gladLoadGLES2Loader((GLADloadproc) glfwGetProcAddress)) {
            GOLIAS_LOG_ERROR("Failed to initialize GLAD");
            return false;
        }

#endif

        glfwSetKeyCallback(mWindow, key_callback);
        glfwSetMouseButtonCallback(mWindow, mouse_button_callback);
        glfwSetScrollCallback(mWindow, scroll_callback);
        glfwSetFramebufferSizeCallback(mWindow, framebuffer_size_callback);
        glfwSetCursorPosCallback(mWindow, cursor_position_callback);


        GOLIAS_LOG_INFO("OpenGL version: %s", glGetString(GL_VERSION));

        return mApplication->Initialize();
    }

    void Engine::Run() {
        if (!mApplication) {
            return;
        }

        mLastTime = std::chrono::high_resolution_clock::now();

        while (!mApplication->ShouldClose() && !glfwWindowShouldClose(mWindow)) {

            glfwPollEvents();


            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime  = (currentTime - mLastTime).count();
            mLastTime        = currentTime;

            mApplication->Update(deltaTime);

            mInputManager.ResetTransientState();

            mGraphicsDevice.SetClearColor();
            mGraphicsDevice.ClearBuffers();


            int fbWidth, fbHeight;
            glfwGetFramebufferSize(mWindow, &fbWidth, &fbHeight);

            CameraCommand cameraCommand;
            if (GameObject* camera = Engine::GetInstance().GetScene()->GetMainCamera()) {
                if (CameraComponent* cameraComponent = camera->GetComponent<CameraComponent>()) {

                    cameraComponent->SetAspectRatio(static_cast<float>(fbWidth) / static_cast<float>(fbHeight));

                    cameraCommand.View       = cameraComponent->GetViewMatrix();
                    cameraCommand.Projection = cameraComponent->GetProjectionMatrix();
                    cameraCommand.Viewport   = {.X = 0, .Y = 0, .Width = fbWidth, .Height = fbHeight};

                    mCommandQueue.Submit(cameraCommand);
                }
            }

            mCommandQueue.BeginFrame();
            mCommandQueue.Execute();
            mCommandQueue.EndFrame();

            glfwSwapBuffers(mWindow);
        }
    }

    void Engine::Shutdown() {

        if (mApplication) {
            mApplication->Shutdown();

            glfwDestroyWindow(mWindow);
            glfwTerminate();

            mWindow = nullptr;
        }


        mApplication.reset();
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

    void Engine::SetScene(Scene* scene) {
        mScene.reset(scene);
    }

    Application* Engine::GetApplication() const {
        return mApplication.get();
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


} // namespace golias
