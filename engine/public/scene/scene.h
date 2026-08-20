#pragma once

#include "game_object.h"


namespace golias {


    class Scene {

    public:
        Scene() = default;

        GameObject* CreateGameObject(CString name, GameObject* parent = nullptr);

        template <typename T, typename = typename std::enable_if<std::is_base_of<GameObject, T>::value>::type>
        T* CreateGameObject(CString name, GameObject* parent = nullptr) {
            T* gameObject = new T();
            gameObject->SetName(name);
         
            if (!SetParent(gameObject, parent)) {
                GOLIAS_LOG_ERROR("Failed to set parent for GameObject '%s'.", name.data());
                return nullptr;
            }

            return gameObject;
        }

        bool SetParent(GameObject* object, GameObject* parent);

        void Update(float deltaTime);

        void Clear();

    private:
        std::vector<std::unique_ptr<GameObject>> mObjects = {};
    };
} // namespace golias
