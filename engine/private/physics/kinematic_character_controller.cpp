#include "physics/kinematic_character_controller.h"

#include "core/engine.h"
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <btBulletDynamicsCommon.h>

namespace golias {
    KinematicCharacterController::KinematicCharacterController(float radius, float height) : mHeight(height), mRadius(radius) {

        btDynamicsWorld* world = Engine::GetInstance().GetPhysicsManager().GetWorld();

        btCapsuleShape* capsuleShape = new btCapsuleShape(mRadius, mHeight);

        mGhostObject = new btPairCachingGhostObject();

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(0, 0, 0));

        mGhostObject->setWorldTransform(transform);
        mGhostObject->setCollisionShape(capsuleShape);
        mGhostObject->setCollisionFlags(mGhostObject->getCollisionFlags() | btCollisionObject::CF_CHARACTER_OBJECT);
        mGhostObject->setUserPointer(static_cast<CollisionObject*>(this));

        world->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

        const btScalar kStepHeight = 0.35f;
        btConvexShape* convexShape = static_cast<btConvexShape*>(mGhostObject->getCollisionShape());

        mController = new btKinematicCharacterController(mGhostObject, convexShape, kStepHeight);
        mController->setMaxSlope(btRadians(50.0f));
        mController->setGravity(world->getGravity());

        world->addCollisionObject(
            mGhostObject, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);

        world->addAction(mController);
    }

    KinematicCharacterController::~KinematicCharacterController() {
        btDynamicsWorld* world = Engine::GetInstance().GetPhysicsManager().GetWorld();

        Engine::GetInstance().GetPhysicsManager().ForgetCollisionObject(this);

        if (mController) {
            world->removeAction(mController);
        }

        if (mGhostObject) {
            world->removeCollisionObject(mGhostObject);
        }

        delete mController;
        mController = nullptr;

        if (mGhostObject) {
            delete mGhostObject->getCollisionShape();
            delete mGhostObject;
            mGhostObject = nullptr;
        }
    }

    glm::vec3 KinematicCharacterController::GetPosition() const {
        const btVector3& origin = mGhostObject->getWorldTransform().getOrigin();
        const glm::vec3 offset  = glm::vec3(0.0f, mHeight * 0.5f, 0.0f);
        return glm::vec3(origin.getX(), origin.getY(), origin.getZ()) + offset;
    }

    glm::quat KinematicCharacterController::GetRotation() const {
        const btQuaternion& rotation = mGhostObject->getWorldTransform().getRotation();
        return glm::quat(rotation.getW(), rotation.getX(), rotation.getY(), rotation.getZ());
    }

    void KinematicCharacterController::SetPosition(const glm::vec3& position) {
        btTransform transform = mGhostObject->getWorldTransform();
        transform.setOrigin(btVector3(position.x, position.y - mHeight * 0.5f, position.z));
        mController->warp(transform.getOrigin());
    }

    void KinematicCharacterController::Walk(const glm::vec3& direction) {
        mController->setWalkDirection(btVector3(direction.x, direction.y, direction.z));
    }

    void KinematicCharacterController::Jump(const glm::vec3& direction) {
        if (!mController->canJump()) {
            return;
        }

        mController->jump(btVector3(direction.x, direction.y, direction.z));
    }

    bool KinematicCharacterController::IsOnGround() const {
        return mController->onGround();
    }
} // namespace golias
