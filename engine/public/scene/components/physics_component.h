#pragma once
#include "component.h"

namespace golias {

    class RigidBody;

    class PhysicsComponent : public Component {
        COMPONENT(PhysicsComponent)
    public:
        PhysicsComponent() = default;
        PhysicsComponent(const Ref<RigidBody>& rigidBody);

        bool LoadProperties(const Json& properties) override;
        
        void Start() override;

        void Update(float deltaTime) override;

        RigidBody* GetRigidBody() const;
        void SetRigidBody(const Ref<RigidBody>& rigidBody);

    private:
        Ref<RigidBody> mRigidBody = nullptr;
    };

} // namespace golias
