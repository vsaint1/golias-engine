#include "core/time_manager.h"

void TimeManager::Pause() {
    if (!bIsPaused) {
        bIsPaused  = true;
        pausedTick = SDL_GetPerformanceCounter();
    }
}

void TimeManager::Resume() {
    if (bIsPaused) {
        bIsPaused         = false;
        Uint64 resumeTick = SDL_GetPerformanceCounter();
        lastTick += (resumeTick - pausedTick);
        pausedTick = 0;
    }
}
void TimeManager::Update() {
    if (bIsPaused) {
        deltaTime = 0.0f; // Avoid accumulating DT when paused
        return;
    }

    currentTick = SDL_GetPerformanceCounter();

    if (lastTick == 0) {
        lastTick = currentTick; 
    }

    deltaTime = (float) (currentTick - lastTick) / SDL_GetPerformanceFrequency();

    lastTick  = currentTick;

    const float maxDeltaTime = 1.0f / MAX_FPS;
    if (deltaTime > maxDeltaTime) {
        deltaTime = maxDeltaTime;
    }

    frameCount++;
    elapsedTime += deltaTime;

    if (elapsedTime >= 1.0f) {
        fps         = (float) frameCount / elapsedTime;
        frameCount  = 0;
        elapsedTime = 0.0f;
    }
}

void TimeManager::FixedFrameRate(Uint32 MAX_FPS) {
    this->MAX_FPS = MAX_FPS;

    if (MAX_FPS > 1) {
        float delay = 1000.0f / MAX_FPS;
        if (delay > 0) {
            SDL_Delay((Uint32) delay);
        }
    }
}