#pragma once

#include "core/file_system.h"

#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
#define SHADER_HEADER "#version 300 es\nprecision mediump float;\n"
#else
#define SHADER_HEADER "#version 330 core\n"
#endif

typedef struct Renderer {
    SDL_Window* window = nullptr;
    SDL_GLContext gl_context = 0;
    unsigned int shaderProgram = 0;
    unsigned int vao = 0, vbo = 0, ebo = 0;
    unsigned int framebuffer = 0;
    int viewport[2] = {640,480};
} RendererState;

static Renderer* renderer = nullptr;

typedef struct Texture {
    unsigned int id = 0;
    int width = 0;
    int height = 0;
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

Renderer* CreateRenderer(SDL_Window* window, SDL_GLContext  gl_context,int view_width, int view_height);

Renderer* GetRenderer();

void DestroyRenderer();

unsigned int CompileShader(unsigned int type, const char* src);

unsigned int CreateShaderProgram();

Texture LoadTexture(const std::string& file_path);

void DrawTexture(Texture tex, Rectangle rect, Color color= {255, 255, 255, 255}) ;

void DrawTextureEx(Texture texture, Rectangle source, Rectangle dest, glm::vec2 origin, float rotation, Color color = {255, 255, 255, 255});

void UnloadTexture(Texture texture);

void ClearBackground(Color color);

void BeginDrawing();

void EndDrawing();