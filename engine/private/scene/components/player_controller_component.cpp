#include "scene/components/player_controller_component.h"

#include "core/engine.h"
#include "physics/kinematic_character_controller.h"

namespace golias {

    CharacterControllerComponent::~CharacterControllerComponent() {

        if (mCharacterController) {
            mCharacterController->RemoveContactListener(this);
            delete mCharacterController;
            mCharacterController = nullptr;
        }
    }

    bool CharacterControllerComponent::LoadProperties(const Json& properties) {

        if (properties.contains("properties") && properties["properties"].is_object()) {

            const Json& prop = properties["properties"];

            if (prop.contains("radius")) {
                mRadius = prop["radius"].get<float>();
            }

            if (prop.contains("height")) {
                mHeight = prop["height"].get<float>();
            }

            if (prop.contains("speed")) {
                mMoveSpeed = prop["speed"].get<float>();
            }

            if (prop.contains("sensitivity")) {
                mSensitivity = prop["sensitivity"].get<float>();
            }

            if (prop.contains("collision") && prop["collision"].is_object()) {
                const auto& collisionObj = prop["collision"];
                if (collisionObj.contains("layer")) {
                    short layer     = parse_collision_bitmask(collisionObj["layer"]);
                    mCollisionLayer = layer;
                }

                if (collisionObj.contains("mask")) {
                    short mask     = parse_collision_bitmask(collisionObj["mask"]);
                    mCollisionMask = mask;
                }
            }
        }

        return true;
    }

    void CharacterControllerComponent::Start() {
        mCharacterController = new KinematicCharacterController(mRadius, mHeight, mCollisionLayer, mCollisionMask);
        mCharacterController->SetGameObject(GetOwner());
        mCharacterController->AddContactListener(this);
        mCharacterController->SetPosition(GetOwner()->GetPosition());
    }

    void CharacterControllerComponent::Update(float deltaTime) {
        InputManager& inputManager = Engine::GetInstance().GetInputManager();

        glm::quat rotation = GetOwner()->GetRotation();

        const glm::vec2 mouseDelta = inputManager.GetMouseDelta();
        if (glm::length2(mouseDelta) > 0.0f) {

            float yaw   = mouseDelta.x * mSensitivity;
            float pitch = mouseDelta.y * mSensitivity;

            glm::quat yawRotation   = glm::angleAxis(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::quat pitchRotation = glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));

            rotation = yawRotation * rotation * pitchRotation;
            GetOwner()->SetRotation(rotation);
        }

        glm::mat4 rotationMatrix = glm::mat4_cast(rotation);


        glm::vec3 forward = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
        forward.y         = 0.0f;
        forward           = glm::normalize(forward);

        glm::vec3 right = glm::vec3(rotationMatrix * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
        right.y         = 0.0f;
        right           = glm::normalize(right);

        glm::vec3 movement(0.0f);

        if (!inputManager.IsCanvasFocused()) {

            if (inputManager.IsKeyPressed(KeyCode::W)) {
                movement += forward;
            }

            if (inputManager.IsKeyPressed(KeyCode::S)) {
                movement -= forward;
            }

            if (inputManager.IsKeyPressed(KeyCode::A)) {
                movement -= right;
            }

            if (inputManager.IsKeyPressed(KeyCode::D)) {
                movement += right;
            }
        }

        if (glm::length2(movement) > 0.0f) {
            movement = glm::normalize(movement) * mMoveSpeed * PhysicsManager::kFixedTimeStep;
        }

        mCharacterController->Walk(movement);

        GetOwner()->SetPosition(mCharacterController->GetPosition());
    }

    float CharacterControllerComponent::GetMoveSpeed() const {
        return mMoveSpeed;
    }
    
    void CharacterControllerComponent::SetMoveSpeed(float speed) {
        mMoveSpeed = speed;
    }

    float CharacterControllerComponent::GetSensitivity() const {
        return mSensitivity;
    }

    void CharacterControllerComponent::SetSensitivity(float sensitivity) {
        mSensitivity = sensitivity;
    }

    KinematicCharacterController* CharacterControllerComponent::GetCharacterController() const {
        return mCharacterController;
    }

    bool CharacterControllerComponent::OnGround() const {
        if (mCharacterController) {
            return mCharacterController->IsOnGround();
        }

        return false;
    }

    void CharacterControllerComponent::Jump(const glm::vec3& direction) {
        if (mCharacterController) {
            mCharacterController->Jump(direction);
        }
    }

    void CharacterControllerComponent::ApplyForce(const glm::vec3& direction, float force) {
        if (mCharacterController) {
            mCharacterController->ApplyForce(direction, force);
        }
    }

    void CharacterControllerComponent::OnCollisionEnter(const Collision& collision) {
        if (GetOwner()) {
            GetOwner()->OnCollisionEnter(collision);
        }
    }

    void CharacterControllerComponent::OnCollisionExit(const Collision& collision) {
        if (GetOwner()) {
            GetOwner()->OnCollisionExit(collision);
        }
    }

} // namespace golias
