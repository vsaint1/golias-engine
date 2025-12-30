#pragma once
#include "core/graphics/rendering_canvas.h"
#include "core/graphics/rendering_device.h"
#include "core/input/input_manager.h"
#include "core/io/file_system.h"
#include "physics/3d/physics_manager.h"
#include "scene/scene.h"

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

        RenderingDevice* GetRenderingDevice();

        RenderingCanvas& GetRenderingCanvas();

        TextureManager2D& GetTextureManager2D();

        PhysicsManager& GetPhysicsManager();

        FileSystem& GetFileSystem();

        Scene* GetScene() const;
        void SetScene(Scene* pScene);

    private:
        Engine()                         = default;
        Engine(const Engine&)            = delete;
        Engine& operator=(const Engine&) = delete;
        Engine(Engine&&)                 = delete;
        Engine& operator=(Engine&&)      = delete;

    private:
        InputManager input_manager;
        FileSystem file_system;
        PhysicsManager physics_manager;

    private:
        std::unique_ptr<Application> application;
        Uint64 last_time_point = 0;

        SDL_Window* window           = nullptr;
        int _width                   = 1280;
        int _height                  = 720;
        std::unique_ptr<Scene> scene = nullptr;

    private:
        RenderingDevice* rendering_device = nullptr;
        RenderingCanvas rendering_canvas;
        TextureManager2D texture_manager_2d;
    };

}; // namespace golias
