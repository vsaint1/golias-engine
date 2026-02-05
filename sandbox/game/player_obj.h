#pragma once
#include "core/engine.h"
#include "scene/3d/animation_component.h"
#include "scene/3d/audio_component.h"
#include "scene/3d/audio_listener_component.h"
#include "scene/3d/camera_component.h"
#include "scene/3d/character_controller_component.h"
#include "scene/3d/mesh_component.h"
#include "scene/3d/physics_component.h"
#include "scene/native_behaviour.h"

class Player : public golias::NativeBehaviour {
    GCLASS(Player)
public:
    Player();
    ~Player() = default;

    void Awake() override;
    void Start() override;
    void Update(float deltaTime) override;

private:
    golias::AnimationComponent* animComp                      = nullptr;
    golias::CharacterControllerComponent* characterController = nullptr;
    golias::AudioComponent* audioComp                         = nullptr;
    golias::AudioListenerComponent* audioListenerComp         = nullptr;
    golias::GameObject* cameraObject                          = nullptr;
    golias::GameObject* gunObject                             = nullptr;

    void Move(float deltaTime);

    float lastDeltaTime    = 0.0f;
    float moveSpeed        = 10.0f;
    float jumpForce        = 6.0f;
    float gravity          = -25.0f;
    float verticalVelocity = 0.0f;
  
    float yaw  = 0.0f;
    float pitch = 0.0f;

    bool wasMousePressed   = false;
};
