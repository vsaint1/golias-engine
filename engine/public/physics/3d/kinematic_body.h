#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class btPairCachingGhostObject;
class btKinematicCharacterController;
// class btGhostPairCallback;

namespace golias {

    class KinematicCharacterController {
    public:
        KinematicCharacterController(float height, float radius,const glm::vec3& position);
        ~KinematicCharacterController();

        glm::vec3 GetPosition() const;
        void SetPosition(const glm::vec3& pos);

        glm::quat GetRotation() const;
        void SetRotation(const glm::quat& rot);

        void Move(const glm::vec3& direction);
        void Jump(const glm::vec3& force);

        float GetMaxSlope() const;
        void SetMaxSlope(float value);
        
        float GetFallSpeed() const;
        void SetFallSpeed(float value);
        
        float GetJumpSpeed() const;
        void SetJumpSpeed(float value);
        
        bool OnGround() const;

    private:
        KinematicCharacterController() = default;

        float maxSlopeDegrees = 50.0f;
        float _height;
        float _radius;

        float fallSpeed = 20.0f;
        float jumpSpeed = 7.0f;

        std::unique_ptr<btPairCachingGhostObject> ghostObject;
        std::unique_ptr<btKinematicCharacterController> controller;
        // std::unique_ptr<btGhostPairCallback> ghostPairCallback;
    };
} // namespace golias
