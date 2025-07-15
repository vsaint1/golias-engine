#pragma once
#include "helpers/logging.h"

class TimeManager {
    Uint64 lastTick    = SDL_GetPerformanceCounter();
    Uint64 currentTick = 0;
    float deltaTime    = 0.0;

    Uint64 frameCount = 0;
    float elapsedTime = 0.0;
    float fps         = 0.0;

    /* MAX_FPS = 1 -> UNLIMITED */
    Uint32 MAX_FPS    = 60;
    bool bIsPaused    = false;
    Uint64 pausedTick = 0;


public:
    TimeManager() {
        lastTick = SDL_GetPerformanceCounter();
    }

    float get_delta_time() {
        if (bIsPaused) {
            return 0.0f;
        }

        return this->deltaTime;
    }

    Uint64 get_frame_count() {
        return this->frameCount;
    }

    float get_elapsed_time() {
        return this->elapsedTime;
    }

    void SetMaxFps(Uint32 fps) {
        this->MAX_FPS = fps;

        LOG_INFO("Target FPS (frames per second) to %02.03f ms", (float) fps * 1000.f);
    }

    float GetFps() {
        return this->fps;
    }

    bool IsPaused() {
        return this->bIsPaused;
    }

    void Pause();

    void Resume();

    void update();

    /* max_fps = 1 -> UNLIMITED */
    void fixed_frame_rate(Uint32 max_fps = 60);
};


