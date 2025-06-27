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

    deltaTime = static_cast<float>(currentTick - lastTick) / SDL_GetPerformanceFrequency();

    const float maxDeltaTime = 1.0f / MAX_FPS;
    if (deltaTime > maxDeltaTime) {
        deltaTime = maxDeltaTime;
    }

    lastTick = currentTick;

    frameCount++;
    
    elapsedTime += deltaTime;

    if (elapsedTime >= 1.0f) {
        fps         = (float) frameCount / elapsedTime;
        frameCount  = 0;
        elapsedTime = 0.0f;
    }
}

void TimeManager::FixedFrameRate(Uint32 max_fps) {
    this->MAX_FPS = max_fps;

    if (MAX_FPS < 1) {
        return;
    }

    Uint64 frameDuration = SDL_GetPerformanceFrequency() / MAX_FPS;
    Uint64 tickEnd       = lastTick + frameDuration;

    while (SDL_GetPerformanceCounter() + (SDL_GetPerformanceFrequency() / 1000) < tickEnd) {
        SDL_Delay(1);
    }

    while (SDL_GetPerformanceCounter() < tickEnd) {
    }
}
