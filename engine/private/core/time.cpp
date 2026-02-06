#include "core/time.h"

namespace golias {

    float Time::deltaTime_         = 0.0f;
    float Time::unscaledDeltaTime_ = 0.0f;
    float Time::time_              = 0.0f;
    float Time::unscaledTime_      = 0.0f;
    float Time::timeScale_         = 1.0f;
    float Time::smoothDeltaTime_   = 0.0f;
    float Time::fixedDeltaTime_    = 0.02f;
    int Time::frameCount_          = 0;
    Uint64 Time::lastTimePoint_    = 0;
    Uint64 Time::startTimePoint_   = 0;

    void Time::Initialize() {
        startTimePoint_ = SDL_GetPerformanceCounter();
        lastTimePoint_  = startTimePoint_;
        deltaTime_      = 0.0f;
        time_           = 0.0f;
        frameCount_     = 0;
    }

    void Time::Update() {
        Uint64 now   = SDL_GetPerformanceCounter();
        Uint64 freq  = SDL_GetPerformanceFrequency();
        float rawDt  = static_cast<float>(now - lastTimePoint_) / static_cast<float>(freq);
        lastTimePoint_ = now;

        // Clamp to avoid spiral-of-death on breakpoints / long hitches
        if (rawDt > 0.25f) {
            rawDt = 0.25f;
        }

        unscaledDeltaTime_ = rawDt;
        deltaTime_         = rawDt * timeScale_;

        unscaledTime_ += rawDt;
        time_         += deltaTime_;

        // Exponential moving average for smooth delta
        constexpr float smoothing = 0.1f;
        smoothDeltaTime_ = smoothDeltaTime_ + smoothing * (unscaledDeltaTime_ - smoothDeltaTime_);

        frameCount_++;
    }

    float Time::GetTime()                  { return time_; }
    float Time::GetDeltaTime()             { return deltaTime_; }
    float Time::GetUnscaledDeltaTime()     { return unscaledDeltaTime_; }
    float Time::GetTimeScale()             { return timeScale_; }
    void  Time::SetTimeScale(float scale)  { timeScale_ = scale; }

    float Time::GetRealtimeSinceStartup() {
        Uint64 now  = SDL_GetPerformanceCounter();
        Uint64 freq = SDL_GetPerformanceFrequency();
        return static_cast<float>(now - startTimePoint_) / static_cast<float>(freq);
    }

    int   Time::GetFrameCount()         { return frameCount_; }
    float Time::GetFps()                { return unscaledDeltaTime_ > 0.0f ? 1.0f / unscaledDeltaTime_ : 0.0f; }
    float Time::GetSmoothDeltaTime()    { return smoothDeltaTime_; }
    float Time::GetFixedDeltaTime()     { return fixedDeltaTime_; }
    void  Time::SetFixedDeltaTime(float dt) { fixedDeltaTime_ = dt; }

} // namespace golias
