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

        static GameObject* LoadModel(const std::string_view pPath);

        glm::vec3 GetWorldPosition() const;
        glm::vec3& GetPosition();
        void SetPosition(const glm::vec3& newPosition);

        glm::quat& GetRotation();
        void SetRotation(const glm::quat& newRotation);

        glm::vec3& GetScale();
        void SetScale(const glm::vec3& newScale);

        glm::mat4 GetLocalTransform() const;
        glm::mat4 GetWorldTransform() const;

    protected:
        GameObject() = default;

        std::vector<std::unique_ptr<GameObject>> children;
        std::vector<std::unique_ptr<Component>> components;

        std::string name;
        GameObject* parent = nullptr;
        bool is_alive      = true;

        friend class Scene;
        Scene* scene = nullptr;

    private:
        glm::vec3 position = glm::vec3(0.0f);
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 scale    = glm::vec3(1.0f);

        bool is_active = true;

    };


    template <typename T, typename>
    T* GameObject::GetComponent() const {
        size_t typeId = Component::StaticTypeId<T>();

        for (const auto& comp : components) {
            if (comp->GetTypeId() == typeId) {
                return static_cast<T*>(comp.get());
            }
        }

        return nullptr;
    }
} // namespace golias
