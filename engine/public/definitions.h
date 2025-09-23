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


#if defined(SDL_PLATFORM_EMSCRIPTEN)
#include <emscripten.h>
#endif


using Json = nlohmann::json;

/// Required SDL_main for portability 
#include <SDL3/SDL_main.h>