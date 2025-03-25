#pragma once

#include "core/file_system.h"
#include "core/renderer/shader.h"
#include "core/component/Camera.h"

#include <stb_truetype.h>


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

struct Renderer {
    SDL_Window* window = nullptr;

    struct {
        SDL_GLContext context   = 0;
        unsigned int vao = 0, vbo = 0;
        Shader default_shader{};
        int viewport[2]          = {480, 270};
    } OpenGL;

    struct {

    } Metal;

    RendererType type = OPENGL;
};

struct Core {

    struct {
        int width;
        int height;
        const char* title = "[EMBER_ENGINE] - Window";
        const SDL_DisplayMode*  data = nullptr;
    } Window;

    struct {
        double current;
        double previous;
        double frame;
        double target;
    } Time;
};

struct Vertex {
    glm::vec3 position; 
    glm::vec2 texCoord; 
};

typedef struct Texture {
    unsigned int id = 0;
    int width       = 0;
    int height      = 0;
} Texture, Texture2D;

struct Glyph {
    float x0, y0, x1, y1;
    int w, h;
    int x_offset, y_offset;
    int advance;
};

struct Font {
    std::map<char, Glyph> glyphs;
    Texture texture;
    int font_size;
    float kerning = 0.0f;
    int ascent, descent, line_gap;
    int scale;

    bool IsValid()
    {
        return texture.id != 0 && glyphs.size() > 0;
    }
    
};

struct Rectangle {
    int x;
    int y;
    int width;
    int height;
};

struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
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
Renderer* CreateRenderer(SDL_Window* window, int view_width, int view_height,RendererType type);

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

static Renderer* renderer = nullptr;

static Core core = {0};
