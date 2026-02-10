#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace golias {

    class GameObject;
    class RigidBody;

    /// @brief A single contact point between two colliding objects.
    struct ContactPoint {
        glm::vec3 point;              ///< World-space contact position
        glm::vec3 normal;             ///< Contact normal (from other toward this object)
        float impulse = 0.0f;         ///< Normal impulse magnitude at this contact
        float distance = 0.0f;        ///< Penetration distance (negative = overlapping)
    };

    /// @brief Information about a collision between two objects.
    /// Passed to OnCollisionEnter / OnCollisionStay / OnCollisionExit callbacks.
    struct CollisionInfo {
        GameObject* gameObject = nullptr;     ///< The other GameObject involved in the collision
        RigidBody* rigidBody   = nullptr;     ///< The other RigidBody involved
        glm::vec3 relativeVelocity;           ///< Relative velocity of the two bodies at the contact point
        std::vector<ContactPoint> contacts;   ///< Contact points for this collision pair
    };

} // namespace golias
