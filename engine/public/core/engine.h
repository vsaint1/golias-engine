#pragma once
#include "core/input/input_manager.h"
#include "core/io/file_system.h"
#include "graphics/graphics_device.h"
#include "graphics/texture.h"
#include "physics/physics_manager.h"
#include "render/command_queue.h"
#include "render/material.h"
#include "scene/scene.h"
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

        Scene* GetScene() const;
        void SetScene(Scene* scene);

        Application* GetApplication() const;

        InputManager& GetInputManager();

        GraphicsDevice& GetGraphicsDevice();

        CommandQueue& GetCommandQueue();

        FileSystem& GetFileSystem();

        TextureManager& GetTextureManager();

        MaterialManager& GetMaterialManager();

        PhysicsManager& GetPhysicsManager();

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
        GraphicsDevice mGraphicsDevice;
        CommandQueue mCommandQueue;
        FileSystem mFileSystem;
        PhysicsManager mPhysicsManager;

        TextureManager mTextureManager;
        MaterialManager mMaterialManager;

        Scope<Scene> mScene = nullptr;
    };
} // namespace golias
