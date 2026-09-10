#pragma once

#include "game_object.h"


namespace golias {


    class Scene {

    public:
        Scene() = default;

        /// @brief  Loads a scene from an asset path (e.g. "scene/main.gscene").
        static Ref<Scene> Load(const char* path);

        /// @brief  Loads a scene from a JSON object.
        static Ref<Scene> Load(const Json& json);

        /// @brief  Loads a scene from an asset path (e.g. "scene/main.gscene").
        static Ref<Scene> Load(CString path);

        /// @brief  Serializes the scene graph (objects, hierarchy, components).
        Json Serialize() const;

        /// @brief  Serializes and writes the scene to an asset path (e.g. "scene/main.gscene").
        bool Save(CString path) const;

        GameObject* CreateGameObject(CString name, GameObject* parent = nullptr);

        GameObject* InstantiatePrefab(const Json& json, GameObject* parent = nullptr);

        /// @brief  Deep-copies an object into scene.
        GameObject* DuplicateObject(GameObject* object, GameObject* parent = nullptr);

        template <typename T, typename = typename std::enable_if<std::is_base_of<GameObject, T>::value>::type>
        T* CreateGameObject(CString name, GameObject* parent = nullptr) {
            T* gameObject = new T();
            gameObject->SetName(name);
            gameObject->mScene = this;

            if (!mIsUpdating) {

                if (!SetParent(gameObject, parent)) {
                    GOLIAS_LOG_ERROR("Failed to set parent for GameObject '%s'.", name.data());
                    return nullptr;
                }
            } else {
                mObjectsToAdd.emplace_back(gameObject, parent);
            }

            return gameObject;
        }

        GameObject* FindGameObjectByName(CString name) const;

        const std::vector<std::unique_ptr<GameObject>>& GetAllObjects() const;

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
        void PreUpdate(float deltaTime);

        void PostUpdate(float deltaTime);

        void PrintObjectTree(const GameObject* object, size_t depth) const;

        Json SerializeObject(const GameObject* object) const;

        void LoadObject(const Json& objectData, GameObject* parent = nullptr);

        GameObject* LoadObject(const Json& objectData, GameObject* parent, bool callStart);

    private:
        std::vector<std::unique_ptr<GameObject>> mObjects              = {};
        std::vector<std::pair<GameObject*, GameObject*>> mObjectsToAdd = {};

        GameObject* mMainCamera = nullptr;

        String mName = "UnnamedScene";

        bool mIsUpdating = false;
    };
} // namespace golias
