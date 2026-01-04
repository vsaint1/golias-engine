#pragma once

#include "game_object.h"
#include <spdlog/spdlog.h>

#include <json.hpp>


namespace golias {


    class Scene {
    public:
        Scene()  = default;
        ~Scene() = default;

        static void RegisterTypes();

        GameObject* CreateObject(const std::string& name, GameObject* pParent = nullptr);
        GameObject* CreateObject(GameObject* pParent = nullptr);
        GameObject* CreateObject(const std::string& type, const std::string& name, GameObject* pParent = nullptr);

        template <typename T, typename = typename std::enable_if_t<std::is_base_of_v<GameObject, T>>>
        T* CreateObject(const std::string& name, GameObject* pParent = nullptr);

        bool SetParent(GameObject* pGameObject, GameObject* pParent);

        const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const;

        void Update(float deltaTime);

        void Clear();

        void SetMainCamera(GameObject* pCamera);
        GameObject* GetMainCamera() const;

        static std::shared_ptr<Scene> Load(const std::string_view pPath);

        void SetName(const std::string_view pName);
        const std::string& GetName() const;

    private:
        void LoadObject(const nlohmann::json& object, GameObject* pParent);

        std::vector<std::unique_ptr<GameObject>> game_objects;
        std::vector<std::pair<GameObject*, GameObject*>> pending_objects;
        bool isUpdating = false;

        GameObject* main_camera = nullptr;
        static size_t next_type_id;

        std::string name;

        std::string MakeUniqueName(const std::string& baseName);
    };

    template <typename T, typename>
    T* Scene::CreateObject(const std::string& name, GameObject* pParent) {
        auto obj = new T();

        std::string fname = MakeUniqueName(name);

        obj->SetName(fname);
        obj->scene = this;

        if (isUpdating) {

            pending_objects.push_back({obj, pParent});
        } else {
            SetParent(obj, pParent);
        }

        spdlog::info("Created GameObject of type {} with Name '{}'", typeid(T).name(), fname);
        return obj;
    }

}; // namespace golias
