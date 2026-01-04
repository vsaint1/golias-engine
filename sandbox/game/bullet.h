#pragma once
#include "scene/3d/physics_component.h"
#include "scene/game_object.h"

class Bullet : public golias::GameObject {
    GCLASS(Bullet)
public:
    Bullet();
    ~Bullet() = default;

    void Start() override;
    void Update(float deltaTime) override;

private:
    float lifeTime = 5.0f;
};
