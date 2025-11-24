#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "definitions.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <chrono>
#include <stdio.h>

// #include <lua.hpp>
#include <sol.hpp>

#define FLECS_CUSTOM_BUILD
#define FLECS_SYSTEM
#define FLECS_NO_LOG
#define FLECS_META
#define FLECS_CPP
#define FLECS_PIPELINE
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <condition_variable>
#include <deque>
#include <flecs/flecs.h>
#include <functional>
#include <glad.h>
#include <map>
#include <mutex>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "json.hpp"
#include <assimp/Importer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <filesystem>
#include <miniaudio.h>

#include <glm/gtx/string_cast.hpp>

#if defined(SDL_PLATFORM_EMSCRIPTEN)
    #include <emscripten.h>
#endif

#include <spdlog/sinks/android_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

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

#include "nuklear.h"
#include "nuklear_sdl3_ogl3.h"
#include <stb_image.h>