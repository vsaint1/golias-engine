#pragma once
#include "core/audio/audio_manager.h"
#include "core/input/input_manager.h"
#include "core/io/asset_manager.h"
#include "core/io/file_system.h"
#include "core/platform/memory.h"
#include "core/time.h"
#include "core/window.h"
#include "graphics/graphics_device.h"
#include "physics/physics_manager.h"
#include "render/command_queue.h"
#include "render/material.h"
#include "render/render_stats.h"
#include "scene/scene.h"

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

        InputMode GetInputMode() const;
        void SetInputMode(InputMode mode);

        void Quit();

        InputManager& GetInputManager();

        GraphicsDevice& GetGraphicsDevice();

        CommandQueue& GetCommandQueue();

        FileSystem& GetFileSystem();

        AssetManager& GetAssetManager();

        PhysicsManager& GetPhysicsManager();

        AudioManager& GetAudioManager();

        const RenderStats& GetRenderStats() const;

        const MemoryStats& GetMemoryStats() const;

    private:
        Engine()                         = default;
        Engine(const Engine&)            = delete;
        Engine& operator=(const Engine&) = delete;
        Engine(Engine&&)                 = delete;
        Engine& operator=(Engine&&)      = delete;

        friend void engine_core_loop();

    private:
        Scope<Application> mApplication = nullptr;

        Window* mWindow = nullptr;

        InputManager mInputManager;
        GraphicsDevice mGraphicsDevice;
        CommandQueue mCommandQueue;
        FileSystem mFileSystem;
        PhysicsManager mPhysicsManager;
        AudioManager mAudioManager;

        AssetManager mAssetManager;

        MemoryStats mMemoryStats = {};

        Ref<Scene> mScene = nullptr;

    };
} // namespace golias
