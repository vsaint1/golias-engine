#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX


#if defined(SDL_PLATFORM_EMSCRIPTEN)
    #include <emscripten.h>
#endif

#include <filesystem>

