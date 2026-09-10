#pragma once
#include "components/component.h"

namespace golias {

    class Scene;
    struct Collision;

    class GameObject {

    public:
        virtual ~GameObject() = default;

        static GameObject* Load(CString modelPath, Scene* scene, CString name = "");

        virtual void Start();

        virtual const char* GetTypeName() const;

        virtual bool LoadProperties(const Json& properties);

        virtual bool SaveProperties(Json& properties) const;

        virtual void Update(float deltaTime);

        virtual void OnCollisionEnter(const Collision& collision);
        virtual void OnCollisionExit(const Collision& collision);

        void SetName(CString name);

        String GetName() const;

        void SetParent(GameObject* parent);

        GameObject* GetParent() const;

        GameObject* GetRoot() const;

        /// @brief  Root of this object's model/prefab instance.
        GameObject* GetInstanceRoot() const;

        GameObject* FindChildByName(CString name) const;

        void Destroy();

        bool IsAlive() const;

        bool IsActive() const;
        bool IsActiveSelf() const;
        void SetActive(bool active);

        glm::vec3 GetPosition() const;
        void SetPosition(const glm::vec3& position);

        glm::vec2 GetPosition2D() const;
        void SetPosition2D(const glm::vec2& position);

        glm::vec3 GetWorldPosition() const;
        glm::vec2 GetWorldPosition2D() const;
        void SetWorldPosition(const glm::vec3& position);

        glm::quat GetRotation() const;
        void SetRotation(const glm::quat& rotation);
        void SetRotation(const glm::vec3& eulerAngles);

        float GetRotation2D() const;
        void SetRotation2D(float angle);

        glm::quat GetWorldRotation() const;
        void SetWorldRotation(const glm::quat& rotation);

        void RotateLocal(const glm::vec3& axis, float angle);

        glm::vec3 GetScale() const;
        void SetScale(const glm::vec3& scale);

        glm::vec2 GetScale2D() const;
        void SetScale2D(const glm::vec2& scale);

        glm::mat4 GetWorldTransform() const;
        glm::mat4 GetWorldTransform2D() const;

        glm::mat4 GetLocalTransform() const;
        glm::mat4 GetLocalTransform2D() const;

        glm::vec3 GetForward() const;

        Scene* GetCurrentScene() const;

        const std::vector<std::unique_ptr<GameObject>>& GetChildren() const;

        const std::vector<std::unique_ptr<Component>>& GetComponentList() const;

        template <typename T, typename = typename std::enable_if<std::is_base_of_v<Component, T>>>
        T* GetComponent() {

            size_t typeId = Component::StaticTypeId<T>();
            for (const auto& component : mComponents) {
                if (component->GetTypeId() == typeId || ComponentRegistry::GetInstance().HasParent(component->GetTypeId(), typeId)) {
                    return static_cast<T*>(component.get());
                }
            }

            return nullptr;
        }

        template <typename T>
        T* Cast(GameObject* object) {
            GOLIAS_ASSERT(object != nullptr);

            if (object) {
                return static_cast<T*>(object);
            }

            return nullptr;
        }

        /// @brief  Adds a component to the game object. The game object takes ownership of the component and will manage its lifetime.
        /// @param component
        void AddComponent(Component* component);

        /// @brief  Removes a component from the game object immediately.
        /// @param component
        void RemoveComponent(Component* component);

    protected:
        GameObject() = default;

        friend class Scene;

    private:
        void RecomputeActiveState();

        /// @brief  Marks this object's cached world transform as stale so it is rebuilt on next access.
        void MarkTransformDirty();

        /// @brief  Recursively marks all descendants' cached world transforms as stale.
        void MarkChildrenTransformDirty() const;

        String mName;

        GameObject* mParent = nullptr;

        /// @brief  Topmost GameObject of the model/prefab instance this node belongs to (nullptr = scene object).
        GameObject* mInstanceRoot = nullptr;

        Scene* mScene = nullptr;

        std::vector<std::unique_ptr<GameObject>> mChildren = {};

        std::vector<std::unique_ptr<Component>> mComponents = {};

        bool mIsAlive           = true;
        bool mIsActiveSelf      = true;
        bool mActiveInHierarchy = true;

        glm::vec3 mPosition = glm::vec3(0.0f);
        glm::quat mRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 mScale    = glm::vec3(1.0f);

        /// @brief  Lazily rebuilt world transform.
        mutable bool mWorldTransformDirty = true;
        mutable glm::mat4 mWorldTransform = glm::mat4(1.0f);
    };


    class ObjectFactoryBase {
    public:
        virtual ~ObjectFactoryBase()       = default;
        virtual GameObject* Create() const = 0;
    };

    template <typename T>
    class ObjectFactory : public ObjectFactoryBase {
    public:
        virtual GameObject* Create() const override {
            return new T();
        }
    };


    class ObjectRegistry {
    public:
        static ObjectRegistry& GetInstance() {
            static ObjectRegistry instance;
            return instance;
        }

        template <typename T>
        void RegisterObject(CString pName) {
            creators.emplace(pName.data(), std::make_unique<ObjectFactory<T>>());
        }

        GameObject* CreateObject(CString pName) const;

    private:
        std::unordered_map<std::string, std::unique_ptr<ObjectFactoryBase>> creators;
    };

#define GCLASS(Clazz)                                                            \
public:                                                                           \
    static void Register() {                                                      \
        golias::ObjectRegistry::GetInstance().RegisterObject<Clazz>(#Clazz);      \
    }                                                                             \
    virtual const char* GetTypeName() const override {                      \
        return #Clazz;                                                            \
    }
} // namespace golias
