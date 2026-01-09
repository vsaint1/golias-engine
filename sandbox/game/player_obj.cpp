#include "player_obj.h"

#include "bullet.h"

void Player::Start() {

#if defined(SCENE_LOAD_FROM_FILE)
    auto scene = golias::Engine::GetInstance().GetScene();

    fpControllerComp = new golias::FirstPersonControllerComponent();
    auto camera      = scene->CreateObject<golias::GameObject>("Camera");
    camera->AddComponent(new golias::CameraComponent());
    camera->AddComponent(fpControllerComp);
    camera->SetPosition({0.0f, 2.0f, 5.0f});
    scene->SetMainCamera(camera);

    auto gun = golias::Model::Load("models/carbine/scene.gltf", scene);
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

    bool isMousePressed = input.IsMouseButtonPressed(SDL_BUTTON_LEFT);
    
    if (isMousePressed && !wasMousePressed) {
        animComp->Play("shoot", false);
        audioComp->Play("GunShot", false);

        auto bullet = golias::Engine::GetInstance().GetScene()->CreateObject<Bullet>("Bullet");
        auto mesh   = golias::Mesh::CreateSphere(0.2f, 32, 32);
        auto mat    = golias::Material::Load("materials/checker.mat");
        bullet->AddComponent(new golias::MeshComponent(mesh, mat));

        auto pos = FindChildByName("BOOM_35")->GetWorldPosition();
        bullet->SetPosition(pos + GetRotation() * glm::vec3(-0.2f, 0.2, -1.7f));

        auto collider = std::make_shared<golias::SphereCollider>(0.2f);
        auto rb       = std::make_shared<golias::RigidBody>(golias::EBodyType::DYNAMIC, collider, 10.0f);

        bullet->AddComponent(new golias::PhysicsComponent(rb));

        glm::vec3 front = GetRotation() * glm::vec3(0.0f, 0.0f, -1.0f);
        rb->ApplyImpulse(front * 500.0f);
    }
    
    wasMousePressed = isMousePressed;

    if (input.IsKeyPressed(SDLK_R)) {
        if (!animComp->IsPlaying()) {
            animComp->Play("reload", false);
        }
    }
}
