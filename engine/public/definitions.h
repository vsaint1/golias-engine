#pragma once
#include <SDL3/SDL.h>
#include  <filesystem>
#include  <string>



#if __ANDROID__
const std::filesystem::path BASE_PATH = "";
#define ASSETS_PATH std::string("")
#elif __APPLE__
const std::filesystem::path BASE_PATH = SDL_GetBasePath();
#define ASSETS_PATH (BASE_PATH / "res/").string()
#else
const std::filesystem::path BASE_PATH = SDL_GetBasePath();
#define ASSETS_PATH std::string("res/")
#endif

#define ENGINE_NAME        "EMBER_ENGINE"
#define ENGINE_VERSION_STR "1.2.0"
#undef WITH_EDITOR
