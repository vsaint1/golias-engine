#include "game.h"

bool GameApplication::Initialize() {
    return true;
}

void GameApplication::Update(float deltaTime) {
    // spdlog::info("GameApplication Update called with deltaTime: {}", deltaTime);

    if (golias::Engine::GetInstance().GetInputManager().IsActionJustPressed("Jump")) {
        spdlog::info("Jump action just pressed!");
    }
}

void GameApplication::Destroy() {

    spdlog::info("GameApplication Destroy called.");
}
