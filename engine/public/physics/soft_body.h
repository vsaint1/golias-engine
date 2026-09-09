#pragma once

#include "physics/collision.h"
#include "stdafx.h"

class btSoftBody;

namespace golias {

    enum class SoftBodyPinnedCorners : uint32_t {
        None        = 0,
        TopLeft     = 1,
        TopRight    = 2,
        BottomLeft  = 4,
        BottomRight = 8,
        Top         = 3,
        Bottom      = 12,
        Left        = 5,
        Right       = 10,
        All         = 15,
    };

    struct SoftBodyMaterial {
        float Mass             = 10.0f;
        float LinearStiffness  = 0.9f;
        float AngularStiffness = 0.4f;
        float VolumeStiffness  = 0.4f;
        float Damping          = 0.02f;
        float Friction         = 0.5f;
        float CollisionMargin  = 0.25f;
        int PositionIterations = 8;
        int VelocityIterations = 4;
    };

    class SoftBody : public CollisionObject {
    public:
        SoftBody(float width,
                 float height,
                 uint32_t subdivisionsX,
                 uint32_t subdivisionsY,
                 SoftBodyPinnedCorners fixedCorners = SoftBodyPinnedCorners::All,
                 const SoftBodyMaterial& material   = {});

        ~SoftBody();

        bool IsAddedToWorld() const;
        void SetAddedToWorld(bool added);

        btSoftBody* GetBody() const;

        glm::vec3 GetPosition() const;
        void SetPosition(const glm::vec3& position);

        void ApplyForce(const glm::vec3& force);

        void SetDamping(float damping);

        void SetStiffness(float linear, float angular, float volume);

        float GetWidth() const;
        float GetHeight() const;

        uint32_t GetSubdivisionsX() const;
        uint32_t GetSubdivisionsY() const;


    private:
        SoftBody(const SoftBody&)            = delete;
        SoftBody& operator=(const SoftBody&) = delete;
        SoftBody(SoftBody&&) noexcept;

    private:
        btSoftBody* mBody = nullptr;

        float mWidth  = 1.0f;
        float mHeight = 1.0f;

        uint32_t mSubdivisionsX = 1;
        uint32_t mSubdivisionsY = 1;

        bool mIsAddedToWorld = false;
        glm::vec3 mPosition  = glm::vec3(0.0f);
    };
} // namespace golias
