#pragma once

#include "core/component/camera.h"
#include "core/component/component.h"
#include "core/file_system.h"
#include "core/renderer/mesh.h"
#include "core/renderer/shader.h"


/*!

   @brief Backend `OpenGL` or `Metal`
   - Choose the graphics backend
    Type: OpenGL
    > WEB: WebGL 3.0
    > DESKTOP: OpenGL 3.3
    > MOBILE: OpenGLES 3.0

    Type: Metal
    > Apple Devices only

   @version 0.0.1

*/
enum RendererType { OPENGL, METAL };

/*!
    @brief Renderer struct

    @version 0.0.1
*/
struct Renderer {

    SDL_Window* window = nullptr;

    unsigned int VAO, VBO, EBO;

    Shader default_shader{};

    int viewport[2]   = {480, 270};
    RendererType type = OPENGL;

    void SetContext(const SDL_GLContext& ctx);

    void SetContext(/*MTL*/);

    void Destroy();

private:
    SDL_GLContext context = 0;
};

/*!
    @brief Core struct (engine stuff)

    @version 0.0.1
*/
struct Core {

    struct {
        bool bDevelopmentLogging = true;
    } Metrics;

    struct {
        int width;
        int height;
        const char* title           = "[EMBER_ENGINE] - Window";
        const SDL_DisplayMode* data = nullptr;

    } Window;

    struct {
        SDL_AudioDeviceID device_id = 0;
        SDL_AudioSpec spec;
        float global_volume = 5.0f;
    } Audio;

    struct {
        double current;
        double previous;
        double frame;
        double target;
    } Time;
};


/*!
    @brief Create a renderer instance
    - Backend `OpenGL` or `Metal`

    @param window SDL window instance
    @param view_width Viewport width
    @param view_height Viewport height
    @param type Renderer type `OPENGL` or `METAL`

    @version 0.0.1
    @return Renderer
*/
Renderer* CreateRenderer(SDL_Window* window, int view_width, int view_height, RendererType type);

/*!
    @brief Create a renderer instance internally
    - Backend `OpenGL`

    @param window SDL window instance
    @param view_width Viewport width
    @param view_height Viewport height

    @version 0.0.2
    @return Renderer
*/
Renderer* CreateRendererGL(SDL_Window* window, int view_width, int view_height);

/*!
    @brief Get the renderer instance

    @version 0.0.1
    @return Renderer
*/
Renderer* GetRenderer();

/*!
    @brief Deinitialize Window, Renderer and modules

    @version 0.0.1
    @return void
*/
void CloseWindow();


/*!
    @brief Initialize SDL window, renderer and modules
    - Window
    - Graphics Backend  `OpenGL` or `Metal`
    - Audio
    - Font

    @version 0.0.1
    @param title Window title
    @param width Window width
    @param height Window height
    @param type Renderer type `OPENGL` or `METAL`
    @param flags SDL window flags

    @link https://wiki.libsdl.org/SDL_WindowFlags @endlink

    @return bool - true on `success` or false on `failure`
*/
bool InitWindow(const char* title, int width, int height, RendererType type, Uint64 flags = 0);

/*!

    @brief Set the target frames per second

    @version 0.0.1
    @param fps Target frames per second
    @return void
*/
void SetTargetFPS(int fps);

/*!

    @brief Initialize Sound Engine
    - mini-audio backend

    @version 0.0.3
    @return bool
*/
bool InitAudio();

/*!

    @brief Close Sound Engine and free allocated resources
    

    @version 0.0.3
    @return void
*/
void CloseAudio();


/*!

    @brief Load Music and keep track of it

    @param file_Path Path to the music file
    
    @version 0.0.3
    @return Music allocated music memory struct
*/
Music* Mix_LoadMusic(const std::string& file_Path);

/*!

    @brief Play music

    @param Music Valid music pointer
    @param loop Play music in loop

    @version 0.0.3
    @return void
*/
void Mix_PlayMusic(Music* music, bool loop = false);


/*!

    @brief Pause music and fade out

    @param Music Valid music pointer

    @version 0.0.3
    @return void
*/
void Mix_PauseMusic(Music* music);

/*!

    @brief Unload Music allocated memory
    - Manually clear allocated music or at the end will be freed automatically 

    @see CloseAudio
    
    @param Music Valid music pointer

    @version 0.0.3
    @return void
*/
void Mix_UnloadMusic(Music* music);

// TODO: create a Resource Manager
inline std::unordered_map<std::string, Music*> musics;

static Renderer* renderer = nullptr;

extern ma_engine engine;

static Core core;
