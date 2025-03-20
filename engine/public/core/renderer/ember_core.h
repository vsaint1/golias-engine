#pragma once

#include "core/file_system.h"

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

static Core core = {0};

typedef struct Texture {
    unsigned int id = 0;
    int width       = 0;
    int height      = 0;
} Texture2D;

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

// OPENGL/ES Renderer
Renderer* CreateRenderer(SDL_Window* window, int view_width, int view_height);

Renderer* GetRenderer();

void DestroyRenderer();

// TODO: METAL


// Initialization
void InitWindow(const char* title, int width, int height, Uint64 flags);

void SetTargetFPS(int fps);

static Renderer* renderer = nullptr;
