#pragma once

#include "component.h"
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace golias {

    class GameObject {
    public:
        virtual void Update(float deltaTime);
        virtual ~GameObject() = default;


        const std::string& GetName() const;
        void SetName(const std::string& newName);

        GameObject* GetParent() const;
        void SetParent(GameObject* pParent);

        template <typename T, typename = typename std::enable_if_t<std::is_base_of_v<Component, T>>>
        T* GetComponent() const;

        void AddComponent(Component* pComponent);

        const std::vector<std::unique_ptr<GameObject>>& GetChildren() const;

        void Destroy();

        bool IsAlive() const;


        glm::vec3& GetPosition();
        void SetPosition(const glm::vec3& newPosition);

        glm::vec3& GetRotation();
        void SetRotation(const glm::vec3& newRotation);

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

    private:
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f);
        glm::vec3 scale    = glm::vec3(1.0f);
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
