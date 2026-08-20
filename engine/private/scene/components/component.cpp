#include "scene/game_object.h"

namespace golias {

    GameObject* Component::GetOwner() const {
        return mOwner;
    }
}