#include  "core/systems/time_manager.h"

TimeManager::TimeManager() {
    _frequency = SDL_GetPerformanceFrequency();
    _last_tick = SDL_GetPerformanceCounter();
    _startup_time = _last_tick;
    _fps_samples.resize(_fps_sample_count, 0.0f);
}

float TimeManager::get_delta_time() const {
    return _is_paused ? 0.0f : _delta_time;
}

float TimeManager::get_raw_delta_time() const {
    return _raw_delta_time;
}

float TimeManager::get_scaled_delta_time() const {
    return _is_paused ? 0.0f : _delta_time * _time_scale;
}

Uint64 TimeManager::get_frame_count() const {
    return _frame_count;
}

float TimeManager::get_elapsed_time() const {
    return _elapsed_time;
}

float TimeManager::get_total_time() const {
    return static_cast<float>(_current_tick - _startup_time) / _frequency;
}

float TimeManager::get_fps() const {
    return _fps;
}

float TimeManager::get_average_fps() const {
    return _average_fps;
}

Uint32 TimeManager::get_target_fps() const {
    return _target_fps;
}

void TimeManager::set_target_fps(Uint32 fps) {
    _target_fps = fps;
    if (fps > 0) {
        _target_frame_time = 1.0f / fps;
        LOG_INFO("Target FPS set to %u (%.3f ms per frame)", fps, _target_frame_time * 1000.0f);
    } else {
        _target_frame_time = 0.0f;
        LOG_INFO("Target FPS set to unlimited");
    }
}

float TimeManager::get_time_scale() const {
    return _time_scale;
}

void TimeManager::set_time_scale(float scale) {
    _time_scale = scale > 0.0f ? scale : 0.0f;
}

bool TimeManager::is_paused() const {
    return _is_paused;
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

void TimeManager::toggle_pause() {
    _is_paused ? resume() : pause();
}

void TimeManager::limit_frame_rate() {
    if (_target_fps == 0) return;

    double frame_time = static_cast<double>(SDL_GetPerformanceCounter() - _last_tick) / _frequency;

    double sleep_time = _target_frame_time - frame_time;
    static double leftover_time = 0.0;
    sleep_time += leftover_time;

    if (sleep_time > 0.0) {
        Uint32 sleep_ms = static_cast<Uint32>(sleep_time * 1000.0);
        if (sleep_ms > 0) SDL_Delay(sleep_ms);

        double slept = sleep_ms / 1000.0;
        leftover_time = sleep_time - slept;
    } else {
        leftover_time = 0.0;
    }

    _current_tick = SDL_GetPerformanceCounter();
}

void TimeManager::update() {
    _current_tick   = SDL_GetPerformanceCounter();
    _raw_delta_time = static_cast<float>(_current_tick - _last_tick) / _frequency;
    _raw_delta_time = _raw_delta_time > _max_delta_time ? _max_delta_time : _raw_delta_time;

    if (!_is_paused) {
        _delta_time = _raw_delta_time;
        _elapsed_time += _delta_time;
        _frame_count++;
        update_fps_calculation();
    } else {
        _delta_time = 0.0f;
    }

    _last_tick = _current_tick;
}

void TimeManager::reset() {
    _last_tick = SDL_GetPerformanceCounter();
    _startup_time = _last_tick;
    _current_tick = _last_tick;
    _delta_time = 0.0f;
    _raw_delta_time = 0.0f;
    _elapsed_time = 0.0f;
    _frame_count = 0;
    _fps = 0.0f;
    _average_fps = 0.0f;
    _is_paused = false;
    _fps_samples.assign(_fps_sample_count, 0.0f);
    _fps_sample_index = 0;
}

void TimeManager::print_debug_info() const {
    LOG_INFO("=== TimeManager Debug Info ===");
    LOG_INFO("Delta Time: %.6f ms", _delta_time * 1000.0f);
    LOG_INFO("FPS: %.2f (avg: %.2f)", _fps, _average_fps);
    LOG_INFO("Frame Count: %llu", _frame_count);
    LOG_INFO("Elapsed Time: %.3f s", _elapsed_time);
    LOG_INFO("Time Scale: %.2f", _time_scale);
    LOG_INFO("Paused: %s", _is_paused ? "Yes" : "No");
    LOG_INFO("Target FPS: %u", _target_fps);
}

void TimeManager::set_max_delta_time(float max_dt) {
    _max_delta_time = max_dt > 0.0f ? max_dt : 0.0f;
}

float TimeManager::get_max_delta_time() const {
    return _max_delta_time;
}

void TimeManager::update_fps_calculation() {
    if (_raw_delta_time > 0.0f) {
        _fps = 1.0f / _raw_delta_time;

        _fps_samples[_fps_sample_index] = _fps;
        _fps_sample_index = (_fps_sample_index + 1) % _fps_sample_count;

        float total = 0.0f;
        for (float sample : _fps_samples) {
            total += sample;
        }
        _average_fps = total / _fps_sample_count;
    }
}
