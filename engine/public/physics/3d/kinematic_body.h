#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class btPairCachingGhostObject;
class btKinematicCharacterController;

namespace golias {

    class KinematicCharacterController {
    public:
        KinematicCharacterController(float height, float radius);
        ~KinematicCharacterController();

        glm::vec3 GetPosition() const;
        void SetPosition(const glm::vec3& pos);

        glm::quat GetRotation() const;
        void SetRotation(const glm::quat& rot);

        void Move(const glm::vec3& velocity);
        void Jump(const glm::vec3& force);

        void SetMaxSlope(float value);

        bool OnGround() const;

    private:
        KinematicCharacterController() = default;

        float maxSlopeDegrees = 50.0f;
        float _height;
        float _radius;

        std::unique_ptr<btPairCachingGhostObject> ghostObject;
        std::unique_ptr<btKinematicCharacterController> controller;
    };
} // namespace golias
