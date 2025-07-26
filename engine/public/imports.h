#pragma once
#include <SDL3/SDL.h>
#include <cmath>
#include <string>
#include <filesystem>
#include <chrono>
#include <queue>

#if !defined(NDEBUG)
    #include <map>
    template <typename K, typename V>
    using HashMap = std::map<K, V>;
#else
#include <unordered_map>
template <typename K, typename V>
using HashMap = std::unordered_map<K, V>;
#endif

#include <sstream>

/* ENABLE MATH CONSTANTS*/
#define _USE_MATH_DEFINES 1
#include <array>
#include <math.h>

#include <glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <miniaudio.h>
#include <tinyxml2.h>

#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"

#include <mutex>

#include <condition_variable>
#include <atomic>

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
#define ENGINE_VERSION_STR "0.0.9"



