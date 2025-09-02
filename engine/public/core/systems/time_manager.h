#pragma once
#include "logging_sys.h"

/**
 * @brief TimeManager class
 *
 * @details Manages time, delta time, frame rate, and time scaling.
 *
 * @version 0.0.1
 */
class TimeManager {
public:
    TimeManager();

    float get_delta_time() const;
    float get_raw_delta_time() const;
    float get_scaled_delta_time() const;
    Uint64 get_frame_count() const;
    float get_elapsed_time() const;
    float get_total_time() const;

    float get_fps() const;
    float get_average_fps() const;
    Uint32 get_target_fps() const;

    void set_target_fps(Uint32 fps = 60);

    float get_time_scale() const;
    void set_time_scale(float scale);

    bool is_paused() const;
    void pause();
    void resume();
    void toggle_pause();

    void limit_frame_rate();
    void update();
    void reset();
    void print_debug_info() const;

    void set_max_delta_time(float max_dt);
    float get_max_delta_time() const;

private:
    void update_fps_calculation();

    // Core timing
    Uint64 _frequency;
    Uint64 _last_tick;
    Uint64 _current_tick = 0;
    Uint64 _startup_time;

    float _delta_time = 0.0f;
    float _raw_delta_time = 0.0f;
    float _elapsed_time = 0.0f;

    // Frame management
    Uint64 _frame_count = 0;
    Uint32 _target_fps = 60;
    float _target_frame_time = 1.0f / 60.0f;
    float _max_delta_time = 1.0f / 15.0f; // Prevent spiral of death (max 15 FPS)

    // Time scaling
    float _time_scale = 1.0f;

    // Pause management
    bool _is_paused = false;
    Uint64 _pause_start_tick = 0;

    // FPS calculation
    float _fps = 0.0f;
    float _average_fps = 0.0f;
    std::deque<float> _fps_samples;
    static constexpr size_t _fps_sample_count = 60;
    size_t _fps_sample_index = 0;
};
