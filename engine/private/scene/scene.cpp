#include "scene/scene.h"

namespace golias {

    GameObject* Scene::CreateGameObject(CString name, GameObject* parent) {
        GameObject* gameObject = new GameObject();

        gameObject->SetName(name);

        if (!SetParent(gameObject, parent)) {
            GOLIAS_LOG_ERROR("Failed to set parent for GameObject '%s'.", name.data());
            return nullptr;
        }

        return gameObject;
    }


    bool Scene::SetParent(GameObject* object, GameObject* parent) {

        auto find_game_object = [](std::vector<std::unique_ptr<GameObject>>& container, GameObject* target) {
            return std::find_if(
                container.begin(), container.end(), [target](const std::unique_ptr<GameObject>& el) { return el.get() == target; });
        };

        auto is_ancestor_of = [](GameObject* potential_ancestor, GameObject* node) {
            for (GameObject* current = node; current != nullptr; current = current->GetParent()) {
                if (current == potential_ancestor) {
                    return true;
                }
            }

            return false;
        };

        auto move_game_object = [](std::vector<std::unique_ptr<GameObject>>& source,
                                   std::vector<std::unique_ptr<GameObject>>::iterator it,
                                   std::vector<std::unique_ptr<GameObject>>& dest,
                                   GameObject* obj,
                                   GameObject* new_parent) {
            dest.push_back(std::move(*it));
            obj->mParent = new_parent;
            source.erase(it);
        };

        GameObject* currentParent = object->GetParent();
        auto& sourceContainer     = currentParent ? currentParent->mChildren : mObjects;
        auto sourceIt             = find_game_object(sourceContainer, object);
        const bool isTracked      = (sourceIt != sourceContainer.end());

        if (parent == nullptr) {
            if (currentParent != nullptr) {
                if (!isTracked) {
                    return false; // inconsistent state, shouldn't happen
                }

                move_game_object(sourceContainer, sourceIt, mObjects, object, nullptr);
                return true;
            }

            // Already parentless: adopt if newly created, no-op if already in root
            if (isTracked) {
                return false;
            }

            mObjects.emplace_back(object);
            return true;
        }

        // parent != nullptr — guard against creating a cycle
        if (isTracked && is_ancestor_of(object, parent)) {
            return false;
        }

        if (!isTracked) {

            if (currentParent != nullptr) {
                return false; // inconsistent state
            }

            parent->mChildren.emplace_back(object);
            object->mParent = parent;
            return true;
        }

        move_game_object(sourceContainer, sourceIt, parent->mChildren, object, parent);
        return true;
    }


    void Scene::Update(float deltaTime) {
        for (auto it = mObjects.begin(); it != mObjects.end();) {
            GameObject* obj = it->get();

            if (obj->IsAlive()) {

                obj->Update(deltaTime);
                ++it;

            } else {
                it = mObjects.erase(it);
            }
        }
    }

    void Scene::Clear() {
        mObjects.clear();
    }
} // namespace golias
