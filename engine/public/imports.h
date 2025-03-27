#pragma once
#include <SDL3/SDL.h>
#include <cmath>
#include <string>
#include <filesystem>
#include <map>
#include <chrono>

/* ENABLE MATH CONSTANTS*/
#define _USE_MATH_DEFINES 1
#include <math.h>

#include <glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>


#if __ANDROID__
const std::filesystem::path BASE_PATH = "";
#define ASSETS_PATH std::string("")
#elif __APPLE__
const std::filesystem::path BASE_PATH = SDL_GetBasePath();
#define ASSETS_PATH (BASE_PATH / "assets/").string()
#else
const std::filesystem::path BASE_PATH = SDL_GetBasePath();
#define ASSETS_PATH std::string("assets/")
#endif

#define ENGINE_NAME        "EMBER_ENGINE"
#define ENGINE_VERSION_STR "0.0.1"

