#pragma once


#include "scene/component.h"
#include "physics/3d/rigid_body.h"
#include <memory>

namespace golias {


    class PhysicsComponent : public Component {
        COMPONENT(PhysicsComponent)
    public:
        PhysicsComponent()                    = default;
        ~PhysicsComponent() override;
        PhysicsComponent(const std::shared_ptr<RigidBody>& pRigidBody);

        RigidBody* GetRigidBody() const;
        void SetRigidBody(std::shared_ptr<RigidBody> pRigidBody);

        void Start() override;
        void Update(float deltaTime) override;

        void LoadProperties(const nlohmann::json& json) override;
    private:
        std::shared_ptr<RigidBody> rigidBody = nullptr;
    };

} // namespace golias
