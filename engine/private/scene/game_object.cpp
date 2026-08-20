#include "scene/game_object.h"

namespace golias {

    void GameObject::Update(float deltaTime) {

        for (auto it = mChildren.begin(); it != mChildren.end();) {
            GameObject* child = it->get();
           
            if (child->IsAlive()) {

                child->Update(deltaTime);
                ++it;
                
            } else {
                it = mChildren.erase(it);
            }

            
        }
    }

    void GameObject::SetName(CString name) {
        mName = name;
    }

    String GameObject::GetName() const {
        return mName;
    }

    void GameObject::SetParent(GameObject* parent) {
        mParent = parent;
    }

    GameObject* GameObject::GetParent() const {
        return mParent;
    }

    void GameObject::Destroy() {
        mIsAlive = false;
    }

    bool GameObject::IsAlive() const {
        return mIsAlive;
    }
} // namespace golias
