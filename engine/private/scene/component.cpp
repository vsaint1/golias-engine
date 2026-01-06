#include "scene/component.h"

namespace golias {
    size_t Component::next_type_id = 1;

    GameObject* Component::GetOwner() const {
        return owner;
    }

    void Component::SetOwner(GameObject* pOwner) {
        owner = pOwner;
    }

    void Component::Start() {
    }

    void Component::Update(float deltaTime) {
    }

    void Component::LoadProperties(const nlohmann::json& json) {
    }

} // namespace golias
