#pragma once

#include "golias.h"

using namespace golias;

class HurtPlatform : public GameObject {
    GCLASS(HurtPlatform)
public:
    void Start() override;

    void Update(float deltaTime) override;

    void OnCollisionEnter(const Collision& collision) override;
    void OnCollisionExit(const Collision& collision) override;

private:
    StaticMeshComponent* mStaticMesh = nullptr;
};
