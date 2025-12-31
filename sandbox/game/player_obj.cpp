#include "player_obj.h"

void Player::Start() {

    spdlog::info("Player created.");
}

Player::Player() {

    auto scene = golias::Engine::GetInstance().GetScene();

    fpControllerComp = new golias::FirstPersonControllerComponent();
    auto camera      = scene->CreateObject<golias::GameObject>("Camera");
    camera->AddComponent(new golias::CameraComponent());
    camera->AddComponent(fpControllerComp);
    camera->SetPosition({0.0f, 2.0f, 5.0f});
    scene->SetMainCamera(camera);

    auto gun = golias::GameObject::LoadModel("models/carbine/scene.gltf");
    gun->SetParent(camera);
    gun->SetPosition({0.75f, -0.5f, -0.75f});
    gun->SetScale({-1.0f, 1.0f, 1.0f});

    if (auto anim = gun->GetComponent<golias::AnimationComponent>()) {


        if (auto bullet = gun->FindChildByName("bullet_33")) {
            bullet->SetActive(false);
        }

        if (auto fire = gun->FindChildByName("BOOM_35")) {
            fire->SetActive(false);
        }

        animComp = anim;
    }
}

void Player::Update(float deltaTime) {
    golias::GameObject::Update(deltaTime);

    auto& input = golias::Engine::GetInstance().GetInputManager();

    if (input.IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
        animComp->Play("shoot", false);
    }

    if (input.IsKeyPressed(SDLK_R)) {
        animComp->Play("reload", false);
    }
}
