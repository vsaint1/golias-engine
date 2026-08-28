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

private:
    GameObject* mGunObject = nullptr;

    int mHealth    = 100;
    int mMaxHealth = 100;

    AnimationComponent* mGunAnimation            = nullptr;
    PlayerControllerComponent* mPlayerController = nullptr;
    AudioSourceComponent* mAudioSource           = nullptr;
};
