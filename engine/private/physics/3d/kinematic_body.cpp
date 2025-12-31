#include "physics/3d/kinematic_body.h"

#include "core/engine.h"
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

namespace golias {
    KinematicCharacterController::KinematicCharacterController(float height, float radius, const glm::vec3& position)
        : _height(height), _radius(radius) {

        const auto physicsWorld = Engine::GetInstance().GetPhysicsManager().GetPhyisicsWorld();
        auto capsule            = new btCapsuleShape(btScalar(radius), btScalar(height));

        ghostObject = std::make_unique<btPairCachingGhostObject>();
        btTransform startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(btVector3(btScalar(position.x), btScalar(position.y), btScalar(position.z)));
        ghostObject->setWorldTransform(startTransform);
        ghostObject->setCollisionShape(capsule);
        ghostObject->setCollisionFlags(ghostObject->getCollisionFlags() | btCollisionObject::CF_CHARACTER_OBJECT);

        // ghostPairCallback = std::make_unique<btGhostPairCallback>();

        physicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

        const btScalar stepHeight = btScalar(0.35f);
        controller                = std::make_unique<btKinematicCharacterController>(ghostObject.get(), capsule, stepHeight);

        controller->setMaxSlope(btRadians(maxSlopeDegrees));
        controller->setGravity(physicsWorld->getGravity());
        controller->setMaxPenetrationDepth(btScalar(0.05f));
        controller->setJumpSpeed(btScalar(7.0f));
        controller->setFallSpeed(btScalar(20.0f));

        controller->setStepHeight(stepHeight);

        controller->setUpInterpolate(true);
        controller->setUseGhostSweepTest(true);

        physicsWorld->addCollisionObject(ghostObject.get(), btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::AllFilter);

        physicsWorld->addAction(controller.get());
    }

    KinematicCharacterController::~KinematicCharacterController() {
        const auto physicsWorld = Engine::GetInstance().GetPhysicsManager().GetPhyisicsWorld();
        if (controller) {
            physicsWorld->removeAction(controller.get());
        }

        if (ghostObject) {
            physicsWorld->removeCollisionObject(ghostObject.get());
        }
    }

    glm::vec3 KinematicCharacterController::GetPosition() const {
        const auto& transform = ghostObject->getWorldTransform();
        // const glm::vec3 offset = {0.0f, _height + 0.5f + _radius, 0.0f};
        btVector3 origin = transform.getOrigin();
        return glm::vec3(origin.getX(), origin.getY(), origin.getZ());
    }

    void KinematicCharacterController::SetPosition(const glm::vec3& pos) {
        btTransform transform = ghostObject->getWorldTransform();
        transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
        ghostObject->setWorldTransform(transform);
        controller->warp(btVector3(pos.x, pos.y, pos.z));
    }

    glm::quat KinematicCharacterController::GetRotation() const {
        const auto& transform   = ghostObject->getWorldTransform();
        const btQuaternion& rot = transform.getRotation();
        return glm::quat(rot.getW(), rot.getX(), rot.getY(), rot.getZ());
    }

    void KinematicCharacterController::SetRotation(const glm::quat& rot) {
        btTransform transform = ghostObject->getWorldTransform();
        transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));
        ghostObject->setWorldTransform(transform);
    }

    void KinematicCharacterController::Move(const glm::vec3& direction) {

        btVector3 dir(btScalar(direction.x), btScalar(direction.y), btScalar(direction.z));
        controller->setWalkDirection(dir);
    }

    void KinematicCharacterController::Jump(const glm::vec3& force) {
        if (controller->onGround()) {
            btVector3 impulse(btScalar(force.x), btScalar(force.y), btScalar(force.z));
            controller->jump(impulse);
        }
    }

    bool KinematicCharacterController::OnGround() const {
        return controller->onGround();
    }

    void KinematicCharacterController::SetMaxSlope(float value) {
        maxSlopeDegrees = value;
        controller->setMaxSlope(btRadians(maxSlopeDegrees));
    }

    void KinematicCharacterController::SetFallSpeed(float value) {
        fallSpeed = value;
        controller->setFallSpeed(btScalar(fallSpeed));
    }

    void KinematicCharacterController::SetJumpSpeed(float value) {
        jumpSpeed = value;
        controller->setJumpSpeed(btScalar(jumpSpeed));
    }

    float KinematicCharacterController::GetMaxSlope() const {
        return maxSlopeDegrees;
    }

    float KinematicCharacterController::GetFallSpeed() const {
        return fallSpeed;
    }

    float KinematicCharacterController::GetJumpSpeed() const {
        return jumpSpeed;
    }
} // namespace golias
