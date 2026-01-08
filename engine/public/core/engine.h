#pragma once
#include "core/graphics/scene_renderer.h"
#include "core/graphics/rendering_device.h"
#include "core/input/input_manager.h"
#include "core/io/file_system.h"
#include "physics/3d/physics_manager.h"
#include "scene/scene.h"
#include "audio/audio_manager.h"
#include "font/font_manager.h"

#if defined(SDL_PLATFORM_EMSCRIPTEN)
void engine_core_loop();
#endif

namespace golias {
    class Application;
    class Engine {
    public:
        ~Engine() = default;
        static Engine& GetInstance();
        bool Initialize(const char* pTitle              = "Golias Engine",
                        int w                           = 1280,
                        int h                           = 720,
                        ERenderingDeviceType deviceType = ERenderingDeviceType::COMPATIBILITY);
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
        
        Scene* GetScene() const;
        void SetScene(const std::shared_ptr<Scene>& pScene);
        
        int GetWidth() const { return _width; }
        int GetHeight() const { return _height; }
        void SetWidth(int width) { _width = width; }
        void SetHeight(int height) { _height = height; }
        Uint64 GetLastTimePoint() const { return last_time_point; }
        void SetLastTimePoint(Uint64 timePoint) { last_time_point = timePoint; }
#if defined(SDL_PLATFORM_EMSCRIPTEN)
        friend void ::engine_core_loop();
#endif

    private:
        Engine()                         = default;
        Engine(const Engine&)            = delete;
        Engine& operator=(const Engine&) = delete;
        Engine(Engine&&)                 = delete;
        Engine& operator=(Engine&&)      = delete;
     
    private:
        std::unique_ptr<Application> application;
        Uint64 last_time_point = 0;
        SDL_Window* window           = nullptr;
        int _width                   = 1280;
        int _height                  = 720;
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
    };
}; // namespace golias