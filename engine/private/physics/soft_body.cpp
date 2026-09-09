#include "physics/soft_body.h"

#include "core/engine.h"
#include "physics/physics_manager.h"
#include <BulletSoftBody/btSoftBody.h>
#include <BulletSoftBody/btSoftBodyHelpers.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>

namespace golias {

    SoftBody::SoftBody(float width,
                       float height,
                       uint32_t subdivisionsX,
                       uint32_t subdivisionsY,
                       SoftBodyPinnedCorners fixedCorners,
                       const SoftBodyMaterial& material)
        : mWidth(width), mHeight(height), mSubdivisionsX(std::max(1u, subdivisionsX)), mSubdivisionsY(std::max(1u, subdivisionsY)) {

        btSoftRigidDynamicsWorld* world = Engine::GetInstance().GetPhysicsManager().GetSoftWorld();

        if (!world) {
            return;
        }

        const btVector3 halfExtents(mWidth * 0.5f, 0.0f, mHeight * 0.5f);

        const btVector3 corner00(-halfExtents.x(), 0.0f, -halfExtents.z());
        const btVector3 corner10(halfExtents.x(), 0.0f, -halfExtents.z());
        const btVector3 corner01(-halfExtents.x(), 0.0f, halfExtents.z());
        const btVector3 corner11(halfExtents.x(), 0.0f, halfExtents.z());

        mBody = btSoftBodyHelpers::CreatePatch(world->getWorldInfo(),
                                               corner00,
                                               corner10,
                                               corner01,
                                               corner11,
                                               static_cast<int>(mSubdivisionsX + 1),
                                               static_cast<int>(mSubdivisionsY + 1),
                                               static_cast<int>(fixedCorners),
                                               true);
        if (!mBody) {
            return;
        }

        mBody->setUserPointer(static_cast<CollisionObject*>(this));

        mBody->setTotalMass(material.Mass);
        mBody->m_cfg.collisions = btSoftBody::fCollision::SDF_RS;
        mBody->m_cfg.kDP        = material.Damping;
        mBody->m_cfg.kDF        = material.Friction;
        mBody->m_cfg.kCHR       = 1.0f;
        mBody->m_cfg.kKHR       = 1.0f;

        mBody->getCollisionShape()->setMargin(material.CollisionMargin);

        mBody->m_cfg.piterations      = material.PositionIterations;
        mBody->m_cfg.viterations      = material.VelocityIterations;
        mBody->m_materials[0]->m_kLST = material.LinearStiffness;
        mBody->m_materials[0]->m_kAST = material.AngularStiffness;
        mBody->m_materials[0]->m_kVST = material.VolumeStiffness;

        Engine::GetInstance().GetPhysicsManager().AddSoftBody(mBody);
        mIsAddedToWorld = true;
    }

    SoftBody::~SoftBody() {

        if (mBody) {
            PhysicsManager& physics = Engine::GetInstance().GetPhysicsManager();
            physics.RemoveSoftBody(mBody);
            physics.ForgetCollisionObject(this);
            mBody->setUserPointer(nullptr);
            mIsAddedToWorld = false;
            delete mBody;
            mBody = nullptr;
        }
    }

    bool SoftBody::IsAddedToWorld() const {
        return mIsAddedToWorld;
    }

    void SoftBody::SetAddedToWorld(bool added) {
        mIsAddedToWorld = added;
    }

    btSoftBody* SoftBody::GetBody() const {
        return mBody;
    }


    void SoftBody::SetPosition(const glm::vec3& position) {
        if (!mBody) {
            return;
        }

        const btVector3 delta(position.x - mPosition.x, position.y - mPosition.y, position.z - mPosition.z);
        mBody->translate(delta);
        mPosition = position;
    }

    glm::vec3 SoftBody::GetPosition() const {
        return mPosition;
    }

    void SoftBody::ApplyForce(const glm::vec3& force) {
        if (mBody) {
            mBody->addForce(btVector3(force.x, force.y, force.z));
        }
    }

    void SoftBody::SetDamping(float damping) {
        if (mBody) {
            mBody->m_cfg.kDP = glm::clamp(damping, 0.0f, 1.0f);
        }
    }

    void SoftBody::SetStiffness(float linear, float angular, float volume) {
        if (!mBody || mBody->m_materials.size() == 0) {
            return;
        }

        btSoftBody::Material* material = mBody->m_materials[0];
        material->m_kLST               = glm::clamp(linear, 0.0f, 1.0f);
        material->m_kAST               = glm::clamp(angular, 0.0f, 1.0f);
        material->m_kVST               = glm::clamp(volume, 0.0f, 1.0f);
    }

    float SoftBody::GetWidth() const {
        return mWidth;
    }

    float SoftBody::GetHeight() const {
        return mHeight;
    }

    uint32_t SoftBody::GetSubdivisionsX() const {
        return mSubdivisionsX;
    }

    uint32_t SoftBody::GetSubdivisionsY() const {
        return mSubdivisionsY;
    }
} // namespace golias
