#include "core/time_manager.h"

void TimeManager::pause() {
    if (!_bPaused) {
        _bPaused  = true;
        _paused_tick = SDL_GetPerformanceCounter();
    }
}

void TimeManager::resume() {
    if (_bPaused) {
        _bPaused         = false;
        Uint64 resumeTick = SDL_GetPerformanceCounter();
        _last_tick += (resumeTick - _paused_tick);
        _paused_tick = 0;
    }
}
void TimeManager::update() {
    if (_bPaused) {
        _delta_time = 0.0f; // Avoid accumulating DT when paused
        return;
    }

    _current_tick = SDL_GetPerformanceCounter();

    if (_last_tick == 0) {
        _last_tick = _current_tick;
    }

    _delta_time = static_cast<float>(_current_tick - _last_tick) / SDL_GetPerformanceFrequency();

    const float maxDeltaTime = 1.0f / MAX_FPS;
    if (_delta_time > maxDeltaTime) {
        _delta_time = maxDeltaTime;
    }

    _last_tick = _current_tick;

    _frame_count++;

    _elapsed_time += _delta_time;

    if (_elapsed_time >= 1.0f) {
        fps         = (float) _frame_count / _elapsed_time;
        _frame_count  = 0;
        _elapsed_time = 0.0f;
    }
}

void TimeManager::fixed_frame_rate(Uint32 max_fps) {
    this->MAX_FPS = max_fps;

    if (MAX_FPS < 1) {
        return;
    }

    Uint64 frameDuration = SDL_GetPerformanceFrequency() / MAX_FPS;
    Uint64 tickEnd       = _last_tick + frameDuration;

    while (SDL_GetPerformanceCounter() + (SDL_GetPerformanceFrequency() / 1000) < tickEnd) {
        SDL_Delay(1);
    }

    while (SDL_GetPerformanceCounter() < tickEnd) {
    }
}
