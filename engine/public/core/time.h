#pragma once

#include <SDL3/SDL_timer.h>

namespace golias {

    /// @brief Centralized time tracking (Unity-style Time API).
    ///        Updated once per frame by the engine core loop.
    class Time {
    public:
        /// Time in seconds since the start of the application (affected by timeScale)
        static float GetTime();

        /// The interval in seconds from the last frame to the current one (affected by timeScale)
        static float GetDeltaTime();

        /// The timeScale-independent interval in seconds from the last frame to the current one
        static float GetUnscaledDeltaTime();

        /// The scale at which time passes. 1.0 = normal, 0.5 = half speed, 0.0 = paused
        static float GetTimeScale();
        static void SetTimeScale(float scale);

        /// Real time in seconds since the application started (unaffected by timeScale)
        static float GetRealtimeSinceStartup();

        /// The total number of frames that have been rendered
        static int GetFrameCount();

        /// The reciprocal of the current delta time (frames per second)
        static float GetFps();

        /// Smoothed delta time (useful for display)
        static float GetSmoothDeltaTime();

        /// Fixed time step for physics (default 0.02 = 50Hz)
        static float GetFixedDeltaTime();
        static void SetFixedDeltaTime(float dt);

    private:
        friend class Engine;
        friend void engine_core_loop();

        /// Called once at engine initialization
        static void Initialize();

        /// Called once per frame by the engine core loop
        static void Update();

        static float deltaTime_;
        static float unscaledDeltaTime_;
        static float time_;
        static float unscaledTime_;
        static float timeScale_;
        static float smoothDeltaTime_;
        static float fixedDeltaTime_;
        static int frameCount_;
        static Uint64 lastTimePoint_;
        static Uint64 startTimePoint_;
    };

} // namespace golias
