#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <chrono>
#include <stdio.h>

#include <lua.hpp>


#if defined(SDL_PLATFORM_EMSCRIPTEN)
#include <emscripten.h>
#endif
