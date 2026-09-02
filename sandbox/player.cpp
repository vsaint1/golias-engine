#include "player.h"

#include "bullet.h"

Player::Player() {
    mSphereMesh = Mesh::CreateSphere(0.2f);
}

void Player::Start() {

    if (GameObject* gun = FindChildByName("Gun")) {
        mGunObject = gun;

        if (GameObject* bullet = gun->FindChildByName("bullet_33")) {
            bullet->SetActive(false);
        }

        // TODO: We should disable the Mesh not the GObject
        if (GameObject* fire = gun->FindChildByName("BOOM_35")) {
            // fire->SetActive(false);
        }

        if (AnimationComponent* animComp = gun->GetComponent<AnimationComponent>()) {
            mGunAnimation = animComp;
        }
    }

    if (PlayerControllerComponent* playerController = GetComponent<PlayerControllerComponent>()) {
        mPlayerController = playerController;
    }

    if (AudioSourceComponent* audioSource = GetComponent<AudioSourceComponent>()) {
        mAudioSource = audioSource;
    }

    if (GameObject* canvas = FindChildByName("HUDCanvas")) {
        if (ProgressBarComponent* healthBar = canvas->FindChildByName("HUDHealth")->GetComponent<ProgressBarComponent>()) {
            healthBar->SetMinValue(0.0f);
            healthBar->SetMaxValue(static_cast<float>(mMaxHealth));
            mHealthBar = healthBar;
        }
    }


    mSphereMaterial = Engine::GetInstance().GetAssetManager().Load<Material>("materials/suzanne.gmat");
}

int Player::GetHealth() const {
    return mHealth;
}

void Player::Heal(int amount) {
    mHealth += amount;
    mHealth = glm::clamp(mHealth, 0, mMaxHealth);

    if (mHealthBar) {
        mHealthBar->SetValue(mHealth);
    }
}

void Player::TakeDamage(int amount) {
    mHealth -= amount;
    mHealth = glm::clamp(mHealth, 0, mMaxHealth);

    if (mHealth <= 0) {
        GOLIAS_LOG_WARN("PLAYER DIED");
    }

    if (mHealthBar) {
        mHealthBar->SetValue(mHealth);
    }
}

void Player::Knockback(const glm::vec3& direction, float force) {
  
    mPlayerController->ApplyForce(direction, force);
}

void Player::Update(float deltaTime) {
    GameObject::Update(deltaTime);

    InputManager& inputManager = Engine::GetInstance().GetInputManager();

    if (inputManager.IsKeyPressed(KeyCode::Escape)) {
        GOLIAS_LOG_INFO("Escape key pressed. Closing the application.");
    }

    if (inputManager.IsMouseButtonJustPressed(MouseButton::Left)) {

        if (mGunAnimation->IsPlaying()) {
            return;
        }

        mGunAnimation->Play("shoot", false);

        mAudioSource->Play("gun_shoot");


        Bullet* bullet = GetCurrentScene()->CreateGameObject<Bullet>("Bullet", GetParent());

        bullet->AddComponent(new StaticMeshComponent(mSphereMesh, mSphereMaterial));

        if (GameObject* child = mGunObject->FindChildByName("BOOM_35")) {
            const glm::vec3 muzzlePosition = child->GetWorldPosition();
            const glm::vec3 direction      = GetRotation() * glm::vec3(-0.1f, 0.2f, 1.75f);

            bullet->SetPosition(muzzlePosition + direction);

            Ref<Collider> collider = std::make_shared<SphereCollider>(0.2f);

            PhysicsMaterial phys = {.Mass = 10, .Restitution = 1};
            Ref<RigidBody> rb    = std::make_shared<RigidBody>(RigidBodyType::Dynamic, collider, phys);

            bullet->AddComponent(new PhysicsComponent(rb));

            rb->ApplyImpulse(direction * 200.0f);
        }
    }

    if (inputManager.IsKeyPressed(KeyCode::LeftShift)) {
        mPlayerController->SetMoveSpeed(8.0f);
    } else {
        mPlayerController->SetMoveSpeed(4.0f);
    }

    if (inputManager.IsKeyJustPressed(KeyCode::Space)) {
        mPlayerController->Jump(glm::vec3(0.0f, 5.0f, 0.0f));
    }
}
