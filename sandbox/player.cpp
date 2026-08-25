#include "player.h"


Player::Player() {
}

void Player::Start() {

    if (GameObject* gun = FindChildByName("Gun")) {
        mGunObject = gun;

        if (GameObject* bullet = FindChildByName("bullet_33")) {
            bullet->SetActive(false);
        }

        if (GameObject* fire = FindChildByName("BOOM_35")) {
            fire->SetActive(false);
        }

        if (AnimationComponent* animComp = gun->GetComponent<AnimationComponent>()) {
            mGunAnimation = animComp;
        }
    }

    if (PlayerControllerComponent* playerController = GetComponent<PlayerControllerComponent>()) {
        mPlayerController = playerController;
    }
}

void Player::Update(float deltaTime) {
    GameObject::Update(deltaTime);

    InputManager& inputManager = Engine::GetInstance().GetInputManager();

    if (inputManager.IsKeyPressed(KeyCode::Escape)) {
        GOLIAS_LOG_INFO("Escape key pressed. Closing the application.");
    }

    if (inputManager.IsMouseButtonJustPressed(MouseButton::Left)) {

        mGunAnimation->Play("shoot", false);
    }

    if (inputManager.IsKeyJustPressed(KeyCode::Space)) {
        mPlayerController->GetCharacterController()->Jump(glm::vec3(0.0f, 5.0f, 0.0f));
    }
}
