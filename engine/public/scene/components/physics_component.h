#pragma once
#include "component.h"
#include "physics/collision.h"

namespace golias {

    class RigidBody;

    class PhysicsComponent : public Component, public ContactListener {
        COMPONENT(PhysicsComponent)
    public:
        PhysicsComponent() = default;
        PhysicsComponent(const Ref<RigidBody>& rigidBody);
        ~PhysicsComponent() override;

        bool LoadProperties(const Json& properties) override;
        
        void Start() override;

        void Update(float deltaTime) override;

        virtual void OnCollisionEnter(const Collision& collision) override;
        virtual void OnCollisionExit(const Collision& collision) override;

        RigidBody* GetRigidBody() const;
        void SetRigidBody(const Ref<RigidBody>& rigidBody);

    private:
        Ref<RigidBody> mRigidBody = nullptr;
        bool mStarted = false;
    };

} // namespace golias
