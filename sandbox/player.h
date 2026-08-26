#pragma once

#include "golias.h"

using namespace golias;


class Player : public GameObject {
    GCLASS(Player)
public:
    Player();

    void Start() override;

    void Update(float deltaTime) override;

private:
    GameObject* mGunObject = nullptr;
    
    AnimationComponent* mGunAnimation = nullptr;
    PlayerControllerComponent* mPlayerController = nullptr;
    AudioSourceComponent* mAudioSource = nullptr;
};
