#include "scene/component.h"

namespace golias {
    size_t Component::next_type_id = 1;

    GameObject* Component::GetOwner() const {
        return owner;
    }

    void Component::SetOwner(GameObject* pOwner) {
        owner = pOwner;
    }


} // namespace golias
