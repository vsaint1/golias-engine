#pragma once

#include "game_object.h"

#include <spdlog/spdlog.h>


namespace golias {


    class Scene {
    public:
        Scene()  = default;
        ~Scene() = default;

        GameObject* CreateObject(const std::string& name, GameObject* pParent = nullptr);

        template <typename T, typename = typename std::enable_if_t<std::is_base_of_v<GameObject, T>>>
        T* CreateObject(const std::string& name, GameObject* pParent = nullptr);

        bool SetParent(GameObject* pGameObject, GameObject* pParent);

        const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const;

        void Update(float deltaTime);

        void Clear();

    private:
        std::vector<std::unique_ptr<GameObject>> game_objects;
    };

    template <typename T, typename>
    T* Scene::CreateObject(const std::string& name, GameObject* pParent) {
        auto obj = new T();
        obj->SetName(name);
        SetParent(obj, pParent);

        spdlog::info("Created GameObject of type {} with Name '{}'", typeid(T).name(), name);
        return obj;
    }

}; // namespace golias
