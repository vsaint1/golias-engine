#pragma once

#include "ember_core.h"

#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
#define SHADER_HEADER "#version 300 es\nprecision mediump float;\n"
#else
#define SHADER_HEADER "#version 330 core\n"
#endif


unsigned int CompileShader(unsigned int type, const char* src);

unsigned int CreateShaderProgram();

Texture LoadTexture(const std::string& file_path);

void DrawTexture(Texture tex, Rectangle rect, Color color= {255, 255, 255, 255}) ;

void DrawTextureEx(Texture texture, Rectangle source, Rectangle dest, glm::vec2 origin, float rotation, Color color = {255, 255, 255, 255});

void UnloadTexture(Texture texture);

void ClearBackground(Color color);

void BeginDrawing();

void EndDrawing();