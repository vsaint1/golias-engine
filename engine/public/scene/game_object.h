#pragma once

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

        void AddChild(std::unique_ptr<GameObject> child);

        const std::vector<std::unique_ptr<GameObject>>& GetChildren() const;

        void Destroy();

        bool IsAlive() const;

        std::vector<std::unique_ptr<GameObject>> children;

        glm::vec3& GetPosition();
        void SetPosition(const glm::vec3& newPosition);
        glm::vec3& GetRotation();
        void SetRotation(const glm::vec3& newRotation);
        glm::vec3& GetScale();
        void SetScale(const glm::vec3& newScale);

        const glm::mat4& GetLocalTransform() const;

        const glm::mat4& GetWorldTransform() const;

    protected:
        GameObject() = default;

        std::string name;
        GameObject* parent = nullptr;
        bool is_alive      = true;

        friend class Scene;

    private:
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f);
        glm::vec3 scale = glm::vec3(1.0f);
    };
} // namespace golias
