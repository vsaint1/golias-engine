#include "scene/scene.h"

#include "core/engine.h"
#include "scene/components/animation_component.h"
#include "scene/components/audio_listener_component.h"
#include "scene/components/audio_source_component.h"
#include "scene/components/camera_component.h"
#include "scene/components/light_component.h"
#include "scene/components/physics_component.h"
#include "scene/components/player_controller_component.h"
#include "scene/components/static_mesh_component.h"

namespace golias {

    void Scene::RegisterTypes() {

        StaticMeshComponent::Register();
        CameraComponent::Register();
        AnimationComponent::Register();
        PlayerControllerComponent::Register();
        PhysicsComponent::Register();
        LightComponent::Register();
        AudioSourceComponent::Register();
        AudioListenerComponent::Register();
    }

    Ref<Scene> Scene::Load(CString path) {

        const String contents = Engine::GetInstance().GetFileSystem().LoadAssetFileText(path);

        if (contents.empty()) {
            GOLIAS_LOG_ERROR("Failed to load scene from path: %s", path.data());
            return nullptr;
        }

        Json json = Json::parse(contents, nullptr, false);

        if (json.is_discarded()) {
            GOLIAS_LOG_ERROR("Failed to parse scene JSON from path: %s", path.data());
            return nullptr;
        }

        const String sceneName = json.value("name", "UnnamedScene");

        Ref<Scene> scene = std::make_shared<Scene>();
        scene->SetName(sceneName);

        if (json.contains("objects") && json["objects"].is_array()) {
            for (const auto& objectData : json["objects"]) {
                scene->LoadObject(objectData, nullptr);
            }

        } else {
            GOLIAS_LOG_WARN("Scene '%s' does not contain an 'objects' array.", sceneName.data());
        }

        bool foundCamera = false;
        if (json.contains("camera") && json["camera"].is_string()) {
            String cameraName = json["camera"].get<String>();

            for (const auto& child : scene->mObjects) {
                if (GameObject* camera = child->FindChildByName(cameraName)) {
                    scene->mMainCamera = camera;
                    foundCamera        = true;
                    break;
                }
            }
        }

        if (!foundCamera) {
            GOLIAS_LOG_WARN("Scene '%s' does not have a valid main camera specified.", sceneName.data());
        }

        GOLIAS_LOG_INFO("Scene '%s' loaded successfully from path: %s", sceneName.data(), path.data());

        return scene;
    }

    void Scene::LoadObject(const Json& objectData, GameObject* parent) {
        if (!objectData.is_object()) {
            GOLIAS_LOG_ERROR("Invalid object data: expected an object.");
            return;
        }

        String name            = objectData.value("name", "Unnamed");
        GameObject* gameObject = nullptr;

        if (objectData.contains("type") && objectData["type"].is_string()) {
            String type = objectData["type"].get<String>();

            if (type == "Model") {
                String path = objectData.value("path", "");
                if (!path.empty()) {
                    gameObject = GameObject::Load(path, this, name);

                    if (gameObject) {
                        if (!SetParent(gameObject, parent)) {
                            GOLIAS_LOG_ERROR("Failed to set parent for GameObject '%s'.", name.data());
                            gameObject = nullptr;
                        } else {
                            gameObject->SetName(name);
                        }
                    } else {
                        GOLIAS_LOG_ERROR("Failed to load model for GameObject '%s' from path: %s", name.data(), path.data());
                    }

                } else {
                    GOLIAS_LOG_ERROR("Model object '%s' is missing a 'path' property.", name.data());
                }
            } else {
                gameObject = ObjectRegistry::GetInstance().CreateObject(type);
                if (gameObject) {
                    gameObject->SetName(name);
                    if (!SetParent(gameObject, parent)) {
                        GOLIAS_LOG_ERROR("Failed to set parent for GameObject '%s'.", name.data());
                        delete gameObject;
                        gameObject = nullptr;
                    }
                } else {
                    GOLIAS_LOG_ERROR("Unknown GameObject type '%s'.", type.data());
                }
            }

        } else {
            gameObject = CreateGameObject(name, parent);
        }

        if (!gameObject) {
            GOLIAS_LOG_ERROR("Failed to create GameObject '%s'.", name.data());
            return;
        }

        if (objectData.contains("position")) {

            Json position = objectData["position"];
            glm::vec3 pos;
            pos.x = position.value("x", 0.0f);
            pos.y = position.value("y", 0.0f);
            pos.z = position.value("z", 0.0f);
            gameObject->SetPosition(pos);
        }

        if (objectData.contains("rotation")) {
            Json rotation = objectData["rotation"];
            glm::vec3 rot;
            rot.x = rotation.value("x", 0.0f);
            rot.y = rotation.value("y", 0.0f);
            rot.z = rotation.value("z", 0.0f);
            gameObject->SetRotation(rot);
        }

        if (objectData.contains("scale")) {
            Json scale = objectData["scale"];
            glm::vec3 scl;
            scl.x = scale.value("x", 1.0f);
            scl.y = scale.value("y", 1.0f);
            scl.z = scale.value("z", 1.0f);
            gameObject->SetScale(scl);
        }

        gameObject->LoadProperties(objectData);

        if (objectData.contains("components") && objectData["components"].is_array()) {
            const Json& components = objectData["components"];
            for (const auto& componentData : components) {
                if (componentData.contains("type") && componentData["type"].is_string()) {
                    String componentType = componentData["type"].get<String>();

                    if (Component* component = ComponentRegistry::GetInstance().CreateComponent(componentType)) {

                        if (component->LoadProperties(componentData)) {
                            gameObject->AddComponent(component);
                        } else {
                            GOLIAS_LOG_ERROR(
                                "Failed to load properties for component '%s' on GameObject '%s'.", componentType.data(), name.data());
                            delete component;
                        }

                    } else {
                        GOLIAS_LOG_ERROR("Unknown component type '%s' for GameObject '%s'.", componentType.data(), name.data());
                    }
                }
            }
        }


        if (objectData.contains("children") && objectData["children"].is_array()) {
            const Json& children = objectData["children"];
            for (const auto& childData : children) {
                LoadObject(childData, gameObject);
            }
        }


        gameObject->Start();
    }

    void Scene::PrintTree() {
        GOLIAS_LOG_INFO("Scene: %s", mName.c_str());

        for (const auto& obj : mObjects) {
            GOLIAS_LOG_INFO("GameObject Name: %s ", obj->GetName().data());
            for (const auto& component : obj->mComponents) {
                GOLIAS_LOG_INFO("  Component: %s (TypeId: %zu)", component->GetTypeName(), component->GetTypeId());
            }

            for (const auto& child : obj->mChildren) {
                GOLIAS_LOG_INFO("  Child: %s", child->GetName().data());
                for (const auto& childComponent : child->mComponents) {
                    GOLIAS_LOG_INFO("    Component: %s (TypeId: %zu)", childComponent->GetTypeName(), childComponent->GetTypeId());
                }
            }
        }
    }

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

    GameObject* Scene::GetMainCamera() const {
        return mMainCamera;
    }

    void Scene::SetMainCamera(GameObject* camera) {
        mMainCamera = camera;
    }

    CString Scene::GetName() const {
        return mName.c_str();
    }

    void Scene::SetName(CString name) {
        mName = name;
    }

    GameObject* Scene::FindGameObjectByName(CString name) const {
        for (const auto& obj : mObjects) {
            if (obj->GetName() == name) {
                return obj.get();
            }
        }

        return nullptr;
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
