#pragma once

#include "golias.h"

using namespace golias;


class Player : public GameObject {
    GCLASS(Player)
public:
    Player();

    void Start() override;

    void Update(float deltaTime) override;

    void TakeDamage(int amount);

    int GetHealth() const;

    void Heal(int amount);

    void Knockback(const glm::vec3& direction, float force);

private:
    GameObject* mGunObject = nullptr;

    int mHealth    = 100;
    int mMaxHealth = 100;

    Ref<Mesh> mSphereMesh         = nullptr;
    Ref<Material> mSphereMaterial = nullptr;

    AnimationComponent* mGunAnimation            = nullptr;
    PlayerControllerComponent* mPlayerController = nullptr;
    AudioSourceComponent* mAudioSource           = nullptr;

    ProgressBarComponent* mHealthBar = nullptr;
};
