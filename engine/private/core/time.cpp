#include "core/time.h"

namespace golias {

    float Time::sDeltaTime            = 0.0f;
    float Time::sElapsedTime          = 0.0f;
    Clock::time_point Time::sLastTime = {};
    
    void Time::Start() {
        sLastTime = Clock::now();
        sDeltaTime = 0.0f;
        sElapsedTime = 0.0f;
    }

    float Time::GetDeltaTime() {
        return sDeltaTime;
    }

    float Time::GetElapsedTime() {
        return sElapsedTime;
    }

    void Time::Tick() {
        Clock::time_point now = Clock::now();
        sDeltaTime            = std::chrono::duration<float>(now - sLastTime).count();
        sElapsedTime += sDeltaTime;
        sLastTime = now;
    }

} // namespace golias
