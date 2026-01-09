#include "scene/scene.h"

#include "core/engine.h"
#include <SDL3/SDL_stdinc.h>

#pragma region COMPONENTS_3D
#include "scene/3d/animation_component.h"
#include "scene/3d/skeleton_animation_component.h"
#include "scene/3d/audio_component.h"
#include "scene/3d/audio_listener_component.h"
#include "scene/3d/camera_component.h"
#include "scene/3d/fp_controller_component.h"
#include "scene/3d/mesh_component.h"
#include "scene/3d/physics_component.h"
#pragma endregion

#pragma region 2D_COMPONENTS
#include "scene/2d/sprite_component_2d.h"
#pragma endregion

#pragma region UI_COMPONENTS
#include "scene/ui/canvas_component.h"
#include "scene/ui/text_component.h"
#pragma endregion

namespace golias {
    size_t Scene::next_type_id = 0;

    GameObject* Scene::CreateObject(const std::string& name, GameObject* pParent) {
        auto obj          = new GameObject();
        std::string fname = MakeUniqueName(name);
        obj->SetName(fname);
        obj->scene = this;

        if (isUpdating) {

            pending_objects.push_back({obj, pParent});
        } else {
            SetParent(obj, pParent);
        }


        spdlog::info("Scene::CreateObject Created GameObject with Name '{}'", fname);
        return obj;
    }

    GameObject* Scene::CreateObject(GameObject* pParent) {
        return CreateObject("GameObject", pParent);
    }

    GameObject* Scene::CreateObject(const std::string& type, const std::string& name, GameObject* pParent) {

        auto obj = ObjectRegistry::GetInstance().CreateObject(type);
        if (obj) {
            std::string fname = MakeUniqueName(name);
            obj->SetName(fname);
            obj->scene = this;

            if (isUpdating) {

                pending_objects.push_back({obj, pParent});
            } else {
                SetParent(obj, pParent);
            }

            spdlog::info("Scene::CreateObject Created GameObject of type {} with Name '{}'", type, fname);
            return obj;
        }

        spdlog::error("Scene::CreateObject Failed to create GameObject of unknown type: {}", type);

        return obj;
    }

    const std::vector<std::unique_ptr<GameObject>>& Scene::GetGameObjects() const {
        return game_objects;
    }

    void Scene::Update(float deltaTime) {

        game_objects.erase(std::remove_if(game_objects.begin(),
                                          game_objects.end(),
                                          [](const std::unique_ptr<GameObject>& obj) { return !obj->IsAlive(); }),
                           game_objects.end());


        for (auto& [obj, parent] : pending_objects) {
            SetParent(obj, parent);
        }

        pending_objects.clear();

        isUpdating = true;
        for (auto it = game_objects.begin(); it != game_objects.end();) {
            if ((*it)->IsAlive()) {
                (*it)->Update(deltaTime);
                ++it;
            } else {
                it = game_objects.erase(it);
            }
        }
        isUpdating = false;
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
                auto it = std::find_if(currentParent->children.begin(),
                                       currentParent->children.end(),
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
                auto it = std::find_if(game_objects.begin(), game_objects.end(), [pGameObject](const std::unique_ptr<GameObject>& el) {
                    return el.get() == pGameObject;
                });

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
                auto it = std::find_if(currentParent->children.begin(),
                                       currentParent->children.end(),
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
                auto it = std::find_if(game_objects.begin(), game_objects.end(), [pGameObject](const std::unique_ptr<GameObject>& el) {
                    return el.get() == pGameObject;
                });

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

    std::string Scene::MakeUniqueName(const std::string& baseName) {

        size_t count = 0;

        for (const auto& obj : game_objects) {
            if (!obj) {
                spdlog::warn("Scene::MakeUniqueName encountered null GameObject pointer");
                continue;
            }

            const std::string& existing = obj->GetName();

            if (existing == baseName) {

                count = SDL_max(count, size_t(1));
            } else if (existing.starts_with(baseName + "_")) {

                auto suffix = existing.substr(baseName.size() + 1);
                if (std::all_of(suffix.begin(), suffix.end(), ::isdigit)) {
                    count = SDL_max(count, std::stoul(suffix) + 1);
                }
            }
        }


        if (count == 0) {
            return baseName;
        }


        return std::format("{}_{}", baseName, count);
    }

    std::shared_ptr<Scene> Scene::Load(const std::string_view pPath) {

        const std::string content = Engine::GetInstance().GetFileSystem().LoadAssetFileText(pPath);
        if (content.empty()) {
            spdlog::error("Scene::Load Failed to load Scene file: {}", pPath);
            return nullptr;
        }

        nlohmann::json json = nlohmann::json::parse(content);

        if (json.is_null() || json.empty()) {
            spdlog::error("Scene::Load Failed to parse Scene file: {}", pPath);
            return nullptr;
        }

        auto scene                   = std::make_shared<Scene>();
        const std::string sceneName  = json.value("name", "UnnamedScene");
        const std::string versionStr = json.value("version", "0.0.0");

        scene->SetName(sceneName);

        spdlog::info("Scene::Load Loading Scene '{}' from file: {}", sceneName, pPath);

        if (json.contains("objects") && json["objects"].is_array()) {

            for (const auto& object : json["objects"]) {
                scene->LoadObject(object, nullptr);
            }
        }

        if (json.contains("main_camera")) {
            const std::string mainCameraName = json.value("main_camera", "");

            std::string cameraObjName = json.value("main_camera", "");
            for (const auto& child : scene->game_objects) {
                if (auto object = child->FindChildByName(cameraObjName)) {
                    spdlog::info("Scene::Load Setting main camera to '{}'", cameraObjName);
                    scene->SetMainCamera(object);
                    break;
                }
            }
        }

        spdlog::info("Scene::Load Successfully loaded Scene '{}' with {} GameObjects", sceneName, scene->GetGameObjects().size());
        return scene;
    }

    void Scene::LoadObject(const nlohmann::json& object, GameObject* pParent) {

        const std::string name = object.value("name", "GameObject");

        GameObject* gameObject = nullptr;

        if (object.contains("type")) {
            const std::string type = object.value("type", "");

            if (type == "model") {
                const std::string modelPath = object.value("path", "");
                spdlog::info("Scene::LoadObject Loading Model GameObject '{}' from path '{}'", name, modelPath);
                gameObject = Model::Load(modelPath, this);

                if (gameObject) {

                    gameObject->SetParent(pParent);

                    gameObject->SetName(name);
                }
            } else {

                gameObject = CreateObject(type, name, pParent);
            }

        } else {
            gameObject = CreateObject(name, pParent);
        }

        if (!gameObject) {
            spdlog::warn("Scene::LoadObject Failed to create GameObject of type '{}' with name '{}'", object.value("type", ""), name);
            return;
        }

        if (object.contains("position")) {
            const auto pos     = object["position"];
            glm::vec3 position = glm::vec3(0.0f);
            position.x         = pos.value("x", 0.0f);
            position.y         = pos.value("y", 0.0f);
            position.z         = pos.value("z", 0.0f);
            gameObject->SetPosition(position);

            spdlog::warn("LoadObject '{}': Setting position ({},{},{}), parent={}",
                         name,
                         position.x,
                         position.y,
                         position.z,
                         gameObject->GetParent() ? gameObject->GetParent()->GetName() : "null");
        }

        if (object.contains("rotation")) {
            const auto rot     = object["rotation"];
            glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            rotation.x         = rot.value("x", 0.0f);
            rotation.y         = rot.value("y", 0.0f);
            rotation.z         = rot.value("z", 0.0f);
            rotation.w         = rot.value("w", 1.0f);
            gameObject->SetRotation(rotation);
        }

        if (object.contains("scale")) {
            const auto scl  = object["scale"];
            glm::vec3 scale = glm::vec3(1.0f);
            scale.x         = scl.value("x", 1.0f);
            scale.y         = scl.value("y", 1.0f);
            scale.z         = scl.value("z", 1.0f);
            gameObject->SetScale(scale);
        }

        gameObject->LoadProperties(object);

        if (object.contains("components") && object["components"].is_array()) {
            for (const auto& comp : object["components"]) {
                const std::string type = comp.value("type", "");

                Component* component = ComponentRegistry::GetInstance().CreateComponent(type);
                if (component) {

                    spdlog::info("Scene::LoadObject Adding Component of type '{}' to GameObject '{}'", type, name);
                    component->LoadProperties(comp);
                    gameObject->AddComponent(component);
                } else {
                    spdlog::warn("Scene::LoadObject Failed to create Component of type '{}' for GameObject '{}'", type, name);
                }
            }
        }

        if (object.contains("children") && object["children"].is_array()) {
            for (const auto& child : object["children"]) {
                LoadObject(child, gameObject);
            }
        }


        gameObject->Start();
    }

    void Scene::SetName(const std::string_view pName) {
        name = pName;
    }

    const std::string& Scene::GetName() const {
        return name;
    }

    void Scene::RegisterTypes() {
        MeshComponent::Register();
        CameraComponent::Register();
        FirstPersonControllerComponent::Register();
        AnimationComponent::Register();
        PhysicsComponent::Register();
        AudioComponent::Register();
        AudioListenerComponent::Register();
        SkeletonAnimationComponent::Register();

        SpriteComponent2D::Register();


        WidgetComponent::Register();
        CanvasComponent::Register();
        TextWidgetComponent::Register();
    }

} // namespace golias
