#pragma once
#include "core/audio/audio_manager.h"
#include "core/input/input_manager.h"
#include "core/io/asset_manager.h"
#include "core/io/file_system.h"
#include "graphics/graphics_device.h"
#include "physics/physics_manager.h"
#include "render/command_queue.h"
#include "render/material.h"
#include "scene/scene.h"
#include "core/window.h"

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
        void SetScene(const Ref<Scene>& scene);

        void SetInputMode(InputMode mode);

        void Quit();

        InputManager& GetInputManager();

        GraphicsDevice& GetGraphicsDevice();

        CommandQueue& GetCommandQueue();

        FileSystem& GetFileSystem();

        AssetManager& GetAssetManager();

        PhysicsManager& GetPhysicsManager();

        AudioManager& GetAudioManager();

    private:
        Engine()                         = default;
        Engine(const Engine&)            = delete;
        Engine& operator=(const Engine&) = delete;
        Engine(Engine&&)                 = delete;
        Engine& operator=(Engine&&)      = delete;

    private:
        Scope<Application> mApplication = nullptr;

        std::chrono::high_resolution_clock::time_point mLastTime;

        Window* mWindow = nullptr;

        InputManager mInputManager;
        GraphicsDevice mGraphicsDevice;
        CommandQueue mCommandQueue;
        FileSystem mFileSystem;
        PhysicsManager mPhysicsManager;
        AudioManager mAudioManager;

        AssetManager mAssetManager;

        Ref<Scene> mScene = nullptr;
    };
} // namespace golias
