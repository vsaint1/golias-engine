#pragma once
#include "helpers/logging.h"

class TimeManager {


public:
    TimeManager() {
        _last_tick = SDL_GetPerformanceCounter();
    }

    [[nodiscard]] float get_delta_time() const {
        if (_bPaused) {
            return 0.0f;
        }

        return this->_delta_time;
    }

    [[nodiscard]] Uint64 get_frame_count() const {
        return this->_frame_count;
    }

    [[nodiscard]] float get_elapsed_time() const {
        return this->_elapsed_time;
    }

    void SetMaxFps(Uint32 fps) {
        this->MAX_FPS = fps;

        LOG_INFO("Target FPS (frames per second) to %02.03f ms", (float) fps * 1000.f);
    }

    [[nodiscard]] float get_fps() const {
        return this->fps;
    }

    [[nodiscard]] bool is_paused() const {
        return this->_bPaused;
    }

    void pause();

    void resume();

    void update();

    /* max_fps = 1 -> UNLIMITED */
    void fixed_frame_rate(Uint32 max_fps = 60);

private:
    Uint64 _last_tick    = SDL_GetPerformanceCounter();
    Uint64 _current_tick = 0;
    float _delta_time    = 0.0;

    Uint64 _frame_count = 0;
    float _elapsed_time = 0.0;
    float fps         = 0.0;

    /* MAX_FPS = 1 -> UNLIMITED */
    Uint32 MAX_FPS    = 60;
    bool paused    = false;
    Uint64 _paused_tick = 0;
};
