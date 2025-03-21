#pragma once

#include "core/file_system.h"
#include <stb_truetype.h>


/*

Type: OpenGL
 > WEB: WebGL 3.0
 > DESKTOP: OpenGL 3.3
 > MOBILE: OpenGLES 3.0

Type: Metal
 > Apple Devices only
*/
enum RendererType { OPENGL, METAL };

typedef struct Renderer {
    SDL_Window* window         = nullptr;
    SDL_GLContext gl_context   = 0;
    unsigned int shaderProgram = 0;
    unsigned int vao = 0, vbo = 0, ebo = 0;
    unsigned int framebuffer = 0;
    int viewport[2]          = {640, 480};
} RendererState;

typedef struct CoreContext {

    struct {
        int width;
        int height;
        const char* title = "";
    } Window;

    struct {
        double current;
        double previous;
        double frame;
        double target;
    } Time;

} Core, CoreData;


typedef struct Texture {
    unsigned int id = 0;
    int width       = 0;
    int height      = 0;
} Texture2D;

struct Glyph {
    float x0, y0, x1, y1;
    int w, h;
    int x_offset, y_offset;
    int advance;
};

typedef struct Font {
    std::map<char, Glyph> glyphs;
    Texture texture;
    int font_size;
    float kerning = 0.0f;
    int ascent, descent, line_gap;
    int scale;
} Font;

struct Rectangle {
    int x;
    int y;
    int width;
    int height;
};

struct FRectangle {
    float x;
    float y;
    float width;
    float height;
};

struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

// OPENGL/ES Renderer
Renderer* CreateRenderer(SDL_Window* window, int view_width, int view_height);

Renderer* GetRenderer();

void DestroyRenderer();

// TODO: METAL


// Initialization
bool InitWindow(const char* title, int width, int height, RendererType type, Uint64 flags = 0);

void SetTargetFPS(int fps);

static Renderer* renderer = nullptr;

static Core core = {0};
