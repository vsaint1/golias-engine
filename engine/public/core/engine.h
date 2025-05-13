#pragma once

#include "core/input/input_manager.h"
#include "core/io/file_system.h"
#include "core/time_manager.h"


/*!

   @brief Backend `OpenGL` or `Metal`
   - Choose the graphics backend
    Type: OpenGL
    > WEB: WebGL 3.0
 >

 *
 * * DESKTOP: OpenGL 3.3
    > MOBILE: OpenGLES 3.0

    Type: Metal
    > Apple Devices only

   @version 0.0.1

*/
enum RendererType { OPENGL, METAL };

class Renderer;
class OpenglShader;
class OpenglRenderer;

class Engine {

    public:

    struct {
        int width                   = 0;
        int height                  = 0;
        const char* title           = "[EMBER_ENGINE] - Window";
        const SDL_DisplayMode* data = nullptr;
        bool bFullscreen            = false;
        int bbWidth = 0, bbHeight = 0; // backbuffer
        SDL_Window* window{};
    } Window;

    struct {
        SDL_AudioDeviceID device_id = 0;
        SDL_AudioSpec spec{};
        float global_volume = 1.f;
    } Audio;

    Ember_VFS ember_vfs{};

    void ResizeWindow(int w, int h) const;

    Renderer* GetRenderer() const;

    InputManager* GetInputManager() const;

    TimeManager* GetTimeManager() const;

    /*!
    @brief Deinitialize Window, Renderer and modules

    @version 0.0.1
    @return void
*/
    void Shutdown();


    /*!
        @brief Initialize SDL window, renderer and modules
        - Window
        - Graphics Backend  `OpenGL`
     * or `Metal`
     -

     * * Audio
        - Font

        @version 0.0.1
        @param title Window title

     * @param width Window width
        @param height

     * * Window height
        @param type Renderer type
     * `OPENGL` or `METAL`
        @param flags SDL window flags

        @link
     *
     *
     * https://wiki.libsdl.org/SDL_WindowFlags @endlink

        @return bool - true on `success` or false on `failure`

     */
    bool Initialize(const char* title, int width, int height, RendererType type, Uint64 flags = 0);


    /*!
    @brief Create a renderer instance
    - Backend `OpenGL` or `Metal`

    @param window SDL window instance


     * *
 * @param view_width Viewport width
    @param view_height Viewport height
    @param type Renderer type
     * `OPENGL` or
 *
 * `METAL`

    @version 0.0.1
    @return Renderer
*/
    Renderer* CreateRenderer(SDL_Window* window, int view_width, int view_height, RendererType type);

private:
    Renderer* renderer_ = nullptr;
    InputManager* input_manager_ = nullptr;
    TimeManager* time_manager_   = nullptr;

    /*!
        @brief Create a renderer instance internally
        - Backend `OpenGL`

        @param window SDL
     * window instance

     *
     * @param view_width Viewport width
        @param view_height Viewport height


     * @version 0.0.2
        @return Renderer
    */
    Renderer* CreateRendererGL(SDL_Window* window, int view_width, int view_height);

    /*
        @brief Create a renderer instance internally
        - Backend `Metal`
        TODO: implement metal api

     */
    Renderer* CreateRendererMTL(SDL_Window* window, int view_width, int view_height);
};


extern std::unique_ptr<Engine> GEngine;

extern ma_engine audio_engine;
