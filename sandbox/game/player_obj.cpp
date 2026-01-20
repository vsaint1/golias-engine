#include "player_obj.h"

#include "bullet.h"

void Player::Start() {
    auto scene          = golias::Engine::GetInstance().GetScene();
    characterController = GetComponent<golias::CharacterControllerComponent>();

    verticalVelocity = 0.0f;

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
}

Player::Player() {
    verticalVelocity = 0.0f;
    wasMousePressed  = false;
}

// *unused* Let the engine handle gravity
void Player::ApplyGravity() {
    if (!characterController) {
        return;
    }

    if (characterController->IsGrounded() && verticalVelocity < 0.0f) {
        verticalVelocity = -2.0f;
    }

    verticalVelocity += gravity * lastDeltaTime;
}

void Player::Move(float deltaTime) {
    if (!characterController) {
        return;
    }

    auto& input = golias::Engine::GetInstance().GetInputManager();

    glm::quat yawRot     = glm::angleAxis(glm::radians(yaw), glm::vec3(0, 1, 0));
    glm::vec3 camForward = yawRot * glm::vec3(0, 0, -1);
    glm::vec3 camRight   = yawRot * glm::vec3(1, 0, 0);

    camForward.y = 0.0f;
    camRight.y   = 0.0f;

    camForward = glm::normalize(camForward);
    camRight   = glm::normalize(camRight);

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

    lastDeltaTime = deltaTime;

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

        auto pos = FindChildByName("BOOM_35")->GetWorldPosition();
        bullet->SetPosition(pos + GetRotation() * glm::vec3(-0.2f, 0.2, -1.7f));

        auto collider = std::make_shared<golias::SphereCollider>(0.2f);
        auto rb       = std::make_shared<golias::RigidBody>(golias::EBodyType::DYNAMIC, collider, 10.0f);
        bullet->AddComponent(new golias::PhysicsComponent(rb));

        glm::vec3 front = GetRotation() * glm::vec3(0.0f, 0.0f, -1.0f);
        rb->ApplyImpulse(front * 500.0f);
    }
    wasMousePressed = isMousePressed;

    // Reload
    if (input.IsKeyPressed(SDLK_R) && animComp && !animComp->IsPlaying()) {
        animComp->Play("reload", false);
    }
}
