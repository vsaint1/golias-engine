#include  "core/systems/time_manager.h"

TimeManager::TimeManager() {
    _frequency   = SDL_GetPerformanceFrequency();
    _last_tick   = SDL_GetPerformanceCounter();
    _startup_time = _last_tick;
}

float TimeManager::get_delta_time() const {
    return _is_paused ? 0.0f : _delta_time;
}

float TimeManager::get_raw_delta_time() const {
    return _raw_delta_time;
}

Uint64 TimeManager::get_frame_count() const {
    return _frame_count;
}

float TimeManager::get_elapsed_time() const {
    return _elapsed_time;
}

float TimeManager::get_fps() const {
    return _fps;
}

void TimeManager::set_target_fps(Uint32 fps) {
    _target_fps = fps;
    if (fps > 0) {
        _target_frame_time = 1.0f / fps;
    } else {
        _target_frame_time = 0.0f;
    }
}

Uint32 TimeManager::get_target_fps() const {
    return _target_fps;
}

void TimeManager::pause() {
    if (!_is_paused) {
        _is_paused = true;
        _pause_start_tick = SDL_GetPerformanceCounter();
    }
}

void TimeManager::resume() {
    if (_is_paused) {
        _is_paused = false;
        Uint64 pause_duration = SDL_GetPerformanceCounter() - _pause_start_tick;
        _last_tick += pause_duration;
        _startup_time += pause_duration;
    }
}

bool TimeManager::is_paused() const {
    return _is_paused;
}

void TimeManager::update() {
    _current_tick   = SDL_GetPerformanceCounter();
    _raw_delta_time = static_cast<float>(_current_tick - _last_tick) / _frequency;

    if (_raw_delta_time > 0.25f) {
        _raw_delta_time = 0.25f;
    }

    if (!_is_paused) {
        if (_target_fps > 0) {
            _delta_time = _target_frame_time;
        } else {
            _delta_time = _raw_delta_time;
        }

        _elapsed_time += _delta_time;
        _frame_count++;
        _fps = (_raw_delta_time > 0.0f) ? 1.0f / _raw_delta_time : 0.0f;
    } else {
        _delta_time = 0.0f;
    }

    _last_tick = _current_tick;
}

void TimeManager::limit_frame_rate() {
    if (_target_fps == 0) return;

    double frame_time = static_cast<double>(SDL_GetPerformanceCounter() - _last_tick) / _frequency;
    double sleep_time = _target_frame_time - frame_time;

    if (sleep_time > 0.0) {
        Uint32 sleep_ms = static_cast<Uint32>(sleep_time * 1000.0);
        if (sleep_ms > 0) SDL_Delay(sleep_ms);
    }
}

void TimeManager::reset() {
    _last_tick    = SDL_GetPerformanceCounter();
    _startup_time = _last_tick;
    _current_tick = _last_tick;
    _delta_time   = 0.0f;
    _raw_delta_time = 0.0f;
    _elapsed_time = 0.0f;
    _frame_count  = 0;
    _fps          = 0.0f;
    _is_paused    = false;
}