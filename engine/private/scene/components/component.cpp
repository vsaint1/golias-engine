#include "scene/components/component.h"
#include "scene/game_object.h"

namespace golias {

    size_t Component::sNextComponentID = 0;

    void Component::Start() {
       
    }

    GameObject* Component::GetOwner() const {
        return mOwner;
    }
}
