#include "game.h"

bool SandboxApplication::Initialize() {

    scene.CreateObject<TestObject>("TestObject1");

    spdlog::info("GameApplication Initialized successfully.");
    return true;
}

void SandboxApplication::Update(float deltaTime) {
    auto& input = golias::Engine::GetInstance().GetInputManager();
   
    if (input.IsActionJustPressed("Jump")) {
        spdlog::info("Jump action just pressed!");
    }

    scene.Update(deltaTime);

}

void SandboxApplication::Destroy() {

    spdlog::info("GameApplication Destroy called.");
}
