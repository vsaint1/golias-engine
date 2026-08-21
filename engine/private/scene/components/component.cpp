#include "scene/components/component.h"
#include "scene/game_object.h"

namespace golias {

    size_t Component::sNextComponentID = 0;

    GameObject* Component::GetOwner() const {
        return mOwner;
    }
}
