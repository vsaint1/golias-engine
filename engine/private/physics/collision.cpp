#include "physics/collision.h"

#include "core/engine.h"

namespace golias {

    CollisonObjectType CollisionObject::GetCollisionObjectType() {
        return mType;
    }

    GameObject* CollisionObject::GetGameObject() const {
        return mOwner;
    }

    void CollisionObject::SetGameObject(GameObject* owner) {
        mOwner = owner;
    }

    void CollisionObject::AddContactListener(ContactListener* listener) {
        mContactListeners.push_back(listener);
    }

    void CollisionObject::RemoveContactListener(ContactListener* listener) {
        auto it = std::find(mContactListeners.begin(), mContactListeners.end(), listener);
        if (it != mContactListeners.end()) {
            mContactListeners.erase(it);
        }
    }

    void CollisionObject::DispatchContactEnter(const Collision& collision) {
        for (const auto& listener : mContactListeners) {
            if (listener) {
                listener->OnCollisionEnter(collision);
            }
        }
    }

    void CollisionObject::DispatchContactExit(const Collision& collision) {
        for (const auto& listener : mContactListeners) {
            if (listener) {
                listener->OnCollisionExit(collision);
            }
        }
    }

} // namespace golias
