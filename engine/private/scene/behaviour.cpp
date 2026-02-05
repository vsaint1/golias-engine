#include "scene/behaviour.h"

namespace golias {

    void Behaviour::Awake() {
        // Override in derived classes
    }

    void Behaviour::Start() {
        // Override in derived classes
    }

    void Behaviour::Update(float deltaTime) {
        // Override in derived classes
        // at the end to update components and children
    }

    void Behaviour::OnDestroy() {
        // Override in derived classes
    }

    void Behaviour::Destroy() {
        if (!IsAlive()) {
            return; 
        }

        OnDestroy();
        GameObject::Destroy();
    }

} // namespace golias
