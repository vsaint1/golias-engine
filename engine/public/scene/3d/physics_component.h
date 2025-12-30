#pragma once


#include "scene/component.h"
#include "physics/3d/rigid_body.h"
#include <memory>

namespace golias {


    class PhysicsComponent : public Component {
        COMPONENT(PhysicsComponent)
    public:
        PhysicsComponent(const std::shared_ptr<RigidBody>& pRigidBody);

        RigidBody* GetRigidBody() const;

        void Start() override;
        void Update(float deltaTime) override;
    private:
        PhysicsComponent()                    = default;
        std::shared_ptr<RigidBody> rigid_body = nullptr;
    };

} // namespace golias
