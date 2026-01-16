#pragma once
#include <SDL3/SDL_video.h>
#include <string>

namespace golias {


    enum ERenderingDeviceType {
        COMPATIBILITY,
        FORWARD_PLUS,
    };

    class Application {
    public:
        Application(const char* pTitle, int width, int height, ERenderingDeviceType deviceType);

        ~Application();

        virtual void RegisterTypes();
        virtual bool Initialize()            = 0;
        virtual void Update(float deltaTime) = 0;
        virtual void Destroy()               = 0;

        bool ShouldClose() const;

        SDL_Window* GetWindowNativeHandle() const ;

        void SetTitle(const std::string_view pTitle);
        const std::string& GetTitle() const;

        int GetWidth() const;
        int GetHeight() const;

        void SetWidth(int w);
        void SetHeight(int h);

        ERenderingDeviceType GetRenderingDeviceType() const;

        void Close();

    protected:
        bool is_running = true;

        int width  = 1280;
        int height = 720;

        SDL_Window* window = nullptr;

        std::string title = "Golias Engine - Application";

        ERenderingDeviceType renderingDeviceType = ERenderingDeviceType::COMPATIBILITY;
    };


}; // namespace golias
