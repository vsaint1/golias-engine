#pragma once

#include "component.h"
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace golias {

    class Scene;

    class GameObject {
    public:
        virtual void LoadProperties(const nlohmann::json& json);

        virtual void Start();
        virtual void Update(float deltaTime);
        virtual ~GameObject() = default;

        bool IsActive() const;
        void SetActive(bool active);

        const std::string& GetName() const;
        void SetName(const std::string& newName);

        GameObject* GetParent() const;
        bool SetParent(GameObject* pParent);

        template <typename T, typename = typename std::enable_if_t<std::is_base_of_v<Component, T>>>
        T* GetComponent() const;

        GameObject* FindChildByName(const std::string_view pName) const;

        void AddComponent(Component* pComponent);

        const std::vector<std::unique_ptr<GameObject>>& GetChildren() const;

        Scene* GetScene() const;

        void Destroy();

        bool IsAlive() const;

        static GameObject* LoadModel(const std::string_view pPath, Scene* pScene);

        glm::vec3 GetWorldPosition() const;
        glm::vec3 GetPosition() const;
        glm::vec2 GetPosition2D() const;
        glm::vec2 GetWorldPosition2D() const;

        void SetWorldPosition(const glm::vec3& pos);
        void SetPosition(const glm::vec3& pos);
        void SetPosition2D(const glm::vec2& pos);

        glm::quat GetWorldRotation() const;
        glm::quat GetRotation() const;
        float GetRotation2D() const;


        void SetWorldRotation(const glm::quat& rot);
        void SetRotation(const glm::quat& rot);
        void SetRotation2D(float degrees);

        glm::vec2 GetScale2D() const;
        glm::vec3 GetScale() const;

        void SetScale(const glm::vec3& value);
        void SetScale2D(const glm::vec2& value);

        glm::mat4 GetLocalTransform() const;
        glm::mat4 GetWorldTransform() const;
        glm::mat4 GetLocalTransform2D() const;
        glm::mat4 GetWorldTransform2D() const;

    protected:
        GameObject() = default;

        std::vector<std::unique_ptr<GameObject>> children;
        std::vector<std::unique_ptr<Component>> components;

        std::string name;
        GameObject* parent = nullptr;
        bool isAlive       = true;

        friend class Scene;
        Scene* scene = nullptr;

    private:
        glm::vec3 position = glm::vec3(0.0f);
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 scale    = glm::vec3(1.0f);

        bool isActive = true;
    };


    template <typename T, typename>
    T* GameObject::GetComponent() const {
        size_t typeId = Component::StaticTypeId<T>();

        for (const auto& comp : components) {
            if (comp->GetTypeId() == typeId || ComponentRegistry::GetInstance().HasParent(comp->GetTypeId(), typeId)) {
                return static_cast<T*>(comp.get());
            }
            
        }

        return nullptr;
    }


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
        void RegisterObject(const std::string_view pName) {
            creators.emplace(pName.data(), std::make_unique<ObjectFactory<T>>());
        }

        GameObject* CreateObject(const std::string_view pName) const {
            auto it = creators.find(pName.data());
            if (it != creators.end()) {
                return it->second->Create();
            }

            return nullptr;
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<ObjectFactoryBase>> creators;
    };

#define GCLASS(Clazz)                                                        \
public:                                                                      \
    static void Register() {                                                 \
        golias::ObjectRegistry::GetInstance().RegisterObject<Clazz>(#Clazz); \
    }
} // namespace golias
