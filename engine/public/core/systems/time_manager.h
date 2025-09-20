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

    float get_delta_time() const;      /// delta time (0.0 if paused)
    float get_raw_delta_time() const;  /// always real frame delta
    Uint64 get_frame_count() const;
    float get_elapsed_time() const;    /// total running time
    float get_fps() const;             /// FPS ( frames per second)

    void set_target_fps(Uint32 fps = 60);
    Uint32 get_target_fps() const;

    void pause();
    void resume();
    bool is_paused() const;

    void update();
    void limit_frame_rate();   ///  optional: frame limiter, call at end of frame
    void reset();

private:
    Uint64 _frequency;
    Uint64 _last_tick;
    Uint64 _current_tick = 0;
    Uint64 _startup_time;

    float _delta_time = 0.0f;
    float _raw_delta_time = 0.0f;
    float _elapsed_time = 0.0f;

    Uint64 _frame_count = 0;
    Uint32 _target_fps = 60;
    float _target_frame_time = 1.0f / 60.0f;

    bool _is_paused = false;
    Uint64 _pause_start_tick = 0;

    float _fps = 0.0f;
};