#pragma once

#include <chrono>

namespace golias {

    using Clock = std::chrono::steady_clock;

    class Time {
    public:
        static float GetDeltaTime();

        static float GetElapsedTime();

        static void Start();

        static void Tick();

    private:
        static float sDeltaTime;
        static float sElapsedTime;
        static Clock::time_point sLastTime;
    };

} // namespace golias
