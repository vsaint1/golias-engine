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

        if (GameObject* fire = gun->FindChildByName("BOOM_35")) {
            fire->SetActive(false);
        }

        if (AnimationComponent* animComp = gun->GetComponent<AnimationComponent>()) {
            mGunAnimation = animComp;
        }
    }

    if (CharacterControllerComponent* playerController = GetComponent<CharacterControllerComponent>()) {
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
        return;
    }

    mAudioSource->Play("hurt");

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

    if (inputManager.IsCanvasFocused()) {
        return;
    }


    if (inputManager.IsKeyJustPressed(KeyCode::R)) {

        if (mGunAnimation->IsPlaying("reload")) {
            return;
        }

        mGunAnimation->Play("reload", false);
        mAudioSource->Play("gun_reload");
        mAmmo = mMaxAmmo;
    }

    if (inputManager.IsMouseButtonJustPressed(MouseButton::Left)) {

        if (mGunAnimation->IsPlaying("shoot") || mGunAnimation->IsPlaying("reload")) {
            return;
        }

        mGunAnimation->Play("shoot", false);
       
        if (mAmmo <= 0) {
            mAudioSource->Play("gun_clip_empty");
            return;
        }

        mAudioSource->Play("gun_shoot");
        mAmmo--;


        Bullet* bullet = GetCurrentScene()->CreateGameObject<Bullet>("Bullet", this);

        bullet->AddComponent(new StaticMeshComponent(mSphereMesh, mSphereMaterial));

        if (GameObject* child = mGunObject->FindChildByName("BOOM_35")) {
            const glm::vec3 muzzlePosition = child->GetWorldPosition();
            const glm::vec3 direction      = glm::normalize(GetRotation() * glm::vec3(-0.1f, 0.1f, 1.75f));

            bullet->SetWorldPosition(muzzlePosition + direction);

            Ref<Collider> collider = std::make_shared<SphereCollider>(0.2f);

            PhysicsMaterial phys = {.Mass = 2.0f, .Restitution = 0.0f};
            Ref<RigidBody> rb    = std::make_shared<RigidBody>(RigidBodyType::Dynamic, collider, phys);
            rb->SetCollisionLayer(6);

            bullet->AddComponent(new PhysicsComponent(rb));

            rb->SetLinearVelocity(direction * 80.0f);
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
