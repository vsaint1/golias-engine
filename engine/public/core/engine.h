#pragma once

#include "core/file_system.h"
#include "core/input/input_manager.h"
#include "core/time_manager.h"


struct Core {

    struct {
        bool bDevelopmentLogging = true;
    } Metrics;

    struct {
        int width                   = 0;
        int height                  = 0;
        const char* title           = "[EMBER_ENGINE] - Window";
        const SDL_DisplayMode* data = nullptr;
        bool bFullscreen            = false;
        int bbWidth = 0, bbHeight = 0; // backbuffer
        SDL_Window* window;
    } Window;

    struct {
        SDL_AudioDeviceID device_id = 0;
        SDL_AudioSpec spec;
        float global_volume = 5.0f;
    } Audio;

    InputManager* Input = nullptr;

    Ember_VFS ember_vfs;

    TimeManager* Time = nullptr;

    void Resize(int w, int h) const;
};

extern ma_engine engine;

extern Core core;
