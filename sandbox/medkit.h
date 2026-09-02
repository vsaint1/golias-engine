#pragma once
#include "golias.h"

using namespace golias;

class Medkit : public GameObject {
    GCLASS(Medkit)
public:
    void Start() override;

    void Update(float deltaTime) override;

    void OnCollisionEnter(const Collision& collision) override;
    void OnCollisionExit(const Collision& collision) override;

private:
    AudioSourceComponent* mAudioSource = nullptr;
    PhysicsComponent* mPhysicsComponent = nullptr;
    StaticMeshComponent* mMeshComponent = nullptr;

    int mHealAmount = 25;

    float mRespawnTime = 10.0f; 

    float mRespawnTimer = 00.0f; 

    bool mIsAvailable = true; 

    void ToggleVisibility(bool visible);
};
