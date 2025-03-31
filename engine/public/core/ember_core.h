#pragma once

#include "core/component/camera.h"
#include "core/engine_structs.h"
#include "core/renderer/mesh.h"
#include "core/renderer/shader.h"
#include "core/audio/ember_audio.h"
#include "core/system_info.h"
#include "core/ember_utils.h"

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

    void Resize(int w, int h);

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

static Renderer* renderer = nullptr;


