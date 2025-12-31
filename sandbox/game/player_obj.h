#pragma once
#include "core/engine.h"
#include "scene/3d/animation_component.h"
#include "scene/3d/camera_component.h"
#include "scene/3d/fp_controller_component.h"
#include "scene/3d/mesh_component.h"
#include "scene/3d/physics_component.h"

class Player : public golias::GameObject {
    GCLASS(Player)
public:
    Player();
    ~Player() = default;

    void Start() override;
    void Update(float deltaTime) override;

private:
    golias::AnimationComponent* animComp                     = nullptr;
    golias::FirstPersonControllerComponent* fpControllerComp = nullptr;
};
