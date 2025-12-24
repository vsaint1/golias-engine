#include "scene/scene.h"

namespace golias {

    GameObject* Scene::CreateObject(const std::string& name, GameObject* pParent) {
        auto obj = new GameObject();
        obj->SetName(name);
        SetParent(obj, pParent);

        spdlog::info("Scene::CreateObject Created GameObject with Name '{}'", name);

        return obj;
    }

    const std::vector<std::unique_ptr<GameObject>>& Scene::GetGameObjects() const {
        return game_objects;
    }

    void Scene::Update(float deltaTime) {

        for (auto it = game_objects.begin(); it != game_objects.end();) {
            if ((*it)->IsAlive()) {

                (*it)->Update(deltaTime);
                ++it;

            } else {
                it = game_objects.erase(it);
            }
        }
    }

    void Scene::Clear() {
        spdlog::info("Scene::Clear Clearing Scene with {} GameObjects", game_objects.size());
        game_objects.clear();
    }


    bool Scene::SetParent(GameObject* pGameObject, GameObject* pParent) {
        bool result        = false;
        auto currentParent = pGameObject->GetParent();

        if (pParent == nullptr) {
            if (currentParent != nullptr) {
                auto it = std::find_if(currentParent->children.begin(), currentParent->children.end(),
                                       [pGameObject](const std::unique_ptr<GameObject>& el) { return el.get() == pGameObject; });

                if (it != currentParent->children.end()) {
                    game_objects.push_back(std::move(*it));
                    pGameObject->parent = nullptr;
                    currentParent->children.erase(it);
                    result = true;
                }
            }
            // No parent currently. This can be in 2 cases
            // 1. The object is in the scene root
            // 2. The object has been just created
            else {
                auto it = std::find_if(game_objects.begin(), game_objects.end(),
                                       [pGameObject](const std::unique_ptr<GameObject>& el) { return el.get() == pGameObject; });

                if (it == game_objects.end()) {
                    std::unique_ptr<GameObject> objHolder(pGameObject);
                    game_objects.push_back(std::move(objHolder));
                    result = true;
                }
            }
        }
        // We are trying to add it as a child of another object
        else {
            if (currentParent != nullptr) {
                auto it = std::find_if(currentParent->children.begin(), currentParent->children.end(),
                                       [pGameObject](const std::unique_ptr<GameObject>& el) { return el.get() == pGameObject; });

                if (it != currentParent->children.end()) {
                    bool found          = false;
                    auto currentElement = pParent;
                    while (currentElement) {
                        if (currentElement == pGameObject) {
                            found = true;
                            break;
                        }
                        currentElement = currentElement->GetParent();
                    }

                    if (!found) {
                        pParent->children.push_back(std::move(*it));
                        pGameObject->parent = pParent;
                        currentParent->children.erase(it);
                        result = true;
                    }
                }
            }
            // No parent currently. This can be in 2 cases
            // 1. The object is in the scene root
            // 2. The object has been just created
            else {
                auto it = std::find_if(game_objects.begin(), game_objects.end(),
                                       [pGameObject](const std::unique_ptr<GameObject>& el) { return el.get() == pGameObject; });

                // The object has been hust created
                if (it == game_objects.end()) {
                    std::unique_ptr<GameObject> objHolder(pGameObject);
                    pParent->children.push_back(std::move(objHolder));
                    pGameObject->parent = pParent;
                    result              = true;
                } else {
                    bool found          = false;
                    auto currentElement = pParent;
                    while (currentElement) {
                        if (currentElement == pGameObject) {
                            found = true;
                            break;
                        }
                        currentElement = currentElement->GetParent();
                    }

                    if (!found) {
                        pParent->children.push_back(std::move(*it));
                        pGameObject->parent = pParent;
                        game_objects.erase(it);
                        result = true;
                    }
                }
            }
        }

        return result;
    }

    void Scene::SetMainCamera(GameObject* pCamera) {
        main_camera = pCamera;
    }

    GameObject* Scene::GetMainCamera() const {
        return main_camera;
    }
    
}; // namespace golias
