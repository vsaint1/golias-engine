#include "core/engine.h"

#include "core/application.h"

#define GLFW_INCLUDE_NONE
#include <glad.h>
#include <glfw/glfw3.h>


namespace golias {

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Engine& engine             = Engine::GetInstance();
        InputManager& inputManager = engine.GetInputManager();

        if (action == GLFW_PRESS) {
            inputManager.SetKeyPressed(key, true);
        } else if (action == GLFW_RELEASE) {
            inputManager.SetKeyPressed(key, false);
        }
    }

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
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
        glfwSetFramebufferSizeCallback(mWindow, framebuffer_size_callback);

        GOLIAS_LOG_INFO("OpenGL version: %s", glGetString(GL_VERSION));

        return mApplication->Initialize();
    }

    void Engine::Run() {
        if (!mApplication) {
            return;
        }

        mLastTime = std::chrono::high_resolution_clock::now();

        while (!mApplication->ShouldClose() || !glfwWindowShouldClose(mWindow)) {

            glfwPollEvents();

            glClearColor(0.25f, 0.45f, 0.75f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime  = (currentTime - mLastTime).count();
            mLastTime        = currentTime;

            mApplication->Update(deltaTime);

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

    Application* Engine::GetApplication() const {
        return mApplication.get();
    }

    InputManager& Engine::GetInputManager() {
        return mInputManager;
    }


} // namespace golias
