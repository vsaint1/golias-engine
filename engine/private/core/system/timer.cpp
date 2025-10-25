#include "core/system/timer.h"


int Timer::get_fps() const {
    if (delta > 0) {
        return static_cast<int>(1.0 / delta);
    }
    return 0;
}

void Timer::start() {
    now          = SDL_GetPerformanceCounter();
    last         = now;
    delta        = 0.0;
    elapsed_time = 0.0;
}

void Timer::tick() {
    last = now;
    now  = SDL_GetPerformanceCounter();

    Uint64 freq = SDL_GetPerformanceFrequency();
    delta       = static_cast<double>(now - last) / static_cast<double>(freq);
    elapsed_time += delta;
}