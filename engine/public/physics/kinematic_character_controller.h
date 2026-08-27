#pragma once

#include "physics/collision.h"
#include "stdafx.h"

class btPairCachingGhostObject;
class btKinematicCharacterController;


namespace golias {
    class KinematicCharacterController : public CollisionObject {
    public:
        KinematicCharacterController(float radius, float height);
        ~KinematicCharacterController();

        glm::vec3 GetPosition() const;

        glm::quat GetRotation() const;

        void SetPosition(const glm::vec3& position);

        void Walk(const glm::vec3& direction);

        void Jump(const glm::vec3& direction);

        bool IsOnGround() const;
    private:
        float mHeight = 0.0f;
        float mRadius = 0.0f;

        btPairCachingGhostObject* mGhostObject = nullptr;
        btKinematicCharacterController* mController = nullptr;
    };
} // namespace golias
