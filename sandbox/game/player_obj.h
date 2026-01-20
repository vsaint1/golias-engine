#pragma once
#include "core/engine.h"
#include "scene/3d/animation_component.h"
#include "scene/3d/audio_component.h"
#include "scene/3d/audio_listener_component.h"
#include "scene/3d/camera_component.h"
#include "scene/3d/character_controller_component.h"
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
    golias::AnimationComponent* animComp                      = nullptr;
    golias::CharacterControllerComponent* characterController = nullptr;
    golias::AudioComponent* audioComp                         = nullptr;
    golias::AudioListenerComponent* audioListenerComp         = nullptr;

    void ApplyGravity();
    void Move(float deltaTime);

    float lastDeltaTime    = 0.0f;
    float moveSpeed        = 5.0f;
    float jumpForce        = 6.0f;
    float gravity          = -25.0f;
    float verticalVelocity = 0.0f;
    float yaw              = 0.0f;
    bool wasMousePressed   = false;
};
