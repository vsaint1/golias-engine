#pragma once
#include "components/component.h"
#include "stdafx.h"

namespace golias {


    class GameObject {

    public:
        virtual ~GameObject() = default;

        virtual void Update(float deltaTime);

        void SetName(CString name);

        String GetName() const;

        void SetParent(GameObject* parent);

        GameObject* GetParent() const;

        void Destroy();

        bool IsAlive() const;

        glm::vec3 GetPosition() const;
        void SetPosition(const glm::vec3& position);

        glm::quat GetRotation() const;
        void SetRotation(const glm::quat& rotation);

        void RotateLocal(const glm::vec3& axis, float angle);

        glm::vec3 GetScale() const;
        void SetScale(const glm::vec3& scale);

        glm::mat4 GetWorldTransform() const;

        glm::mat4 GetLocalTransform() const;

        glm::vec3 GetForward() const;


        template <typename T, typename = typename std::enable_if<std::is_base_of_v<Component, T>>>
        T* GetComponent() {
           
            for (const auto& component : mComponents) {
                if (component->GetTypeId() == Component::StaticTypeId<T>()) {
                    return static_cast<T*>(component.get());
                }
            }

            return nullptr;
        }

        /// @brief  Adds a component to the game object. The game object takes ownership of the component and will manage its lifetime.
        /// @param component
        void AddComponent(Component* component);

    protected:
        GameObject() = default;

        friend class Scene;

    private:
        String mName;

        GameObject* mParent = nullptr;

        std::vector<std::unique_ptr<GameObject>> mChildren = {};

        std::vector<std::unique_ptr<Component>> mComponents = {};

        bool mIsAlive = true;

        glm::vec3 mPosition = glm::vec3(0.0f);
        glm::quat mRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 mScale    = glm::vec3(1.0f);
    };
} // namespace golias
