#pragma once

#include "game_object.h"


namespace golias {


    class Scene {

    public:
        Scene() = default;

        static Ref<Scene> Load(CString path);

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

        GameObject* FindGameObjectByName(CString name) const;

        bool SetParent(GameObject* object, GameObject* parent);

        GameObject* GetMainCamera() const;
        void SetMainCamera(GameObject* camera);

        CString GetName() const;
        void SetName(CString name);
        
        void Update(float deltaTime);

        void Clear();

        static void RegisterTypes();

        void PrintTree();

    private:
        void LoadObject(const Json& objectData, GameObject* parent = nullptr);

    private:
        std::vector<std::unique_ptr<GameObject>> mObjects = {};

        GameObject* mMainCamera = nullptr;

        String mName = "UnnamedScene";
    };
} // namespace golias
