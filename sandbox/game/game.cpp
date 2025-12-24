#include "game.h"

#include "scene/3d/camera_component.h"

bool SandboxApplication::Initialize() {

    scene = new golias::Scene();
    scene->CreateObject<TestObject>("TestObject1");

    auto camera = scene->CreateObject<golias::GameObject>("Camera");
    camera->AddComponent(new golias::CameraComponent());
    camera->SetPosition({0.0f, 0.0f, 2.0f});
    scene->SetMainCamera(camera);

 
    camera->GetWorldTransform();

    golias::Engine::GetInstance().SetScene(scene);

    spdlog::info("GameApplication Initialized successfully.");
    return true;
}

void SandboxApplication::Update(float deltaTime) {
    auto& input = golias::Engine::GetInstance().GetInputManager();

    if (input.IsActionJustPressed("Jump")) {
        spdlog::info("Jump action just pressed!");
    }

    scene->Update(deltaTime);
}

void SandboxApplication::Destroy() {

    spdlog::info("GameApplication Destroy called.");
}
