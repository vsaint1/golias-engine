#pragma once
#include "core/graphics/scene_renderer.h"
#include "core/graphics/rendering_device.h"
#include "core/input/input_manager.h"
#include "core/time.h"
#include "core/io/file_system.h"
#include "physics/3d/physics_manager.h"
#include "scene/scene.h"
#include "audio/audio_manager.h"
#include "core/asset_manager.h"
#include "core/input/canvas_input_manager.h"
#include "core/input/cursor.h"


namespace golias {
    class Application;
    
    class Engine {
    public:
        ~Engine() = default;
        static Engine& GetInstance();
        
        bool Initialize();
        void Run();
        void Destroy();
        void SetApplication(Application* pApplication);
        
        Application* GetApplication() const;
        InputManager& GetInputManager();
        SceneRenderer& GetSceneRenderer();
        AssetManager& GetAssetManager();
        PhysicsManager& GetPhysicsManager();
        FileSystem& GetFileSystem();
        AudioManager& GetAudioManager();
        CanvasInputManager& GetCanvasInputManager();

        Scene* GetScene() const;
        void SetScene(const std::shared_ptr<Scene>& pScene);

    private:
        Engine()                         = default;
        Engine(const Engine&)            = delete;
        Engine& operator=(const Engine&) = delete;
        Engine(Engine&&)                 = delete;
        Engine& operator=(Engine&&)      = delete;
     
    private:
        std::unique_ptr<Application> application;
        std::shared_ptr<Scene> scene = nullptr;
    private:
        SceneRenderer sceneRenderer;
        AssetManager assetManager;
        InputManager inputManager;
        FileSystem fileSystem;
        PhysicsManager physicsManager;
        AudioManager audioManager;
        CanvasInputManager canvasInputManager;
    };
}; // namespace golias