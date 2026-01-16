#pragma once
#include "core/graphics/scene_renderer.h"
#include "core/graphics/rendering_device.h"
#include "core/input/input_manager.h"
#include "core/io/file_system.h"
#include "physics/3d/physics_manager.h"
#include "scene/scene.h"
#include "audio/audio_manager.h"
#include "font/font_manager.h"
#include "core/graphics/texture_manager.h"
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
        TextureManager& GetTextureManager();
        PhysicsManager& GetPhysicsManager();
        FileSystem& GetFileSystem();
        MaterialManager& GetMaterialManager();
        AudioManager& GetAudioManager();
        FontManager& GetFontManager();
        CanvasInputManager& GetCanvasInputManager();

        Scene* GetScene() const;
        void SetScene(const std::shared_ptr<Scene>& pScene);

        Uint64 GetLastTimePoint() const ;
        void SetLastTimePoint(Uint64 timePoint);

    private:
        Engine()                         = default;
        Engine(const Engine&)            = delete;
        Engine& operator=(const Engine&) = delete;
        Engine(Engine&&)                 = delete;
        Engine& operator=(Engine&&)      = delete;
     
    private:
        std::unique_ptr<Application> application;
        Uint64 last_time_point = 0;
 
        std::shared_ptr<Scene> scene = nullptr;
    private:
        SceneRenderer sceneRenderer;
        TextureManager textureManager;
        InputManager inputManager;
        FileSystem fileSystem;
        PhysicsManager physicsManager;
        MaterialManager materialManager;
        AudioManager audioManager;
        FontManager fontManager;
        CanvasInputManager canvasInputManager;
    };
}; // namespace golias