#pragma once
#define NOMINMAX
#include <SDL3/SDL.h>
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
#include <flecs/flecs.h>

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>
#include <map>
#include <deque>
#include <condition_variable>
#include <random>

#include <glad.h>

#include "json.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>


#include <filesystem>

#include <miniaudio.h>

#if defined(SDL_PLATFORM_EMSCRIPTEN)
#include <emscripten.h>
#endif

#include <tinyxml2.h>


#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/android_sink.h>

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

#include "nuklear_include.h"

#include <stb_image.h>
