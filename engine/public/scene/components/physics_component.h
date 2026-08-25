#pragma once
#include "component.h"
#include "stdafx.h"

namespace golias {

    class RigidBody;

    class PhysicsComponent : public Component {
        COMPONENT(PhysicsComponent)
    public:
        PhysicsComponent() = default;
        PhysicsComponent(const Ref<RigidBody>& rigidBody);

        void Start() override;

        void Update(float deltaTime) override;

    private:
        Ref<RigidBody> mRigidBody = nullptr;
    };

} // namespace golias
