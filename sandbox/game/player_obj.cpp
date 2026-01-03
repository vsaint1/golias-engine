#include "player_obj.h"

void Player::Start() {

#if defined(SCENE_LOAD_FROM_FILE)
    auto scene = golias::Engine::GetInstance().GetScene();

    fpControllerComp = new golias::FirstPersonControllerComponent();
    auto camera      = scene->CreateObject<golias::GameObject>("Camera");
    camera->AddComponent(new golias::CameraComponent());
    camera->AddComponent(fpControllerComp);
    camera->SetPosition({0.0f, 2.0f, 5.0f});
    scene->SetMainCamera(camera);

    auto gun = golias::GameObject::LoadModel("models/carbine/scene.gltf", scene);
    gun->SetParent(camera);
    gun->SetPosition({0.75f, -0.5f, -0.75f});
    gun->SetScale({-1.0f, 1.0f, 1.0f});

    if (auto bullet = gun->FindChildByName("bullet_33")) {
        bullet->SetActive(false);
    }

    if (auto fire = gun->FindChildByName("BOOM_35")) {
        fire->SetActive(false);
    }

    if (auto ac = gun->GetComponent<golias::AnimationComponent>()) {
        animComp = ac;
    }

#else

    if (auto bullet = FindChildByName("bullet_33")) {
        bullet->SetActive(false);
    }

    if (auto fire = FindChildByName("BOOM_35")) {
        fire->SetActive(false);
    }

    if (auto gun = FindChildByName("Gun")) {
        if (auto ac = gun->GetComponent<golias::AnimationComponent>()) {
            animComp = ac;
        }
    }

    audioComp         = GetComponent<golias::AudioComponent>();
    audioListenerComp = GetComponent<golias::AudioListenerComponent>();
#endif
}

Player::Player() {
}

void Player::Update(float deltaTime) {
    golias::GameObject::Update(deltaTime);

    auto& input = golias::Engine::GetInstance().GetInputManager();

    if (input.IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
        // Just play - the Audio::Play() method handles stopping and restarting
        animComp->Play("shoot", false);
        audioComp->Play("GunShot", false);
    }

    if (input.IsKeyPressed(SDLK_R)) {
        if (!animComp->IsPlaying()) {
            animComp->Play("reload", false);
        }
    }
}
