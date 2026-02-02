#include "player_obj.h"

#include "bullet.h"

void Player::Awake() {
    spdlog::info("Player::Awake called for Player Object");
}

void Player::Start() {
    characterController = GetComponent<golias::CharacterControllerComponent>();
    audioComp           = GetComponent<golias::AudioComponent>();
    audioListenerComp   = GetComponent<golias::AudioListenerComponent>();

    if (auto camObj = FindChildByName("MainCamera")) {
        cameraObject = camObj;

        if (auto gun = cameraObject->FindChildByName("Gun")) {
            gunObject = gun;
            if (auto ac = gun->GetComponent<golias::AnimationComponent>()) {
                animComp = ac;
            }

            if (auto bullet = gun->FindChildByName("bullet_33")) {
                bullet->SetActive(false);
            }

            if (auto fire = gun->FindChildByName("BOOM_35")) {
                fire->SetActive(false);
            }
        }
    }
}

Player::Player() {
    verticalVelocity = 0.0f;
    wasMousePressed  = false;
    yaw              = 0.0f;
    pitch            = 0.0f;
}

void Player::Move(float deltaTime) {
    if (!characterController) {
        return;
    }

    auto& input = golias::Engine::GetInstance().GetInputManager();

    if (cameraObject) {
        glm::vec2 mouseDelta = input.GetMouseDelta();
        float sensitivity    = 0.3f;

        yaw -= mouseDelta.x * sensitivity;
        pitch -= mouseDelta.y * sensitivity;
        
        pitch = glm::clamp(pitch, -89.0f, 89.0f);

        glm::quat yawQuat = glm::angleAxis(glm::radians(yaw), glm::vec3(0, 1, 0));
        SetRotation(yawQuat);

        glm::quat pitchQuat = glm::angleAxis(glm::radians(pitch), glm::vec3(1, 0, 0));
        cameraObject->SetRotation(pitchQuat);
    }

    glm::quat yawRot     = glm::angleAxis(glm::radians(yaw), glm::vec3(0, 1, 0));
    glm::vec3 camForward = glm::normalize(yawRot * glm::vec3(0, 0, -1));
    glm::vec3 camRight   = glm::normalize(yawRot * glm::vec3(1, 0, 0));

    glm::vec3 horizontalMove(0.0f);
    if (input.IsKeyPressed(SDLK_W)) {
        horizontalMove += camForward;
    }

    if (input.IsKeyPressed(SDLK_S)) {
        horizontalMove -= camForward;
    }

    if (input.IsKeyPressed(SDLK_D)) {
        horizontalMove += camRight;
    }

    if (input.IsKeyPressed(SDLK_A)) {
        horizontalMove -= camRight;
    }

    if (glm::length(horizontalMove) > 0.01f) {
        horizontalMove = glm::normalize(horizontalMove);
    }

    glm::vec3 velocity = horizontalMove * moveSpeed;
    characterController->Move(velocity * deltaTime);
}

void Player::Update(float deltaTime) {
    golias::GameObject::Update(deltaTime);
    if (!characterController) {
        return;
    }

    auto& input = golias::Engine::GetInstance().GetInputManager();

    if (input.IsKeyPressed(SDLK_SPACE)) {
        characterController->Jump(glm::vec3(0.0f, jumpForce, 0.0f));
    }

    Move(deltaTime);

    bool isMousePressed = input.IsMouseButtonPressed(SDL_BUTTON_LEFT);
    if (isMousePressed && !wasMousePressed) {
        if (animComp) {
            animComp->Play("shoot", false);
        }

        if (audioComp) {
            audioComp->Play("GunShot", false);
        }

        auto bullet = golias::Engine::GetInstance().GetScene()->CreateObject<Bullet>("Bullet");
        auto mesh   = golias::Mesh::CreateSphere(0.2f, 32, 32);
        auto mat    = golias::Material::Load("materials/checker.mat");
        bullet->AddComponent(new golias::MeshRendererComponent(mesh, mat));

        if (auto muzzleFlash = gunObject->FindChildByName("BOOM_35")) {
            bullet->SetPosition(muzzleFlash->GetWorldPosition());
        }

        auto collider = std::make_shared<golias::SphereCollider>(0.2f);
        auto rb       = std::make_shared<golias::RigidBody>(golias::EBodyType::DYNAMIC, collider, 10.0f);
        bullet->AddComponent(new golias::PhysicsComponent(rb));

        glm::quat combinedRotation = GetRotation() * cameraObject->GetRotation();

        glm::vec3 shootDirection = combinedRotation * glm::vec3(0.0f, 0.0f, -1.0f);
        rb->ApplyImpulse(glm::normalize(shootDirection) * 500.0f);
    }
    wasMousePressed = isMousePressed;

    if (input.IsKeyPressed(SDLK_R) && animComp && !animComp->IsPlaying()) {
        animComp->Play("reload", false);
    }
}
