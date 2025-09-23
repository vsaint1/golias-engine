#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <chrono>
#include <stdio.h>

#include <lua.hpp>

#include <flecs.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>
#include <nlohmann/json.hpp>
#include <map>

#include <filesystem>


#if defined(SDL_PLATFORM_EMSCRIPTEN)
#include <emscripten.h>
#endif


using Json = nlohmann::json;

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

#include "definitions.h"

