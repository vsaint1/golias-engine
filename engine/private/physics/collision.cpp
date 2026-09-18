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

    short parse_collision_bitmask(const Json& value) {
        short mask = 0;

        if (value.is_array()) {
            for (const auto& layer : value) {
                const int index = layer.get<int>();
                if (index < 0 || index > 15) {
                    GOLIAS_LOG_ERROR("Collision layer index out of range 0-15. Got %d.", index);
                    continue;
                }

                mask = static_cast<short>(mask | static_cast<short>(1 << index));
            }

            return mask;
        }

        const int index = value.get<int>();
        if (index < 0 || index > 15) {
            GOLIAS_LOG_ERROR("Collision layer index out of range 0-15. Got %d.", index);
            return 0;
        }

        return static_cast<short>(1 << index);
    }

} // namespace golias
